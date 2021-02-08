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
// void printDoubleStringVector(vector<vector<string>> doubleVector);
vector<vector<string>> clients;
fstream outputStream;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool isAFree = true,isBFree = true,isCFree = true;
pthread_t aTid,bTid,cTid;
void * printDoubleStringVector(void * args);
void * putClientsInQueue(void * args);
void * tellers(void * args);
void rezerveSeat(string type, pair<pthread_t,vector<string>> clientInfo, int indexOfSeat);
int getAvailableSeatForReserve();
queue<pair<pthread_t,vector<string>>> clientQueue;
vector<pthread_t> tids;
int completedClientNumber = 0;
int numClients;
int numberOfSeats;
map<int,string> seats;
string FULL  = "FULL";
int main(int argc, char *argv[]) {
   
    string theatorName,numClientsStr; // Second line of input file
    string clientName,arrivalTime,serviceTime,seatNumber;
    
    string line; //for every line of input file
    input.open(argv[1]);
    outputStream.open(argv[2], ios::app);
    outputStream << "Welcome to the Sync-Ticket!\n";
    outputStream.flush();
    getline(input, theatorName); // Get theator name
    getline(input, numClientsStr); // Get number of clients
    stringstream ss1(theatorName);
    ss1 >> theatorName;
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
        seats.insert(make_pair(i,""));

    string type[3] = {"A","B","C"};
    for(int i = 0; i < 3; i++){
        pthread_t tid;
        pthread_create(&tid, NULL, tellers, (void *) &type[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep for 1 millisecond
    }


    for(int i = 0; i < clients.size(); i++){
        pthread_t tid;
        pthread_create(&tid, NULL, putClientsInQueue, &clients[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // sleep for 1 millisecond
    }

    pthread_join(aTid, NULL);
    pthread_join(bTid, NULL);
    pthread_join(cTid, NULL);

    for(int i = 0; i < tids.size(); i++){
	    pthread_join(tids[i], NULL);
    }

    outputStream << "All clients received service.";

    return 0;
}


void * putClientsInQueue(void * args) {
    vector<string> treadInfo = *(vector<string> *) args; //clientName,arrivalTime,serviceTime,seatNumber;
    
    int sleepTime = stoi(treadInfo[1]);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime)); // sleep for arrival time
    
    // critical section
    pthread_mutex_lock(&mutex);
    tids.push_back(pthread_self());
    clientQueue.push(make_pair(pthread_self(),treadInfo)); //QUEUE OF first=>threadPid, second=>clientName,arrivalTime,serviceTime,seatNumber;
    pthread_mutex_unlock(&mutex);
    // End Of critical section
    
    wait(NULL);

    return 0;
}

void * tellers(void * args) {
    string type = *(string *) args;
    if(type == "A") aTid = pthread_self();
    else if(type == "B") bTid = pthread_self();
    else if(type == "C") cTid = pthread_self();

    outputStream << "Teller "<< type <<" has arrived.\n";
    outputStream.flush();
    //cout << type << endl;
    string which = "";
    pair<pthread_t,vector<string>> clientInfo;
    while (completedClientNumber != numClients) {
        which = isAFree ? "A" : (isBFree ? "B" : (isCFree ? "C" : ""));

        if(which != type) continue;

        if(clientQueue.empty()) continue;

        // if(isAFree) {
            // if(pthread_self() != aTid) continue;
        // } else {
            // if(isBFree) {
                // if(pthread_self() != bTid) continue;
            // }
        // }


    
        int indexOfSeat=-1;
       
        pthread_mutex_lock(&mutex);
        clientInfo = clientQueue.front(); // //first=>threadPid, second=>clientName,arrivalTime,serviceTime,seatNumber;
        clientQueue.pop();
        completedClientNumber++;
        pthread_mutex_unlock(&mutex);

        if(seats[stoi(clientInfo.second[3])] != FULL && stoi(clientInfo.second[3]) > 0 && stoi(clientInfo.second[3]) <= numberOfSeats) {
            indexOfSeat = stoi(clientInfo.second[3]);

            pthread_mutex_lock(&mutex);
            seats[indexOfSeat] = FULL;
            pthread_mutex_unlock(&mutex);

        } else {
            indexOfSeat = getAvailableSeatForReserve();
            if(indexOfSeat != -1) {
                pthread_mutex_lock(&mutex);
                seats[indexOfSeat] = FULL;
                pthread_mutex_unlock(&mutex);
            }
        }
        if(type == "A") isAFree = false;
        else if(type == "B") isBFree = false;
        else if(type == "C") isCFree = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep 
        rezerveSeat(type,clientInfo,indexOfSeat);

        if(type == "A") isAFree = true;
        else if(type == "B") isBFree = true;
        else if(type == "C") isCFree = true;

            
    }
    
    pthread_exit(NULL);

}

void rezerveSeat(string type, pair<pthread_t,vector<string>> clientInfo, int indexOfSeat){
        pthread_mutex_lock(&mutex);
        if(indexOfSeat != -1){
            outputStream << clientInfo.second[0] << " requests seat " << stoi(clientInfo.second[3]) << "," << " reserves seat " << indexOfSeat << ". Signed by Teller "<<type<<".\n";
        } else {
            outputStream << clientInfo.second[0] << " requests seat " << stoi(clientInfo.second[3]) << ", reserves None. " << "Signed by Teller "<<type<<".\n";
        }
        pthread_mutex_unlock(&mutex);
}


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

void * printDoubleStringVector(void * args) {
    for(int i = 0; i < clients.size(); i++){
        for(int j=0; j < clients[i].size(); j++){
            // Start critical section
            pthread_mutex_lock(&mutex);
            cout << " " << clients[i][j] << " ";
            // End critical section
            pthread_mutex_unlock(&mutex);
        }
        cout << endl;
    }
    pthread_exit(NULL);
}
