// g++ -o main -std=c++14 main.cpp -lpthread
// ./main configuration_file.txt output.txt
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <map>
using namespace std;
ifstream input; // Opens the input file
vector<vector<string>> clients; // clients Queue line by line
fstream outputStream; // For output
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool isAFree = true,isBFree = true,isCFree = true; //For priority
pthread_t aTid,bTid,cTid; // Tid of teller threads
void * putClientsInQueue(void * args); // wait for arrival time and put clients in queue
void * tellers(void * args); // Tellers do their job in this function
void rezerveSeat(string type, pair<pthread_t,vector<string>> clientInfo, int indexOfSeat); // Fill the wanted seat
int getAvailableSeatForReserve(); // Reserve seats
queue<pair<pthread_t,vector<string>>> clientQueue; //Clients Queue with all features
vector<pthread_t> tids; // all tids of client threads
int completedClientNumber = 0; //total number of clients to stop while loop
int numClients; // Total number of clients
int numberOfSeats; // Total number of seats
map<int,string> seats; // seats for putting clients
string FULL  = "FULL"; // for filling seats
int main(int argc, char *argv[]) {
   
    string theatorName,numClientsStr; // Theator name, number of clients
    string clientName,arrivalTime,serviceTime,seatNumber; // client name, arrival time, service time, seat number
    
    string line; //for every line of input file
    input.open(argv[1]); // opens input file
    outputStream.open(argv[2], ios::app);
    outputStream << "Welcome to the Sync-Ticket!\n";
    outputStream.flush();
    getline(input, theatorName); // Get theator name
    getline(input, numClientsStr); // Get number of clients
    stringstream ss1(theatorName);
    ss1 >> theatorName;
    //These ifs determine the number of seats
    if(theatorName == "OdaTiyatrosu") numberOfSeats = 60;
    else if(theatorName == "UskudarStudyoSahne") numberOfSeats = 80;
    else if(theatorName == "KucukSahne") numberOfSeats = 200;
    numClients = stoi(numClientsStr); // make number of clients integer
    
    while (getline(input, line)) { //Reading all lines and putting them into data array.
        stringstream ss(line);
        do {
            if (ss.eof())
                break;
            string word;
            try {
                vector<string> line;
                for(int i = 0; i < 4 && getline(ss, word, ','); i++){
                    line.push_back(word); //clientName,arrivalTime,serviceTime,seatNumber;
                }
                clients.push_back(line);
                line.clear();
            }
            catch (exception e) {
                break;
            }
        } while (ss);
    }
    input.close();

    for(int i = 0; i <= numberOfSeats; i++)
        seats.insert(make_pair(i,"")); // filling seats with "" to indicate that the seat is empty.

    string type[3] = {"A","B","C"};
    for(int i = 0; i < 3; i++){
        pthread_t tid;
        pthread_create(&tid, NULL, tellers, (void *) &type[i]); // creating all tellers
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep for 1 millisecond to make sync order of tellers.
    }


    for(int i = 0; i < clients.size(); i++){
        pthread_t tid;
        pthread_create(&tid, NULL, putClientsInQueue, &clients[i]); // create all clients
    }

    //Joining all threads.
    pthread_join(aTid, NULL);
    pthread_join(bTid, NULL);
    pthread_join(cTid, NULL);

    for(int i = 0; i < tids.size(); i++){
	    pthread_join(tids[i], NULL);
    }

    outputStream << "All clients received service.";
    outputStream.flush();
    return 0;
}

// Putting all clients into clientqueue
void * putClientsInQueue(void * args) {
    vector<string> treadInfo = *(vector<string> *) args; //clientName,arrivalTime,serviceTime,seatNumber;
    
    int sleepTime = stoi(treadInfo[1]);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime)); // sleep until arrival time
    
    // critical section
    pthread_mutex_lock(&mutex);
    tids.push_back(pthread_self()); // pushing tids to tids vector.
    clientQueue.push(make_pair(pthread_self(),treadInfo)); //QUEUE OF first=>threadPid, second=>clientName,arrivalTime,serviceTime,seatNumber;
    pthread_mutex_unlock(&mutex);
    // End Of critical section
    
    wait(NULL);

    return 0;
}

