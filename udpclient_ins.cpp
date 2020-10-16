#include "udpclient_ins.h"
#include <string.h>
#include "math.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

udpClient_INS::udpClient_INS()
{

}

udpClient_INS::~udpClient_INS()
{
    close();
}

udpClient_INS *udpClient_INS::instance()
{
    static udpClient_INS inst;
    return &inst;
}

bool udpClient_INS::init()
{
    memset(&mAddr, 0, sizeof (struct sockaddr_in));

    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(mPort);
    mAddr.sin_addr.s_addr = INADDR_ANY;
    //mAddr.sin_addr.s_addr = inet_addr(mAddr_str.c_str());

    /*
     * int socket(int domain, int type, int protocol);
     * domain: 协议族，对于TCP/IP协议族，该参数置AF_INET;
     * type：套接字类型，流套接字类型为SOCK_STREAM、数据报套接字类型为SOCK_DGRAM、原始套接字SOCK_RAW
     * protocol：协议，IPPROTO_TCP，IPPROTO_UDP
     */
    mSfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(mSfd == -1)
    {
        std::cout << "socket error" << std::endl;
        return false;
    }

    if (mIsNeedSaveTxt == true) {
        txt.open(txt_name, std::ios::out);
        if (!txt.is_open()) {
            std::cout << "error: txt file is not open" << std::endl;
            return false;
        }
    }

    return true;
}

void udpClient_INS::set_hostAddress(std::string addr_str_)
{
    mAddr_str = addr_str_;
}

void udpClient_INS::set_port(unsigned short port_)
{
    mPort = port_;
}

bool udpClient_INS::close()
{
    being_udpRecv_processing = false;

    std::thread::id null_thread_id;
    if (null_thread_id != udp_recv_thread.get_id())
    {
        if (udp_recv_thread.joinable())
        {
            udp_recv_thread.join();
        }
    }
    std::cout << "recv thread closed" << std::endl;

    return true;
}

