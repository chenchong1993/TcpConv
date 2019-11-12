//#include <iostream>
#include <stdio.h>
#include<string.h>
#include "Strsvr.h"
#include "libproxy-sdk.h"
#include <unistd.h>
#include<pthread.h>
#include "http_client.h"
#include "json.hpp"
#include<map>
using json = nlohmann::json;
using namespace std;
#define maxStationNum 3000


stationInfo newStationInfo[maxStationNum]; //刚收到的信息
stationInfo oldStationInfo[maxStationNum];
stationInfo addStationInfo[maxStationNum];
stationInfo editStationInfo[maxStationNum];
stationInfo delStationInfo[maxStationNum];
int newStationInfoNum=0,oldStationInfoNum=0;
map<string,stationInfo> newStationInfoMap,oldStationInfoMap;  //以stationName为键值，以整个对象为实值
// map<string,stationInfo>::iterator iter1;
/*
实现单线程数据流转发功能，线程调用此函数，来完成多线程多站的转发
参数：数据流转发参数结构体
*/
void *StartSingleThread(void* arg)
{
        stationInfo *strsvr_arg = (stationInfo*)arg;

        //printf("start result %s\n %s\n %s\n",strsvr_arg->startArg[0],strsvr_arg->startArg[1],strsvr_arg->startArg[2]);
        printf("start result %s\n",Start(strsvr_arg->startArg[0],strsvr_arg->startArg[1],strsvr_arg->startArg[2]));
}
/*
 *HTTP请求，获取基准站信息列表
 */
void stationInfoRecv(string rsp)
{
	auto stations = json::parse(rsp);
	newStationInfoNum = stations.size();
	for(int i=0;i<newStationInfoNum;i++){
        newStationInfo[i].stationName = stations[i]["stationName_pinyin"];
        newStationInfo[i].stationIP = stations[i]["stationIP"];
        newStationInfo[i].stationPort = stations[i]["stationPort"];
        newStationInfo[i].stationStatus = stations[i]["stationStatus"];
        newStationInfo[i].serverIP = stations[i]["serverIP"];
        newStationInfo[i].serverPort = stations[i]["serverPort"];
        newStationInfo[i].serverCmd = stations[i]["serverCmd"];
	}
    printf("Get Date successful.\n");

}


int main()
{
    cout << "Hello world!" << endl;

    printf("This is demo application.\n");
    string url1 = "http://121.28.103.199:5603/api/apiGetStationInfo";

    while (1)
    {
        HttpClient::SendReq(url1, stationInfoRecv);
        int editStationNum=0,delStationNum=0,addStationNum = 0;
        //初始化新的Map
        for (int i = 0; i < newStationInfoNum; ++i)
        {
            newStationInfoMap[newStationInfo[i].stationName] = newStationInfo[i];
        }
        //处理增加
        for (int i = 0; i < newStationInfoNum; ++i)
        {
            //在老的Map里面没找到新的站名，则说明这个站是新增的
            if(oldStationInfoMap.find(newStationInfo[i].stationName) == oldStationInfoMap.end()){
                addStationInfo[addStationNum] = newStationInfo[i];
                addStationNum++;
            }

        }
        //处理删除
        for (int i = 0; i < oldStationInfoNum; ++i)
        {
            //在新的Map里面没找到老的站名，则说明这个站被删除了
            if(newStationInfoMap.find(oldStationInfo[i].stationName) == newStationInfoMap.end()){
                delStationInfo[delStationNum] = oldStationInfo[i];
                delStationNum++;
            }
        }
        //处理变更
        for(int i=0;i<newStationInfoNum;i++){
            for (int j = 0; j < oldStationInfoNum; j++)
            {
                if (newStationInfo[i].stationName == oldStationInfo[j].stationName)
                {
                    if ((newStationInfo[i].stationStatus == oldStationInfo[j].stationStatus)&&
                        (newStationInfo[i].serverCmd == oldStationInfo[j].serverCmd))
                    {
                        break;
                    }else{
                        editStationInfo[editStationNum] = newStationInfo[i];
                        editStationNum++;
                    }
                }
            }
        }
        oldStationInfoNum = newStationInfoNum;
        oldStationInfoMap.swap(newStationInfoMap);
        newStationInfoMap.clear();

        //新开服务
        for (int i = 0; i < addStationNum; ++i)
        {
            //addStationInfo[i].stationThreadID = addStationInfo[i].stationName;
            if (addStationInfo[i].stationStatus == "online")
            {
                addStationInfo[i].startArg[0] = const_cast<char*>(addStationInfo[i].stationName.c_str());
                addStationInfo[i].startArg[1] = const_cast<char*>(addStationInfo[i].serverCmd.c_str());
                addStationInfo[i].startArg[2] = const_cast<char*>(addStationInfo[i].noUse.c_str());
                if (pthread_create(&addStationInfo[i].stationThreadID, NULL, StartSingleThread, &addStationInfo[i]))
                {
                    cout<<"转发成功:"<<endl;
                }
                cout<<"add:"<<addStationInfo[i].startArg[0]<<endl;
            }

        }
        //修改服务
        for (int i = 0; i < editStationNum; ++i)
        {
            editStationInfo[i].startArg[0] = const_cast<char*>(editStationInfo[i].stationName.c_str());
            editStationInfo[i].startArg[1] = const_cast<char*>(editStationInfo[i].serverCmd.c_str());
            editStationInfo[i].startArg[2] = const_cast<char*>(editStationInfo[i].noUse.c_str());
            //状态变更为上线
            if (editStationInfo[i].stationStatus == "online")
            {
                if (pthread_create(&editStationInfo[i].stationThreadID, NULL, StartSingleThread, &editStationInfo[i]))
                {
                    cout<<"转发成功:"<<endl;
                }
            }
            //状态变更为下线
            if (editStationInfo[i].stationStatus == "offline")
            {
                Stop(editStationInfo[i].startArg[0]);
            }
        }
        //删除站之后下线服务
        for (int i = 0; i < delStationNum; ++i)
        {
            delStationInfo[i].startArg[0] = const_cast<char*>(delStationInfo[i].stationName.c_str());
            delStationInfo[i].startArg[1] = const_cast<char*>(delStationInfo[i].serverCmd.c_str());
            delStationInfo[i].startArg[2] = const_cast<char*>(delStationInfo[i].noUse.c_str());
            Stop(delStationInfo[i].startArg[0]);
        }

        for (int i = 0; i < newStationInfoNum; ++i)
        {
            oldStationInfo[i] = newStationInfo[i];
        }

        printf("demo is running.\n");
        sleep (3);
        // memset(&addStationInfo, 0, sizeof(addStationInfo));
        // memset(&delStationInfo, 0, sizeof(delStationInfo));
        // memset(&editStationInfo, 0, sizeof(editStationInfo));
        // memset(&newStationInfo, 0, sizeof(newStationInfo));

    }
     // char *p0 = "http01";
     // char *p1 = "tcp -t tcp -p :20001 -T tcp -P 211.101.24.55:26036";
     // // Start the service, returns an empty string description starts successfully;returns a non-empty string description fails to start, the returned string is the cause of the error
     // printf("start result %s\n",Start(p0,p1,""));
     // printf("Version %s\n", Version());



    //  printf("start result %s\n", Version());
     //停止服务,没有返回值
    // Stop(p0);

     return 0;
}
