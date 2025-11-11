#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpMsgFixedSizeDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"

TcpMsgFixedSizeDemarcar::TcpMsgFixedSizeDemarcar(uint16_t msg_fixed_size)
    : TcpMsgDemarcar(DEFAULT_CBC_SIZE)
{
    this->msg_fixed_size = msg_fixed_size;
}

TcpMsgFixedSizeDemarcar::~TcpMsgFixedSizeDemarcar()
{
}

/**
 * @brief 判断当前缓冲区中是否存在完整的一条(或多条)固定大小的消息可以读取
 *
 * @return true 当前缓冲区中存储的数据长度 >= 1 个完整消息的长度
 * @return false 前缓冲区中存储的数据长度不够一条完整消息
 */
bool TcpMsgFixedSizeDemarcar::IsBufferReadyToFlush()
{
    // bcb->current_size: 当前缓冲区中一储存的字节数
    // msg->fixed_size 表示每条消息的固定长度
    // 如果当前可用数据长度足够组成至少一条完整消息, 返回 true
    if ((this->TcpMsgDemarcar::bcb->current_size / this->msg_fixed_size) > 0)
        return true;

    return false;
}

void TcpMsgFixedSizeDemarcar::Destroy()
{
    this->TcpMsgDemarcar::Destroy();
}

/**
 * @brief 从环形缓冲区中提取完整的固定大小消息并交给上层（控制器）处理
 *
 * @param tcp_client 指向当前客户端对象，用于回调消息处理函数
 */
void TcpMsgFixedSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client)
{
    uint16_t bytes_read;

    if (!this->IsBufferReadyToFlush())
        return;

    // 不断从缓冲区中读取完整的固定大小消息
    // 每次读取 msg_fixed_size 字节，直到读不到（返回0）为止
    while (bytes_read = BCBRead(this->TcpMsgDemarcar::bcb,
                                this->TcpMsgDemarcar::buffer,
                                this->msg_fixed_size, true))
    {
        // 将读取到的每条消息交给控制器回调函数处理
        // client_msg_recvd 是用户层的“消息接收完成”回调(函数)
        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr,
                                                tcp_client,
                                                this->TcpMsgDemarcar::buffer,
                                                bytes_read);
    }
}