#ifndef TCPMSGFIXEDSIZEDEMARCAR_H_
#define TCPMSGFIXEDSIZEDEMARCAR_H_

#include <stdint.h>
#include "TcpMsgDemarcar.h"

class TcpClient;
class TcpMsgFixedSizeDemarcar : TcpMsgDemarcar
{
private:
    uint16_t msg_fixed_size;

public:
    bool IsBufferReadyToFlush() override;
    void ProcessClientMsg(TcpClient *) override;

    TcpMsgFixedSizeDemarcar(uint16_t fixed_size);
    ~TcpMsgFixedSizeDemarcar();

    void Destroy();
};

#endif