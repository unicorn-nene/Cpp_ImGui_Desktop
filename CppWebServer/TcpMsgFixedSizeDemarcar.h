#ifndef _TCPMSGFIXEDSIZEDEMARCAR_H_
#define _TCPMSGFIXEDSIZEDEMARCAR_H_

#include <stdint.h>
#include "TcpMsgDemarcar.h"

class TcpClient;

/**
 * @brief 固定长度消息处理分包器(demarcar)
 *
 */
class TcpMsgFixedSizeDemarcar : public TcpMsgDemarcar
{
private:
    uint16_t msg_fixed_size;

public:
    bool IsBufferReadyToFlush() override;
    void ProcessClientMsg(TcpClient *tcp_client) override;

    // constructor
    TcpMsgFixedSizeDemarcar(uint16_t fixed_size);
    ~TcpMsgFixedSizeDemarcar();
    void Destroy();
};

#endif