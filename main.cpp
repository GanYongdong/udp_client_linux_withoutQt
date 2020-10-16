#include <iostream>
#include <udpclient_ins.h>
#include <unistd.h>

int main()
{
    std::cout << "Hello World!" << std::endl;

    udpClient_INS *IMU = udpClient_INS::instance();

    IMU->set_saveTxt();
    IMU->init();
    IMU->set_freq(20);
    IMU->recev_loop_in_sup_thread();

    pthread_exit(NULL);


    return 0;
}
