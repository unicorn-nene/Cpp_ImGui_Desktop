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

#include "TcpClientServiceManager.h"
#include "TcpServerController.h"
#include "TcpClient.h"
#include "network_utils.h"

#define CLIENT_RECV_BUFFER_SIZE 1024
static unsigned char common_recv_buffer[CLIENT_RECV_BUFFER_SIZE];

TcpClientServiceManager::TcpClientServiceManager(TcpServerController *tcp_ctrlr)
{
    this->tcp_ctrlr = tcp_ctrlr;
    this->udp_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (this->udp_fd < 0)
    {
        printf("UDP Socket Creation Failed, error = %d\n", errno);
        exit(0);
    }

    this->max_fd = this->udp_fd;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no + 1);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    if (bind(this->udp_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("Error: UDP socket bind failed [%s(0x%x), %d], error = %d\n",
               network_convert_ip_n_to_p(tcp_ctrlr->ip_addr, 0),
               tcp_ctrlr->ip_addr,
               tcp_ctrlr->port_no,
               errno);
        exit(0);

        FD_ZERO(&active_fd_set);
        FD_ZERO(&backup_fd_set);

        client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
        sem_init(&this->wait_for_thread_operation_to_complete, 0, 0);
        sem_init(&sem0_1, 0, 0);
        sem_init(&sem0_2, 0, 0);
        pthread_rwlock_init(&this->rwlock, nullptr);
    }
}

TcpClientServiceManager::~TcpClientServiceManager()
{
    assert(this->tcp_client_db.empty());
    assert(this->udp_fd);
    assert(this->client_svc_mgr_thread);
}

void TcpClientServiceManager::RemoveClientFromDB(TcpClient *tcp_client)
{
    this->tcp_client_db.remove(tcp_client);
    tcp_client->Dereference();
}

void TcpClientServiceManager::AddClientToDB(TcpClient *tcp_client)
{
    this->tcp_client_db.push_back(tcp_client);
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

// old implemention
void TcpClientServiceManager::StartTcpClientServiceManagerThreadInternal2()
{
}

/**
 * @brief 使用 select() 模式处理多个 tcpClient 的读事件(简单实现)
 *
 */
void TcpClientServiceManager::StartTcpClientServiceManagerThreadInternalSimple()
{
    int rcv_bytes;
    TcpClient *tcp_client, *next_tcp_client;
    struct sockaddr_in client_addr;
    std::list<TcpClient *>::iterator it;

    socklen_t addr_len = sizeof(client_addr);

    // 如果不是“仅服务器端不监听客户端”，就复制所有客户端到本地 list
    if (!this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
    {
        this->tcp_ctrlr->CopyAllClientsTolist(&this->tcp_client_db);
    }

    this->max_fd = this->GetMaxFdSimple();

    // 初始化 fd_set 备份, 用于每轮 select 复制
    FD_ZERO(&this->backup_fd_set);
    this->CopyClientFDtoFDSet(&this->backup_fd_set);

    // 信号量 +1; 唤醒(通知)外部线程当前线程已经启动完成
    sem_post(&this->wait_for_thread_operation_to_complete);

    while (true)
    {
        pthread_testcancel();

        // 每轮 select() 前都复制 fd_set，因为 select 会修改 active_fd_set
        memcpy(&this->active_fd_set, &this->backup_fd_set, sizeof(fd_set));
        // 阻塞等待任一 client_fd 上有读事件
        select(this->max_fd + 1, &this->active_fd_set, nullptr, nullptr, nullptr);

        // iterate so that we can delete the current element while travering
        // 遍历所有客户端
        for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
        {
            next_tcp_client = *(++it);

            // 如果此客户端有数据可读
            if (FD_ISSET(tcp_client->comm_fd, &this->active_fd_set))
            {
                rcv_bytes = recvfrom(tcp_client->comm_fd,
                                     common_recv_buffer,
                                     CLIENT_RECV_BUFFER_SIZE,
                                     0,
                                     (struct sockaddr *)&client_addr,
                                     &addr_len);

                // 接受信息出现错误
                if (rcv_bytes == 0 || rcv_bytes == 65535 || rcv_bytes < 0)
                {
                    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);

                    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);                                       // 移除 此 fd_set
                    this->max_fd = this->GetMaxFdSimple();                                                   // 更新 max_fd
                    this->tcp_ctrlr->EnqueMsg(CTRLR_ACTION_TCP_CLIENT_RECONNECT, (void *)tcp_client, false); // 通知tcp_ctrlr, 需要重新链接此客户端
                }
                else // 正常接收数据
                {
                    // if client has TcpMsgDemarcar, then push the data to Demarcar, else notify the application straightaway
                    tcp_client->conn.bytes_recvd += rcv_bytes;
                    // 根据客户端的 MsgDemarcar, 应用其对应的拆包逻辑
                    if (tcp_client->msgd)
                    {
                        tcp_client->msgd->ProcessMsg(tcp_client, common_recv_buffer, rcv_bytes);
                    }
                    // 直接回调给上层应用
                    else if (this->tcp_ctrlr->client_msg_recvd)
                    {
                        this->tcp_ctrlr->client_msg_recvd(this->tcp_ctrlr, tcp_client, common_recv_buffer, rcv_bytes);
                    }
                }
            }
        }
    } // while ends
}

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
static void *tcp_client_svc_manager_thread_fn(void *arg)
{
    TcpClientServiceManager *svc_mgr = (TcpClientServiceManager *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);

    svc_mgr->StartTcpClientServiceManagerThreadInternalSimple();

    return nullptr;
}

