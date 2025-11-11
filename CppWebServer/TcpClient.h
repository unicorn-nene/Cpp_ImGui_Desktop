#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <stdint.h>

class TcpServerController;
class TcpMsgDemarcar;

#define MAX_CLIENT_BUFFER_SIZE 1024

class TcpClient
{
private:
    /* data */
public:
    uint32_t ip_addr;
    uint16_t port_no;
    int comm_fd;
    int ref_count;

    TcpServerController *tcp_ctrlr;
    TcpMsgDemarcar *msgd;

    TcpClient(uint32_t ip_addr, uint16_t port_no);
    ~TcpClient();
    void Display();
    void Reference();
};

#endif