#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"

TcpMsgFixedSizeDemarcar::TcpMsgFixedSizeDemarcar(uint16_t fixed_size)
    : TcpMsgDemarcar(DEFAULT_CBC_SIZE)
{
    this->msg_fixed_size = fixed_size;
}

TcpMsgFixedSizeDemarcar::~TcpMsgFixedSizeDemarcar()
{
}

void TcpMsgFixedSizeDemarcar::Destroy()
{
    this->TcpMsgDemarcar::Destroy();
}

/**
 * @brief 判断环形缓冲区是否缓冲了一条完整的"固定长度"消息
 *
 * @return true
 * @return false
 */
bool TcpMsgFixedSizeDemarcar::IsBufferReadyToFlush()
{
    if ((this->TcpMsgDemarcar::bcb->current_size / this->msg_fixed_size) > 0)
        return true;

    return false;
}

/**
 * @brief 从环形缓冲区中读取并处理一条 "固定大小"的 TCP 消息
 *
 * @param tcp_client 发来消息的客户端对象
 */
void TcpMsgFixedSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client)
{
    uint16_t bytes_read;

    while (this->IsBufferReadyToFlush())
    {
        bytes_read = BCBRead(this->TcpMsgDemarcar::bcb,
                             this->TcpMsgDemarcar::buffer,
                             this->msg_fixed_size,
                             true);

        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr, tcp_client, this->TcpMsgDemarcar::buffer, bytes_read);
    }
}