/**
 * @brief
 *
 */
void TcpClientServiceManager::StartTcpClientServiceManagerThread()
{
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (!this->client_svc_mgr_thread)
    {
        this->client_svc_mgr_thread = (pthread_t *)calloc(1, sizeof(pthread_t));
    }

    pthread_create(this->client_svc_mgr_thread, &attr, tcp_client_svc_manager_thread_fn, (void *)(this));

    sem_wait(&this->wait_for_thread_operation_to_complete);
    printf("Service started: TcpClientServiceMangerThread\n");
}

/**
 * @brief 按照 IP地址和端口查找客户端
 *
 * @param ip_addr
 * @param port_no
 * @return TcpClient*
 */
TcpClient *TcpClientServiceManager::LookUpClientDB(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;

        if (tcp_client->ip_addr == ip_addr && tcp_client->port_no == port_no)
            return tcp_client;
    }

    return nullptr;
}

/**
 * @brief 将一个 tcp_client 客户端 添加到服务管理器的监听合计中
 *
 * @param tcp_client 指向要加入监听的客户端对象
 */
void TcpClientServiceManager::ClientFDStartListenSimple(TcpClient *tcp_client)
{
    // 如果服务管理线程正在运行，先停掉以便安全更新 FD 集合
    if (!this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
    {
        this->StopTcpClientServiceManagerThread();
        printf("Client Svc Mgr thread Cancalled\n");
    }

    // 此客户端不应该已经存在（防御性检查）
    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));

    // 添加到监听数据库
    this->AddClientToDB(tcp_client);
    tcp_client->SetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    // 如果系统处于“监听客户端”状态，重新启动线程
    if (!this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
        this->StartTcpClientServiceManagerThread();
}

/**
 * @brief 将一个 TcpClient客户端 从监听集合中移除。
 * @param tcp_client 指向要停止监听的客户端对象
 */
void TcpClientServiceManager::ClientFDStopListenSimple(TcpClient *tcp_client)
{
    // 若线程正在执行 select/epoll，需要先停掉
    if (!this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
        this->StopTcpClientServiceManagerThread();

    // 客户端必须存在于监听数据库中（防御性检查）
    assert(this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));

    // 从监听数据库中移除，并更新状态
    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    // 如果系统允许监听，更新完成后重新启动线程
    if (this->tcp_ctrlr->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
        this->StartTcpClientServiceManagerThread();
}

/**
 * @brief  将一个 TcpClient 加入到监听 FD 集合 (Adv)
 *
 * 设计思路: 双信号量临界区保护
 * select() 线程 ---> (sem0_1) ---> main 线程可以修改 FD.
 *  main()线程   ---> (sem0_2) ---> select() 线程继续工作
 * @param tcp_client
 */
void TcpClientServiceManager::ClientFDStartListenAdv(TcpClient *tcp_client)
{
    // select() 调用会阻塞，所以无法直接在运行期间修改 fd_set
    // 调用 ForceUnblockSelect() 唤醒 select，使其退出阻塞
    ForceUnblockSelect();

    // 等待线程确认, 可以修改 FD集合
    sem_wait(&this->sem0_1);

    assert(!this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));

    // 添加到监听数据库(DB), 并设置监听状态
    this->AddClientToDB(tcp_client);
    tcp_client->SetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    // update FDs
    if (this->max_fd < tcp_client->comm_fd)
        this->max_fd = tcp_client->comm_fd;

    // 将 客户端tcp_client的fd 加入到 FD 集合
    FD_SET(tcp_client->comm_fd, &this->backup_fd_set);

    // 唤醒正在等待的其他线程,修改结束
    sem_post(&sem0_2);
}

