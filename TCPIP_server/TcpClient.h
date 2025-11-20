#ifndef TCPCLENT_H_
#define TCPCLENT_H_

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include "TcpConn.h"

#define MAX_CLIENT_BUFFER_SIZE 1024

// Tcp client States
#define TCP_CLIENT_STATE_CONNECT_IN_PROGRESS 1 // 正在主动连接服务器（主动模式）
#define TCP_CLIENT_STATE_CONNECTED 2           // 已建立 TCP 连接
#define TCP_CLIENT_STATE_PASSIVE_OPENER 8      // 由服务器“被动接受”建立的连接
#define TCP_CLIENT_STATE_ACTIVE_OPENER 16      // 作为客户端主动发起的连接
#define TCP_CLIENT_STATE_KA_BASED 32           // 启用了 KeepAlive 机制
#define TCP_CLIENT_STATE_KA_EXPIRED 64         // KeepAlive 超时（可能无响应）
#define TCP_CLIENT_STATE_MULTIPLEX_LISTEN 128  // 多路 select/poll 监听模式
#define TCP_CLIENT_STATE_THREADED 256          // 为此客户端创建了独立处理线程

typedef uint32_t client_state_bit;

class TcpClientServiceManager;
class TcpServerController;
class TcpMsgDemarcar;
class TcpConn;

/**
 * @brief 表示一个 TCP 客户端连接（无论是主动还是被动建立）
 *
 * TcpClient 代表服务器中的一个独立客户端会话对象
 *
 */
class TcpClient
{
private:
    pthread_rwlock_t rwlock; // 客户端内部读写锁
    uint32_t state_flags;    // 位掩码记录多个状态

    void Abort(); // 立即终止此客户端
    ~TcpClient(); // 析构函数

public:
    uint32_t ip_addr;        // IP地址
    uint16_t port_no;        // 端口号
    uint32_t server_ip_addr; // 服务器本地绑定地址
    uint16_t server_port_no; // 服务器本地绑定端口

    int comm_fd;   // 与客户端通信的 sokcet FD
    int ref_count; // 引用计数

    unsigned char recv_buffer[MAX_CLIENT_BUFFER_SIZE]; // 接收缓冲区
    pthread_t *client_thread;                          // 客户端专用线程
    pthread_t *active_connect_thread;                  // 主动连接线程
    TcpServerController *tcp_ctrlr;                    //
    sem_t wait_for_thread_operation_to_complete;       // 信号量

    TcpMsgDemarcar *msgd; // 指向消息分包器
    TcpConn conn;         // 封装发送/接收逻辑的连接对象

    TcpClient(uint32_t, uint16_t); // 使用 IP + Port 构造客户端
    TcpClient();                   // 默认构造函数
    TcpClient(TcpClient *);        // 拷贝构造函数

    int SendMsg(char *, uint32_t);                 // 发送消息
    void StartThread();                            // 启动客户端专用线程
    void StopThread();                             // 停止客户端线程
    void StopConnectorThread();                    // 停止主动连接
    void ClientThreadFunction();                   // 客户端工作函数
    TcpClient *Dereference();                      // 引用计数减少
    void Reference();                              // 引用计数增加
    void Display();                                // 打印客户端信息
    void SetTcpMsgDemarcar(TcpMsgDemarcar *);      // 设置消息分包器
    void SetConnectionType(tcp_connection_type_t); // 设置连接类型
    int TryClientConnect(bool);                    // 尝试主动连接服务器
    void SetState(client_state_bit flag_bit);      // 设置状态位
    void UnSetState(client_state_bit flag_bit);    // 清除状态位
    bool IsStateSet(client_state_bit flag_bit);    // 判断某个状态位是否被设置
};

#endif