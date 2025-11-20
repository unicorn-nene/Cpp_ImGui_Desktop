#ifndef TCPMSGVARIABLESIZEDEMARCAR_H_
#define TCPMSGVARIABLESIZEDEMARCAR_H_

#include <stdint.h>

#include "TcpMsgDemarcar.h"

#define VARIABLE_SIZE_MAX_BUFFER 256

class Tcpclient;
class TcpMsgVariableSizeDemarcar : TcpMsgDemarcar
{
private:
public:
    bool IsBufferReadyToFlush() override;
    void ProcessClientMsg(TcpClient *) override;

    TcpMsgVariableSizeDemarcar();
    ~TcpMsgVariableSizeDemarcar();
    void Destroy();
};
#endif