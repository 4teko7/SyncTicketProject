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
queue<pair<pthread_t,vector<string>>> clientQueue;
vector<pthread_t> tids;
int completedClientNumber = 0;
int numClients;
int numberOfSeats;
map<int,string> seats;
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
                    line.push_back(word);
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
        if(i == 0) aTid = tid;
        else if(i == 1) bTid = tid;
        else if(i == 2) cTid = tid;
    }


    for(int i = 0; i < clients.size(); i++){
        pthread_t tid;
        pthread_create(&tid, NULL, putClientsInQueue, &clients[i]);
        tids.push_back(tid);
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
    vector<string> treadInfo = *(vector<string> *) args;
    int sleepTime = stoi(treadInfo[1]);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime)); // sleep for arrival time
    
    // critical section
    pthread_mutex_lock(&mutex);
    clientQueue.push(make_pair(pthread_self(),treadInfo));
    pthread_mutex_unlock(&mutex);
    // End Of critical section
    
    wait(NULL);

    return 0;
}

void * tellers(void * args) {
    string type = *(string *) args;
    //cout << type << endl;

    pair<pthread_t,vector<string>> clientInfo;
    while (completedClientNumber != numClients) {
    int indexOfSeat=-1;
        if(!clientQueue.empty()){

            if(isAFree && pthread_self() == aTid) {
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                clientQueue.pop();
                completedClientNumber++;
                isAFree = false;
                if(seats[stoi(clientInfo.second[3])] != "FULL") {
                    seats[stoi(clientInfo.second[3])] = "FULL";
                    indexOfSeat = stoi(clientInfo.second[3]);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                rezerveSeat(type,clientInfo,indexOfSeat);
                //cout << "A : " << clientInfo.first << " " << clientInfo.second[0] << endl;
                isAFree = true;
            }
            else if(!isAFree && isBFree && pthread_self() == bTid){
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                clientQueue.pop();
                completedClientNumber++;
                isBFree = false;
                if(seats[stoi(clientInfo.second[3])] != "FULL") {
                    seats[stoi(clientInfo.second[3])] = "FULL";
                    indexOfSeat = stoi(clientInfo.second[3]);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                rezerveSeat(type,clientInfo,indexOfSeat);
                //cout << "B : " << clientInfo.first << " " << clientInfo.second[0] << endl;
                isBFree = true;
            }
            else if(!isAFree && !isBFree && isCFree && pthread_self() == cTid){
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                clientQueue.pop();
                completedClientNumber++;
                isCFree = false;
                if(seats[stoi(clientInfo.second[3])] != "FULL") {
                    seats[stoi(clientInfo.second[3])] = "FULL";
                    indexOfSeat = stoi(clientInfo.second[3]);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                rezerveSeat(type,clientInfo,indexOfSeat);
                //cout << "C : " << clientInfo.first << " " << clientInfo.second[0] << " " << stoi(clientInfo.second[1]) << " " << clientInfo.second[2] << " " << clientInfo.second[3] << endl;
                isCFree = true;
            }
            // A,B,C => [C,A,B] => A,B,C
            
        }
    }
    
    pthread_exit(NULL);

}

void rezerveSeat(string type, pair<pthread_t,vector<string>> clientInfo, int indexOfSeat){
    pthread_mutex_lock(&mutex);
            if(clientInfo.second[0] !="") {
                cout<< type << " " << clientInfo.second[0] << " " << clientInfo.second[1] << " " << clientInfo.second[2] << " " << clientInfo.second[3] <<endl;

                if(indexOfSeat != -1){
                    seats[stoi(clientInfo.second[3])] = "FULL";
                    outputStream << clientInfo.second[0] << " requests seat" << stoi(clientInfo.second[3]) << "," << " reserves seat " << indexOfSeat << ". Signed by Teller C.\n";
                } else {
                    int index = -1;
                    for(int i = 1; i <= numberOfSeats; i++){
                        if(seats.at(i) == "") {
                            index = i;
                            break;
                        }
                    }
                    if(index != -1){
                        seats[index] = "FULL";
                        outputStream << clientInfo.second[0] << " requests seat" << stoi(clientInfo.second[3]) << ", reserves seat " << index << ". Signed by Teller C.\n";
                    }
                }
            }
            

            pthread_mutex_unlock(&mutex);
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
