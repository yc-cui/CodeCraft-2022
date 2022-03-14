#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "utils.h"

using namespace std;

class Node {
public:
    string name;
    int index;
    int bandwidth;
    Node(string name, int index) {
        this->name = name;
        this->index = index;
    }
};

class User {
public:
    string name;
    int index;
    vector<int> available;
    User(string name, int index) {
        this->name = name;
        this->index = index;
    }
};

class Time {
public:
    string name;
    int index;
    Time(string name, int index) {
        this->name = name;
        this->index = index;
    }
};

string CONFIG_PATH = "../data/config.ini";
string DATA_PATH = "../data/";
int QOS_MAX;
vector<Node> nodes;
vector<User> users;
vector<Time> times;
vector<vector<int>> qos;
vector<vector<int>> demand;

// 初始化 qos users nodes
void read_qos() {
    std::ifstream data;
    data.open(DATA_PATH + "qos.csv");
    std::string tmp_line;
    // users
    getline(data, tmp_line);
    stringstream ss(tmp_line);
    string str;
    int is_first = true;
    while (getline(ss, str, ',')) {
        if (is_first) {
            is_first = false;
        }
        else {
            int pos = users.size();
            users.emplace_back(User(str, pos));
        }
    }
    //    for (int i = 0; i < users.size(); ++i) {
    //        cout << users[i].name << " " << users[i].index << endl;
    //    }
    // qos
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray;
        int cnt = -1;
        while (getline(ss, str, ',')) {
            if (cnt == -1) {
                nodes.emplace_back(Node(str, nodes.size()));
            }
            else {
                int current_qos = atoi(str.c_str());
                lineArray.push_back(current_qos);
                if (current_qos < QOS_MAX) {
                    users[cnt].available.emplace_back(nodes.size() - 1);
                }
            }
            cnt++;
        }
        qos.emplace_back(lineArray);
    }
    data.close();
    data.clear();
}

// 初始化 times
void read_demand() {
    std::ifstream data;
    data.open(DATA_PATH + "demand.csv");
    std::string tmp_line;
    getline(data, tmp_line);
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray;
        int is_first = true;
        while (getline(ss, str, ',')) {
            if (is_first) {
                times.emplace_back(Time(str, times.size()));
                is_first = false;
            }
            else {
                lineArray.push_back(atoi(str.c_str()));
            }
        }
        demand.emplace_back(lineArray);
    }
    data.close();
    data.clear();
//        for (int i = 0; i < times.size(); ++i) {
//            cout << times[i].name << " " << times[i].index << endl;
//        }

}

// 初始化 nodes
void read_bandwidth() {
    std::ifstream data;
    data.open(DATA_PATH + "site_bandwidth.csv");
    std::string tmp_line;
    getline(data, tmp_line);
    int cnt = 0;
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        while (getline(ss, str, ',')) {
            if (is_first) {
                is_first = false;
            }
            else {
                nodes[cnt++].bandwidth = atoi(str.c_str());
            }
        }
    }
    data.close();
    data.clear();
//    for (int i = 0; i < nodes.size(); ++i) {
//        cout << nodes[i].name << " " << nodes[i].index << " " << nodes[i].bandwidth <<endl;
//    }
}

void read_conf(){
    std::ifstream config;
    config.open(CONFIG_PATH);
    std::string tmp_line;
    getline(config,tmp_line);
    while(getline(config,tmp_line)){
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        while (getline(ss, str, '=')) {
            if (is_first) {
                is_first = false;
            }
            else {
                QOS_MAX = atoi(str.c_str());
                break;
            }
        }
    }
}

void baseline() {

}

//vector<Node> nodes;
//vector<User> users;
//vector<Time> times;
//vector<vector<int>> qos;
//vector<vector<int>> demand;


int main() {

    read_conf();
    cout << QOS_MAX << endl;

    read_qos();
    read_demand();
    read_bandwidth();

    cout << "read demand.csv" << endl;
    logger_mat2d(demand);

    cout << "read qos.csv" << endl;
    logger_mat2d(qos);

    cout << "nodes" << endl;
    for (int i = 0; i < nodes.size(); ++i) {
        cout << nodes[i].name << " " << nodes[i].index << " " << nodes[i].bandwidth << endl;
    }

    cout << "users" << endl;
    for (int i = 0; i < users.size(); ++i) {
        cout << users[i].name << " " << users[i].index << endl;
    }

    cout << "times" << endl;
    for (int i = 0; i < times.size(); ++i) {
        cout << times[i].name << " " << times[i].index << endl;
    }


    return 0;
}