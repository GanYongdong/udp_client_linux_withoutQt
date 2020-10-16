#ifndef UDPCLIENT_INS_H
#define UDPCLIENT_INS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <fstream>

bool udp_recev_ins_loop();

class udpClient_INS
{
public:
    udpClient_INS();
    ~udpClient_INS();
    static udpClient_INS* instance();

    bool init();
    void set_hostAddress(std::string addr_str_);
    void set_port(unsigned short port_);
    bool close();

    bool recev_a_message(int timeout_ = 5000);
    bool recev_loop_in_sup_thread();

    int ToInt32(unsigned char *p, int sta);
    void set_freq(int freq_ = 50);

    void set_saveTxt(bool is_need_save_txt_ = true);

public:
    bool being_udpRecv_processing = false;
    int mSfd = -1;
    unsigned char *mBuf = new unsigned char[2014];
    int mBufSize = 2014;
    struct sockaddr_in mAddr;
    double mGPSInfos[3] = {0, 0, 0};//Lat, Lon, Speed
    float mAltitude = 0;
    std::string mAddr_str = "192.168.1.9";
    unsigned short mPort = 3000;
    bool mIsEverBind = false;
    int mFreq = 50; //接收和转发频率
    std::ofstream txt;
    bool mIsNeedSaveTxt = false;

//thread相关
private:
    std::thread udp_recv_thread;
    std::string txt_name = "/home/ganyd/Projects/udp_origin_content.txt";

};

#endif // UDPCLIENT_INS_H