/**
 * @brief 根据 ip 和 port 停止监听某个客户端（Adv 模式）
 *
 * @param ip_addr  客户端 IP
 * @param port_no  客户端端口
 * @return TcpClient* 返回被移除的 client 指针；客户端不存在返回 nullptr
 */
TcpClient *TcpClientServiceManager::ClientFDStopListListenAdv(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client;
    ForceUnblockSelect();

    // 等待 select() 线程发送"可以修改 FD集合"信号
    sem_wait(&this->sem0_1);

    tcp_client = this->LookUpClientDB(ip_addr, port_no);

    // 未找到则无需操作，直接让 select 线程继续工作
    if (!tcp_client)
    {
        sem_post(&this->sem0_2);
        return nullptr;
    }

    // 从 DB 移除并清除监听状态
    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    // update FDs
    max_fd = GetMaxFdSimple();

    // 从 FD 集合中移除当前tcp_client 的 FD
    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client); // 通知控制器断开当前客户端的连接

    // 发送信号, 当前线程工作完成,其他线程可以继续工作
    sem_post(&this->sem0_2);

    return tcp_client;
}

/**
 * @brief 根据tcp_client客户端对象停止监听客户端（Adv 模式），
 *
 * @param tcp_client 要停止监听的客户端对象
 */
void TcpClientServiceManager::ClientFDStopListenAdv(TcpClient *tcp_client)
{
    ForceUnblockSelect();

    // 等待 select() 线程发送"可以修改 FD集合"信号
    sem_wait(&this->sem0_1);

    assert(tcp_client == this->LookUpClientDB(tcp_client->ip_addr, tcp_client->port_no));

    // 从 DB 中删除
    this->RemoveClientFromDB(tcp_client);
    tcp_client->UnSetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);

    // update FDs
    max_fd = GetMaxFdSimple();

    FD_CLR(tcp_client->comm_fd, &this->backup_fd_set);
    this->tcp_ctrlr->client_disconnected(this->tcp_ctrlr, tcp_client);

    // 发送信号, 当前线程工作完成,其他线程可以继续工作
    sem_post(&this->sem0_2);
}

/**
 * @brief
 *
 * @return int
 */
int TcpClientServiceManager::GetMaxFdSimple()
{
    int max_fd_lcl = 0;
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        if (tcp_client->comm_fd > max_fd_lcl)
            max_fd_lcl = tcp_client->comm_fd;
    }

    return max_fd_lcl;
}

// unused
int TcpClientServiceManager::GetMaxFdAdv()
{
}

/**
 * @brief
 *
 */
void TcpClientServiceManager::ForceUnblockSelect()
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (fd < 0)
    {
        printf("%s() Socket Creation Failed \n", __FUNCTION__);
        return;
    }

    unsigned char dummy_data;
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    int rc = sendto(fd, (unsigned char *)dummy_data, 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (rc < 0)
        printf("%s() Sending Data to UDP Socket Failed\n", __FUNCTION__);

    close(fd);
}

/**
 * @brief
 *
 */
void TcpClientServiceManager::Purge()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    // This fn assumes that Svc mgr thread is already cancelled, hence no need to lock anything
    assert(this->client_svc_mgr_thread = nullptr);

    for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
    {
        next_tcp_client = *(++it);

        this->tcp_client_db.remove(tcp_client);
        tcp_client->UnSetState(TCP_CLIENT_STATE_MULTIPLEX_LISTEN);
        tcp_client->Dereference();
    }
}

void TcpClientServiceManager::Stop()
{
    this->StopTcpClientServiceManagerThread();
    close(this->udp_fd);
    this->udp_fd = 0;
    this->Purge();
    delete this;
}

void TcpClientServiceManager::StopTcpClientServiceManagerThread()
{
    if (!this->client_svc_mgr_thread)
        return;

    pthread_cancel(*this->client_svc_mgr_thread);
    pthread_join(*this->client_svc_mgr_thread, nullptr);
    free(this->client_svc_mgr_thread);
    this->client_svc_mgr_thread = nullptr;

    printf("Service stopped : TcpClientServiceManagerThread\n");
}