//Tellers do their job in this function. They reserve seats for clients.
void * tellers(void * args) {
    string type = *(string *) args; //Getting arguments
    //These if for determining the thread type.
    if(type == "A") aTid = pthread_self();
    else if(type == "B") bTid = pthread_self();
    else if(type == "C") cTid = pthread_self();

    outputStream << "Teller "<< type <<" has arrived.\n";
    outputStream.flush();
    //cout << type << endl;
    string which = ""; //For priority
    pair<pthread_t,vector<string>> clientInfo; // Info all client
    while (completedClientNumber != numClients) {
        which = isAFree ? "A" : (isBFree ? "B" : (isCFree ? "C" : "")); //For priority

        if(which != type) continue; //Check thread for their priority.

        if(clientQueue.empty()) continue;//If queue is empty continue.

    
        int indexOfSeat=-1;
        
        // critical section
        pthread_mutex_lock(&mutex);
        clientInfo = clientQueue.front(); // //first=>threadPid, second=>clientName,arrivalTime,serviceTime,seatNumber;
        clientQueue.pop(); // Pop client to reserve its seat
        completedClientNumber++; //incremented when a client process is being completed
        pthread_mutex_unlock(&mutex);
        // end of critical section
        
        //This if check whether the requested seat is available.
        if(seats[stoi(clientInfo.second[3])] != FULL && stoi(clientInfo.second[3]) > 0 && stoi(clientInfo.second[3]) <= numberOfSeats) {
            indexOfSeat = stoi(clientInfo.second[3]);

            // critical section
            pthread_mutex_lock(&mutex);
            seats[indexOfSeat] = FULL; //reserve the requested seat
            pthread_mutex_unlock(&mutex);
            // end of critical section

        } else {
            indexOfSeat = getAvailableSeatForReserve(); //If requested seat is not available, then find another seat for the client.
            if(indexOfSeat != -1) { //If a seat is found for the client, reserve it.
                // critical section
                pthread_mutex_lock(&mutex);
                seats[indexOfSeat] = FULL; //Reserve the seat for the client.
                pthread_mutex_unlock(&mutex);
                // end of critical section
            }
        }
        //The teller is now busy.
        if(type == "A") isAFree = false;
        else if(type == "B") isBFree = false;
        else if(type == "C") isCFree = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for the service time.
        rezerveSeat(type,clientInfo,indexOfSeat); //Write output for the reserved seat.

        //The teller is now available.
        if(type == "A") isAFree = true;
        else if(type == "B") isBFree = true;
        else if(type == "C") isCFree = true;

            
    }
    
    pthread_exit(NULL);

}

//Write the output for the reserved seat.
void rezerveSeat(string type, pair<pthread_t,vector<string>> clientInfo, int indexOfSeat){
        // critical section
        pthread_mutex_lock(&mutex);
        if(indexOfSeat != -1){
            outputStream << clientInfo.second[0] << " requests seat " << stoi(clientInfo.second[3]) << "," << " reserves seat " << indexOfSeat << ". Signed by Teller "<<type<<".\n";
        } else {
            outputStream << clientInfo.second[0] << " requests seat " << stoi(clientInfo.second[3]) << ", reserves None. " << "Signed by Teller "<<type<<".\n";
        }
        outputStream.flush();
        pthread_mutex_unlock(&mutex);
        // end of critical section
}

// Reserve a seat for the client. This function is called when the requested seat is not available.
int getAvailableSeatForReserve() {
    int index = -1;
    for(int i = 1; i <= numberOfSeats; i++){
        if(seats.at(i) == "") {
            index = i;
            break;
        }
    }
    return index;
}