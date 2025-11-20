#ifndef _TCPNEWCONNECTIONACCEPTOR_H_
#define _TCPNEWCONNECTIONACCEPTOR_H_

#include <pthread.h>
#include <semaphore.h>

class TcpServerController;

/**
 * @brief 负责监听新的 TCP 客户端连接的接受器类
 *
 * 这个类的主要作用是：
 *
 * 1. 在后台启动一个线程，不停监听 accept()；
 *
 * 2. 发现新连接后，把连接信息交给 TcpServerController；
 *
 * 3. 可动态开启/关闭是否接受新连接；
 *
 * 4. 使用信号量和读写锁进行同步控制；
 */
class TcpNewConnectionAcceptor
{
private:
    int accept_fd;                               // 服务器监听的 socket 的文件描述符
    pthread_t *accept_new_conn_thread;           // 接受新连接的专用线程指针
    sem_t wait_for_thread_operation_to_complete; // 用于同步线程启动/停止操作的信号量
    pthread_rwlock_t rwlock;                     // 保护共享状态 accept_new_conn 的读写锁
    bool accept_new_conn;                        // 是否运行继续进行新连接

public:
    TcpServerController *tcp_ctrlr; // back pointer to owing Server

    TcpNewConnectionAcceptor(TcpServerController *);
    ~TcpNewConnectionAcceptor();

    void StartTcpNewConnectionAcceptorThread(); // 启动用于接受新连接的后台线程
    void StopTcpNewConnectionAcceptorThread();  // 停止后台的接受连接线程

    void SetShareSemaphore(sem_t *); // 共享外部 semaphore

    void StartTcpNewConnectionAcceptorThreadInternal(); // 内部线程执行函数
    void Stop();                                        // 停止接受新连接
    void SetAcceptNewConnectionStatus(bool);            // 谁知是否允许接受新连接
};

#endif