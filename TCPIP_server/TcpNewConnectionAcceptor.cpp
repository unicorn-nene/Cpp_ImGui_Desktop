#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h> // for IPPROTO_TCP
#include <unistd.h>
#include <memory.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClient.h"
#include "network_utils.h"

/**
 * @brief Construct a new Tcp New Connection Acceptor:: Tcp New Connection Acceptor object
 *
 * @param TcpServerController
 */
TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServerController *TcpServerController)
{
    this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->accept_fd < 0)
    {
        printf("Error : Could not create Accept FD\n");
        exit(0);
    }

    this->accept_new_conn_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
    pthread_rwlock_init(&this->rwlock, nullptr);
    this->accept_new_conn = true;
    this->tcp_ctrlr = TcpServerController;
}

TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor()
{
    assert(this->accept_fd == 0);
    assert(this->accept_new_conn_thread);
}

/**
 * @brief 线程主函数, 用于监听新的 TCP 客户端连接并将其交给 TcpServerController 处理
 *
 * 此函数运行在独立线程中:
 *
 * 1. 配置监听 socket（reuseaddr、bind、listen）
 *
 * 2. 循环 accept() 新连接
 *
 * 3. 根据服务器控制器状态决定是否接受连接
 *
 * 4. 为每个连接创建一个 TcpClient 对象
 *
 * 5. 初始化 TcpClient 状态、文件描述符、IP/端口等
 *
 * 6. 发送欢迎消息
 *
 * 7. 将新连接通过消息队列交给 TcpServerController 进一步处理
 */
void TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThreadInternal()
{
    int opt = 1;
    bool accept_new_conn;
    bool create_multi_thread_client;
    tcp_server_msg_code_t ctrlr_code = (tcp_server_msg_code_t)0;

    // 1.设置服务监听地址(IP + port)
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(this->tcp_ctrlr->ip_addr);
    server_addr.sin_port = htonl(this->tcp_ctrlr->port_no);

    // 设置地址复用
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setsockopt Failed\n");
        exit(0);
    }

    // 设置端口复用
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setsockopt Failed\n");
        exit(0);
    }

    // 绑定 IP 和 端口
    if (bind(this->accept_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Error: Acceptor socket bind failed [%s(0x%x), %d], error = %d\n",
               network_convert_ip_n_to_p(tcp_ctrlr->ip_addr, 0),
               tcp_ctrlr->ip_addr,
               tcp_ctrlr->port_no,
               errno);
        exit(0);
    }

    // 开始监听
    if (listen(this->accept_fd, 5) < 0)
    {
        printf("listen failed\n");
        exit(0);
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int comm_socket_fd;

    // 信号量+1, 通知外部当前函数(线程)已经完成初始化(bind + listen)
    sem_post(&this->wait_for_thread_operation_to_complete);

    // 主循环: 等待客户端连接
    while (true)
    {
        pthread_testcancel(); // 支持 pthread_cancel 中断

        // 阻塞等待一个新客户端连接
        comm_socket_fd = accept(this->accept_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (comm_socket_fd < 0)
        {
            printf("Error in Accepting New Connections\n");
            continue;
        }

        // 检查服务器是否允许接受新连接
        pthread_rwlock_rdlock(&rwlock);
        accept_new_conn = this->accept_new_conn;

        if (!accept_new_conn)
        {
            close(comm_socket_fd); // 不接受关闭服务器
            continue;
        }

        // send welcome msg to client
        // 为新连接创建 一个 TcpClient 对象
        TcpClient *tcp_client = new TcpClient;
        tcp_client->comm_fd = comm_socket_fd;
        tcp_client->ip_addr = htonl(client_addr.sin_addr.s_addr);
        tcp_client->port_no = htons(client_addr.sin_port);
        tcp_client->tcp_ctrlr = this->tcp_ctrlr;
        tcp_client->server_ip_addr = this->tcp_ctrlr->ip_addr;
        tcp_client->server_port_no = this->tcp_ctrlr->port_no;
        tcp_client->SetState(TCP_CLIENT_STATE_CONNECTED | TCP_CLIENT_STATE_PASSIVE_OPENER); // 设置客户端连接状态为 已连接|被动连接(服务器accept)
        tcp_client->SetConnectionType(tcp_conn_via_accept);                                 // 设置连接来源为 "accept"

        // 如果用户注册了连接回调,则调用
        if (this->tcp_ctrlr->client_connected)
        {
            this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client);
        }

        // 向客户端发送欢迎消息
        tcp_client->SendMsg("Welcome\n", strlen("Welcome\n"));
        // 不处理消息分界, 直接传递
        tcp_client->SetTcpMsgDemarcar(TcpMsgDemarcar::InstantiateTcpMsgDemarcar(TCP_DEMARCAR_NONE, 0, 0, 0, 0, 0));

        // 根据服务器配置(单线程/多线程)选择处理client
        if (this->tcp_ctrlr->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        {
            ctrlr_code = CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED;
        }
        else
        {
            ctrlr_code = CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN;
        }

        // 将 client 交给 TcpServerController 的消息队列处理
        this->tcp_ctrlr->EnqueMsg((tcp_server_msg_code_t)(CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW | ctrlr_code), tcp_client, false);
    }
}

/**
 * @brief 静态函数, pthread 的线程入口函数
 *
 * pthread_create 只能接受普通函数指针，不能直接接受成员函数，
 * 所以这里用一个 static 函数包装，并把 this 指针传进去。
 *
 * @param arg 线程函数参数
 * @return void*
 */
static void *tcp_listen_for_new_connections(void *arg)
{
    TcpNewConnectionAcceptor *tcp_new_conn_acc = (TcpNewConnectionAcceptor *)arg;

    // 设置为线程可取消
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    // 进入真正的线程逻辑
    tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();

    return nullptr;
}

/**
 * @brief 新线程创建, 启动新连接监听线程
 *
 */
void TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThread()
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // 设置线程属性 为 joinable (可被 join 等待)

    if (pthread_create(this->accept_new_conn_thread, &attr, tcp_listen_for_new_connections, (void *)this))
    {
        printf("%s() Thread Creation falied, error = %d\n", __FUNCTION__, errno);
        exit(0);
    }

    // 信号量+1, 线程已经完成初始化(bind + listen), 唤醒当前线程
    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("Service started : TcpNewConnectionAcceptThread\n");
}

/**
 * @brief 设置是否允许 accept 新连接，用写锁保护
 *
 * @param status
 */
void TcpNewConnectionAcceptor::SetAcceptNewConnectionStatus(bool status)
{
    pthread_rwlock_wrlock(&this->rwlock);
    this->accept_new_conn = status;
    pthread_rwlock_unlock(&this->rwlock);
}

/**
 * @brief 停止 TcpNewConnectionAcceptor 服务
 *
 */
void TcpNewConnectionAcceptor::Stop()
{
    // 停止 accept 线程
    this->StopTcpNewConnectionAcceptorThread();

    // 关闭监听 socket
    close(accept_fd);
    this->accept_fd = 0;

    // 销毁信号量和读写锁
    sem_destroy(&this->wait_for_thread_operation_to_complete);
    pthread_rwlock_destroy(&this->rwlock);

    delete this;
}

void TcpNewConnectionAcceptor::StopTcpNewConnectionAcceptorThread()
{
    // 请求取消线程
    pthread_cancel(*this->accept_new_conn_thread);
    // 等待线程退出
    pthread_join(*this->accept_new_conn_thread, nullptr);

    // 释放存放线程局句柄的内存
    free(this->accept_new_conn_thread);
    this->accept_new_conn_thread = nullptr;
}