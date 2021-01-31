#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
ifstream input; // Opens the input file
void printDoubleStringVector(vector<vector<string>> doubleVector);
vector<vector<string>> clients;
fstream outputStream;
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

    printDoubleStringVector(clients);

    return 0;
}

void printDoubleStringVector(vector<vector<string>> doubleVector) {
    for(int i = 0; i < doubleVector.size(); i++){
        for(int j=0; j < doubleVector[i].size(); j++){
            cout << " " << doubleVector[i][j] << " ";
        }
        cout << endl;
    }
}
