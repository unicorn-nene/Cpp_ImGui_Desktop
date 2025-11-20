#ifndef TCPMSGDEMARCAR_H_
#define TCPMSGDEMARCAR_H_

#include <stdint.h>
#define DEFAULT_CBC_SIZE (10240)

typedef enum
{
    TCP_DEMARCAR_NONE,          // 不做分帧
    TCP_DEMARCAR_FIXED_SIZE,    // 固定长度分帧
    TCP_DEMARCAR_VARIABLE_SIZE, // 变长分帧
    TCP_DEMARCAR_PATTERN        // 根据起始/结束模式匹配分帧
} TcpMsgDemarcarType;

class TcpClient;
typedef struct ByteCircularBuffer_ ByteCircularBuffer_t;

/**
 * @brief TCP 消息分帧器(Message Demarcar)
 *
 * 提供一个抽象接口，允许子类以不同方式处理 TCP 流数据，将连续的字节流分割为完整的业务消息。
 */
class TcpMsgDemarcar
{
private:
protected:
    ByteCircularBuffer_t *bcb; // 字节环形缓冲区, 用于存储未解析完的字节流
    unsigned char *buffer;     // 临时缓冲区, 存放提取出的完整消息

public:
    /**
     * @brief 工厂函数，根据分帧类型创建不同的子类实例
     *
     * @param type                分帧类型，例如固定长度 / 模式匹配
     * @param fixed_size          固定长度模式下的长度（其他模式忽略）
     * @param start_pattern       起始标记（模式匹配模式使用）
     * @param start_pattern_size  起始标记长度
     * @param end_pattern         结束标记（模式匹配模式使用）
     * @param end_pattern_size    结束标记长度
     */
    static TcpMsgDemarcar *InstantiateTcpMsgDemarcar(TcpMsgDemarcarType,
                                                     uint16_t fixed_size,
                                                     unsigned char start_pattern[],
                                                     uint8_t start_pattern_size,
                                                     unsigned char end_pattern[],
                                                     uint8_t end_pattern_size);

    // to be implemented by derieved classes
    virtual bool IsBufferReadyToFlush() = 0;                  // 纯虚函数, 判断环形缓冲区中的字节是否足够组成一个完整消息
    virtual void ProcessClientMsg(TcpClient *tcp_client) = 0; // 纯虚函数, 解析环形缓冲区中的数据, 并处理完整消息

    TcpMsgDemarcar(uint16_t circular_buffer_len);
    TcpMsgDemarcar();
    ~TcpMsgDemarcar();

    uint16_t GetTotalMsgSize();                                                // 返回当前完整消息的长度
    void Destroy();                                                            // 删除对象
    void ProcessMsg(TcpClient *, unsigned char *msg_recvd, uint16_t msg_size); // 将接收到的消息写入环形缓冲区
};

#endif