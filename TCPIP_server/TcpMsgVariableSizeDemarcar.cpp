#include <assert.h>
#include "TcpMsgDemarcar.h"
#include "TcpClient.h"
#include "TcpServerController.h"
#include "ByteCircularBuffer.h"
#include "TcpMsgVariableSizeDemarcar.h"

#define HDR_MSG_SIZE 2 // 消息头字段, 储存当前消息的长度
//  2字节消息长度(uint16_t) | 消息体(msg_size字节)

TcpMsgVariableSizeDemarcar::TcpMsgVariableSizeDemarcar()
    : TcpMsgDemarcar(DEFAULT_CBC_SIZE)
{
}

TcpMsgVariableSizeDemarcar::~TcpMsgVariableSizeDemarcar()
{
}

void TcpMsgVariableSizeDemarcar::Destroy()
{
    this->TcpMsgDemarcar::Destroy();
}

/**
 * @brief 判断当前环形缓冲区中的数据是否构成了一条 "完整消息"
 *
 * @return true
 * @return false
 */
bool TcpMsgVariableSizeDemarcar::IsBufferReadyToFlush()
{
    uint16_t msg_size;

    ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;

    // 当前消息长度不够组成消息头字段,肯定不足以组成完成消息
    if (bcb->current_size <= HDR_MSG_SIZE)
        return false;

    // 读取消息头字段存储的长度数据,只读不删除
    BCBRead(bcb, (unsigned char *)&msg_size, HDR_MSG_SIZE, false);

    // 当前缓冲区读取的消息长度 大于 当前消息长度,则已经组成至少一条完整消息
    if (msg_size <= bcb->current_size)
        return true;

    return false;
}

/**
 * @brief 从环形缓冲区中解析并处理 "长度可变"的 TCP 消息
 *
 * @param tcp_client 发来 TCP 的客户端对象
 */
void TcpMsgVariableSizeDemarcar::ProcessClientMsg(TcpClient *tcp_client)
{
    uint16_t msg_size;

    // 储存网络收到但是未处理的数据
    ByteCircularBuffer_t *bcb = this->TcpMsgDemarcar::bcb;

    // 当缓冲区中还有完整的消息时
    while (this->IsBufferReadyToFlush())
    {
        // 读取消息头数据
        BCBRead(bcb, (unsigned char *)&msg_size, HDR_MSG_SIZE, false);
        // 根据消息头读取到的消息长度 读取完整的消息
        assert(msg_size == BCBRead(bcb, this->TcpMsgDemarcar::buffer, msg_size, true));

        // 将一条完成消息交给应用层处理
        tcp_client->tcp_ctrlr->client_msg_recvd(tcp_client->tcp_ctrlr, tcp_client, this->TcpMsgDemarcar::buffer, msg_size);
    }
}