bool udpClient_INS::recev_a_message(int timeout_)
{
    int ret = -1;

    if (udpClient_INS::instance()->mIsEverBind == false)
    {
        ret = bind(mSfd, (struct sockaddr *)&mAddr, sizeof (sockaddr_in));
        if(ret == -1)
        {
            std::cout << "bind error" << std::endl;
            return false;
        }
        else
        {
            udpClient_INS::instance()->mIsEverBind = true;
        }
    }

    int count = 0;//计数 判断是否超时用
    while (1)
    {
        if (++count > timeout_) //是否超时
        {
            std::cout << "timeout: udp attempt recev a message failed" << std::endl;
            break;
        }

        ret = recv(mSfd, mBuf, mBufSize, 0); //接收一组数据到缓冲区
        if (ret == -1)
        {
            std::cout << "recvfrom error" << std::endl;
            return false;
        }
        else
        {
            //std::cout << "recvive a message" << std::endl;
            //std::string str(mBuf, mBuf+sizeof(mBuf));
            //std::cout << str << std::endl;
        }

        if(mBuf[0] == 0xE7 && ret == 72) //判断是不是惯导发过来的
        {

            memcpy(&mGPSInfos[0], &mBuf[23], sizeof(double));
            memcpy(&mGPSInfos[1], &mBuf[31], sizeof(double));
            mGPSInfos[0] *= 180 / M_PI;
            mGPSInfos[1] *= 180 / M_PI;
            double norSpe = ToInt32(mBuf, 43) * 0.0001;
            double EastSpe  = ToInt32(mBuf, 46) * 0.0001;
            mGPSInfos[2] = sqrt(norSpe * norSpe + EastSpe * EastSpe) * 3.6;
            memcpy(&mAltitude, &mBuf[39], sizeof(float));

            printf("lat=%.12f, lon=%.12f, speed=%.4f, altitude=%.4f\n", mGPSInfos[0], mGPSInfos[1], mGPSInfos[2], mAltitude);

            break;//接收到一条消息就退出
        }

        //延时
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return true;
}

bool udp_recev_ins_loop()
{
    std::cout << "udp_recev_ins_loop" << std::endl;

    int ret = -1;
    static int coutn_print = 0;
    if (udpClient_INS::instance()->mIsEverBind == false)
    {
        ret = bind(udpClient_INS::instance()->mSfd, (struct sockaddr *)&udpClient_INS::instance()->mAddr, sizeof(sockaddr_in));
        if(ret == -1)
        {
            std::cout << "bind error" << std::endl;
            return false;
        }
        else
        {
            udpClient_INS::instance()->mIsEverBind = true;
        }
    }

    bool isstop = false;
    while (udpClient_INS::instance()->being_udpRecv_processing && !isstop)
    {
        ret = recv(udpClient_INS::instance()->mSfd, udpClient_INS::instance()->mBuf, udpClient_INS::instance()->mBufSize, 0);
        if (ret == -1)
        {
            std::cout << "recvfrom error" << std::endl;
            return false;
        } else {
//            std::cout << "recvive a message" << std::endl;
//            std::string str(udpClient_INS::instance()->mBuf, udpClient_INS::instance()->mBuf+sizeof(udpClient_INS::instance()->mBuf));
//            std::cout << str << std::endl;
        }

        if(udpClient_INS::instance()->mBuf[0] == 0xE7 && ret == 72)
        {
            if (udpClient_INS::instance()->mIsNeedSaveTxt) {
                static int count_rr = 0;
                unsigned char content[2014];
                memcpy(&content[0], &udpClient_INS::instance()->mBuf[0], sizeof(udpClient_INS::instance()->mBuf));
                for (int i = 0; i < 72; i++) {
                    udpClient_INS::instance()->txt << udpClient_INS::instance()->mBuf[i];
                }
                //udpClient_INS::instance()->txt << std::endl;

                if (count_rr++ > 1000)
                {
                    udpClient_INS::instance()->txt.close();
                    udpClient_INS::instance()->set_saveTxt(false);
                    std::cout << "txt closed ********" << std::endl;
                    isstop = true;
                    break;
                }
            }

            memcpy(&udpClient_INS::instance()->mGPSInfos[0], &udpClient_INS::instance()->mBuf[23], sizeof(double));
            memcpy(&udpClient_INS::instance()->mGPSInfos[1], &udpClient_INS::instance()->mBuf[31], sizeof(double));
            udpClient_INS::instance()->mGPSInfos[0] *= 180 / M_PI;
            udpClient_INS::instance()->mGPSInfos[1] *= 180 / M_PI;
            double norSpe = udpClient_INS::instance()->ToInt32(udpClient_INS::instance()->mBuf, 43) * 0.0001;
            double EastSpe  = udpClient_INS::instance()->ToInt32(udpClient_INS::instance()->mBuf, 46) * 0.0001;
            udpClient_INS::instance()->mGPSInfos[2] = sqrt(norSpe * norSpe + EastSpe * EastSpe) * 3.6;
            memcpy(&udpClient_INS::instance()->mAltitude, &udpClient_INS::instance()->mBuf[39], sizeof(float));

            if (coutn_print%20 == 1){
                printf("lat=%.12f, lon=%.12f, speed=%.4f, altitude=%.4f\n", udpClient_INS::instance()->mGPSInfos[0],
                        udpClient_INS::instance()->mGPSInfos[1], udpClient_INS::instance()->mGPSInfos[2], udpClient_INS::instance()->mAltitude);
                fflush(stdout);
            }coutn_print++;

            std::this_thread::sleep_for(std::chrono::milliseconds(1000/udpClient_INS::instance()->mFreq));
        }
    }

    return true;
}

bool udpClient_INS::recev_loop_in_sup_thread()
{
    being_udpRecv_processing = true;

    udp_recv_thread = std::thread(udp_recev_ins_loop);

    return true;
}

int udpClient_INS::ToInt32(unsigned char *p, int sta)
{
    memcpy(mBuf, &p[sta], 3);
    mBuf[3] = 0;
    if ((p[sta + 2] & 1 << 7) != 0)						//最高位为1表示负数
        mBuf[3] = 0xff;                              	//最高8位补1
    int ru;
    memcpy(&ru, mBuf, sizeof(int));
    return ru;
}

void udpClient_INS::set_freq(int freq_)
{
    mFreq = freq_;
}

void udpClient_INS::set_saveTxt(bool is_need_save_txt_)
{
    mIsNeedSaveTxt = is_need_save_txt_;
}
