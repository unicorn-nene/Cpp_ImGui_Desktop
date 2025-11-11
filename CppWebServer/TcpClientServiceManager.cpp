#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include "TcpServerController.h"
#include "TcpClientServiceManager.h"
#include "TcpClient.h"

#define TCP_CLIENT_RECV_BUFFER_SIZE_ 1024
unsigned char client_recv_buffer[TCP_CLIENT_RECV_BUFFER_SIZE_];

TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr)
{
    this->tcp_ctrlr = tcp_ctrlr;

    this->max_fd = 0;
    FD_ZERO(&this->active_fd_set);
    FD_ZERO(&this->backup_fd_set);

    client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
}

TcpClientServiceManager::~TcpClientServiceManager()
{
}
void TcpClientServiceManager::CopyClientFDtoFDSet(fd_set *fdset)
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        FD_SET(tcp_client->comm_fd, fdset);
    }
}

int TcpClientServiceManager::GetMaxFd()
{

    int max_fd_lcl = 0;
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        if (tcp_client->comm_fd > max_fd_lcl)
            max_fd_lcl = tcp_client->comm_fd;
    }

    return max_fd_lcl;
}

void TcpClientServiceManager::AddClientToDB(TcpClient *tcp_client)
{
    this->tcp_client_db.push_back(tcp_client);
    tcp_client->Reference();
}

/**
 * @brief 线程处理内部实现函数
 *
 */
void TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal()
{
    // Invoke select system call on all Clients present in Clinet DB
    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    // 设置 client socket 的 fd 到 backup_fd_set
    this->max_fd = this->GetMaxFd();
    FD_ZERO(&this->backup_fd_set);
    this->CopyClientFDtoFDSet(&this->backup_fd_set);

    while (true)
    {
        // 从备份 fd_set 到  active_fd_set(调用前恢复), 因为 select()会修改传入的 fd_set
        memcpy(&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));
        // 阻塞等待: 直到出现一个 socket 就绪可读,
        // select()调用后, active_fd_set 会包含就绪的 fd(对应的socket)
        select(this->max_fd + 1, &this->active_fd_set, nullptr, nullptr, nullptr);

        // 在 active_fd_set 上查找对应的 tcp_client
        for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
        {

            next_tcp_client = *(++it);

            if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set))
            {
                // 从已连接的TCP套接字接收数据，并获取发送方的地址信息
                rcv_bytes = recvfrom(tcp_client->comm_fd,
                                     client_recv_buffer,
                                     TCP_CLIENT_RECV_BUFFER_SIZE_,
                                     0,
                                     (struct sockaddr *)&client_addr,
                                     &addr_len);

                // 如果上层注册了消息到达回调，就调用它把收到的数据交给上层处理
                if (this->tcp_ctrlr->client_msg_recvd)
                {
                    this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client, client_recv_buffer, rcv_bytes);
                }
            }
        }
    }
}

/**
 * @brief 客户端管理线程处理函数
 *
 * @param arg 线程函数参数
 * @return void*
 */
void *tcp_client_svc_manager_thread_fn(void *arg)
{
    TcpClientServiceManager *svr_mgr = (TcpClientServiceManager *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);  // 允许当前线程被取消
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr); // 线程启用延迟取消 -> 只在安全点(取消点)时才真正退出线程

    svr_mgr->StartTcpClientServiceManagerThreadInternal();

    return nullptr;
}

void TcpClientServiceManager::StartTcpClientServiceManagerThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(this->client_svc_mgr_thread, &attr, tcp_client_svc_manager_thread_fn, (void *)this);

    printf("Service started: TcpClientServiceManagerThread\n");
}
void TcpClientServiceManager::StopTcpClientServiceManagerThread()
{
    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, nullptr);

    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = nullptr;
}

/**
 * @brief 处理新的客户端连接.
 *
 * - 停止旧的的监听线程.
 * - 更新客户端数据库
 * - 并重启监听线程
 *
 * @param tcp_client 新加入的客户端对象指针
 */
void TcpClientServiceManager::ClientFDStartListen(TcpClient *tcp_client)
{
    // CAS thread cancels the DRS thread ai cancellation Points(pthread_cancel())
    // CAS thread Waits for the Cancellaion to complete (pthread_join())
    // CAS thread Update DRS's Client DB
    // CAS thread Restart the DRS thread

    this->StopTcpClientServiceManagerThread();
    printf("Client Svc Mgr Thread is cancelled\n");

    this->AddClientToDB(tcp_client);

    // Linux 下的 select() 调用
    // 它只监听 调用时 被放进 fd_set 的文件描述符；
    // 如果有新 socket 加入，必须重新构造 fd_set 并重新调用 select()；
    // 如果旧线程正卡在 select() 里，就无法自动感知新 socket
    this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    this->StartTcpClientServiceManagerThread();
}