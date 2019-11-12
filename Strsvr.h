//#ifndef STRSVR_H
#include <iostream>
//#include <sstream>
using namespace std;

/*数据流转发参数结构体*/
typedef struct {
    string stationName; //p0
    string stationIP;
    string stationPort;
    string stationStatus;
    string serverIP;
    string serverPort;
    string serverCmd;  //p1

    pthread_t stationThreadID;
    string noUse = ""; //p3
    char* startArg[3]; //p2
} stationInfo;

/*数据流转发参数结构体*/
typedef struct {
    string stationID; //p0
    //string inType;
    string inIP;
    string inport;
    //string inMountPoint;
    //string inUserID;
    //string inPassWord;
    string outIP;
    string outport;
    string cmd;  //p1
    string noUse = "";

    char* startArg[3];
} strsvrArg_t;

//#endif // STRSVR_H
