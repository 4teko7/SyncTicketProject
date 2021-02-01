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
queue<pair<pthread_t,vector<string>>> clientQueue;
vector<pthread_t> tids;
int completedClientNumber = 0;
int numClients;
int main(int argc, char *argv[]) {
   
    string theatorName,numClientsStr; // Second line of input file
    string clientName,arrivalTime,serviceTime,seatNumber;
    
    string line; //for every line of input file
    cout << argv[1] << endl;
    input.open(argv[1]);
    outputStream.open(argv[2], ios::app);
    outputStream << "Welcome to the Sync-Ticket!\n";
    outputStream.flush();
    outputStream.close();
    getline(input, theatorName); // Get second line
    getline(input, numClientsStr); // Get second line
    numClients = stoi(numClientsStr);
    
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
                // ss >> clientName >> arrivalTime >> serviceTime >> seatNumber;
                // cout << clientName << "  -  " << arrivalTime << "  -  " << serviceTime << "  -  " << seatNumber << endl;
            }
            catch (exception e) {
                break;
            }
        } while (ss);
    }
    input.close();

    for(int i = 0; i < 3; i++){
        pthread_t tid;
        pthread_create(&tid, NULL, tellers, NULL);
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


    return 0;
}


void * putClientsInQueue(void * args) {
    vector<string> treadInfo = *(vector<string> *) args;
    int sleepTime = stoi(treadInfo[1]);
    // sleep(sleepTime); // wait for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime)); // sleep for 1 second
    // cout << "SLEEP TIME " << treadInfo[0] << " : " << sleepTime << endl;
    // critical section
    pthread_mutex_lock(&mutex);
    clientQueue.push(make_pair(pthread_self(),treadInfo));
    pthread_mutex_unlock(&mutex);
    // End Of critical section

    pthread_exit(NULL);
    
    return 0;
}

void * tellers(void * args) {
    // string numberOfTeller = * (string *) args;
    // Creatical Section
    pair<pthread_t,vector<string>> clientInfo;
    while (completedClientNumber != numClients) {
        // critical section
        if(!clientQueue.empty()){

            // cout << "isAFree : " << isAFree << " - isBFree : " << isBFree << " - isCFree : " << isCFree << endl; 
            // if(isAFree && pthread_self() != aTid) continue;
            if(isAFree && pthread_self() == aTid) {
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                pthread_mutex_lock(&mutex);
                clientQueue.pop();
                completedClientNumber++;
                pthread_mutex_unlock(&mutex);
                isAFree = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                cout << "A : " << clientInfo.first << " " << clientInfo.second[0] << endl;
                isAFree = true;
            }
            // else if(!isAFree && isBFree && pthread_self() != bTid) continue;
            else if(!isAFree && isBFree && pthread_self() == bTid){
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                pthread_mutex_lock(&mutex);
                clientQueue.pop();
                completedClientNumber++;
                pthread_mutex_unlock(&mutex);
                isBFree = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                cout << "B : " << clientInfo.first << " " << clientInfo.second[0] << endl;
                isBFree = true;
            }
            // else if(!isAFree && !isBFree && isCFree && pthread_self() != cTid) continue;
            else if(!isAFree && !isBFree && isCFree && pthread_self() == cTid){
                if(clientQueue.empty()) continue;
                clientInfo = clientQueue.front();
                pthread_mutex_lock(&mutex);
                clientQueue.pop();
                completedClientNumber++;
                pthread_mutex_unlock(&mutex);
                isCFree = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(stoi(clientInfo.second[2]))); // sleep for 1 second
                cout << "C : " << clientInfo.first << " " << clientInfo.second[0] << " " << clientInfo.second[1] << " " << clientInfo.second[2] << " " << clientInfo.second[3] << endl;
                isCFree = true;
            }

                
            
        }
                
        
        
        // End Of critical section

    }
    
    pthread_exit(NULL);

    // return 0;
    // END OF Creatical Section

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
