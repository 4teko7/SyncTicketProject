#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <pthread.h>
using namespace std;
ifstream input; // Opens the input file
// void printDoubleStringVector(vector<vector<string>> doubleVector);
vector<vector<string>> clients;
fstream outputStream;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void * printDoubleStringVector(void * args);
int main(int argc, char *argv[]) {
   
    string theatorName,numClientsStr; // Second line of input file
    string clientName,arrivalTime,serviceTime,seatNumber;
    int numClients;
    string line; //for every line of input file
    cout << argv[1] << endl;
    input.open(argv[1]);
    outputStream.open(argv[2], ios::app);
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

    // printDoubleStringVector(clients);

    pthread_t tid_1; // first thread ID
    pthread_t tid_2; // second thread ID

    int offset1 = 1;
	// Create thread and assign count function
	pthread_create(&tid_1, NULL, printDoubleStringVector, NULL);
    
	int offset2 = -1;
	pthread_create(&tid_2, NULL, printDoubleStringVector, NULL);

	// Wait for the threads to finish 
	pthread_join(tid_1, NULL);
	pthread_join(tid_2, NULL);


    return 0;
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
