#include <assert.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "TcpClientDbManager.h"
#include "TcpClientServiceManager.h"
#include "TcpMsgDemarcar.h"
#include "network_utils.h"
#include "TcpClient.h"

class TcpMsgDemarcar;

TcpServerController::TcpServerController(std::string ip_addr, uint16_t port_no, std::string name)
{
    this->ip_addr = network_convert_ip_p_to_n(ip_addr.c_str());
    this->port_no = port_no;
    this->name = name;

    this->tcp_new_conn_acc = new TcpNewConnectionAcceptor(this);
    this->tcp_client_db_mgr = new TcpClientDbManager(this);
    this->tcp_client_svc_mgr = new TcpClientServiceManager(this);

    this->msgd_type = TCP_DEMARCAR_FIXED_SIZE;
    pthread_mutex_init(&this->msgq_mutex, nullptr);
    pthread_cond_init(&this->msgq_cv, nullptr);
    pthread_rwlock_init(&this->connect_db_rwlock, nullptr);

    this->state_flags = 0;
    this->SetBit(TCP_SERVER_INITIALZED);
}

TcpServerController::~TcpServerController()
{
    assert(!this->tcp_new_conn_acc);
    assert(!this->tcp_client_db_mgr);
    assert(!this->tcp_client_svc_mgr);

    assert(this->connectpendingClients.empty());
    assert(this->establishedClient.empty());

    printf("TcpServerController %s Stopped\n", this->name.c_str());
}

/**
 * @brief Tcp 服务器消息队列处理线程函数
 *
 * @param arg  指向TcpServerController对象的指针，需要强制类型转换
 * @return void*
 */
static void *tcp_server_msgq_thread_fn(void *arg)
{
    TcpServerController *tcp_ctrlr = (TcpServerController *)arg;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);  // 允许线程被取消
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr); // 延迟取消模式

    tcp_ctrlr->MsgQProcessingThreadFn();
    return nullptr;
}

/**
 * @brief 启动 TCP服务器
 *
 */
void TcpServerController::Start()
{
    assert(this->tcp_new_conn_acc);
    assert(this->tcp_client_db_mgr);
    assert(this->tcp_client_svc_mgr);

    // 启动新连接接受线程
    if (!this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
    {
        this->tcp_new_conn_acc->StartTcpNewConnectionAcceptorThread();
    }

    // 启动客户端服务器管理线程
    if (!this->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
    {
        this->tcp_client_svc_mgr->StartTcpClientServiceManagerThread();
    }

    // initializing and starting TCP Server Msg Q thread
    pthread_create(&this->msgQ_op_thread, nullptr, tcp_server_msgq_thread_fn, (void *)this);

    this->SetBit(TCP_SERVER_RUNNING);

    printf("Tcp Server is Up and Running [%s, %d]\nOk.\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);
}

/**
 * @brief 停止 TCP 服务器
 *
 */
void TcpServerController::Stop()
{
    TcpClient *tcp_client;

    // 停止 CAS
    if (this->tcp_new_conn_acc)
    {
        this->tcp_new_conn_acc->Stop();
        this->tcp_new_conn_acc = nullptr;
        this->SetBit(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    }

    // 停止 DRMS
    if (this->tcp_client_svc_mgr)
    {
        this->tcp_client_svc_mgr->Stop();
        this->tcp_client_svc_mgr = nullptr;
        this->SetBit(TCP_SERVER_NOT_LISTENING_CLIENT);
    }

    // Stopping the above two services frist ensures that now no thread is alive which could add tcpclient back into DB
    // 停止 DRS
    this->tcp_client_db_mgr->Purge();
    delete this->tcp_client_db_mgr;
    this->tcp_client_db_mgr = nullptr;

    // 使用读写锁,互斥的切断正在等待的客户端
    pthread_rwlock_wrlock(&this->connect_db_rwlock);
    while (!this->connectpendingClients.empty())
    {
        tcp_client = this->connectpendingClients.front();
        assert(tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECT_IN_PROGRESS));
        tcp_client->StopConnectorThread();
        this->connectpendingClients.pop_front();
        tcp_client->Dereference();
    }

    // 切断已经建立的客户端
    while (!this->establishedClient.empty())
    {
        tcp_client = this->establishedClient.front();
        assert(tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED));
        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED))
        {
            tcp_client->StopThread();
        }
        this->establishedClient.pop_front();
        tcp_client->Dereference();
    }

    // 解锁并删除读写锁
    pthread_rwlock_unlock(&this->connect_db_rwlock);
    pthread_rwlock_destroy(&this->connect_db_rwlock);

    this->UnSetBit(TCP_SERVER_RUNNING);

    delete this;
}

/**
 * @brief 获取 TCP 服务器的状态位
 *
 * @return uint32_t 当前的状态标志位掩码
 */
uint32_t TcpServerController::GetStateFlags()
{
    return this->state_flags;
}

/**
 * @brief 设置客户端创建模式(mode), 单线程/多线程
 *
 * @param status true: 启用多线程客户端模式，false: 禁用多线程客户端模式
 */
void TcpServerController::SetClientCreationMode(bool status)
{
    // 如果请求启用多线程模式，但当前已经是多线程模式，则直接返回
    if (status && !this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        return;

    // 如果请求禁用多线程模式，但当前已经是单线程模式，则直接返回
    if (!status && !this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        return;

    // 根据请求的状态设置相应的标志位
    if (status)
        this->SetBit(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
    else
        this->UnSetBit(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT);
}

/**
 * @brief 设置通知回调
 *
 * @param client_connected      客户端连接成功回调
 * @param client_disconnected   客户端断开连接回调
 * @param client_msg_recvd      接受到客户端消息回调
 * @param client_ka_pending     等待或失效回调
 */
void TcpServerController::SetServerNotifCallbacks(void (*client_connected)(const TcpServerController *, const TcpClient *),
                                                  void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                                                  void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t),
                                                  void (*client_ka_pending)(const TcpServerController *, const TcpClient *))

{
    // should be called before invoking Start() on TCP Server
    assert(this->state_flags == TCP_SERVER_INITIALZED);
    this->client_connected = client_connected;
    this->client_disconnected = client_disconnected;
    this->client_msg_recvd = client_msg_recvd;
    this->client_ka_pending = client_ka_pending;
}

/**
 * @brief 为指定的Tcp客户端创建多线程处理环境
 *
 * @param tcp_client 需要启动多线程处理的 TCP客户端
 */
void TcpServerController::CreateMultiThreadedClient(TcpClient *tcp_client)
{
    assert(tcp_client->client_thread == nullptr);
    tcp_client->StartThread();
}

/**
 * @brief 处理新连接的TCP客户端
 *
 * @param tcp_client
 */
void TcpServerController::ProcessNewClient(TcpClient *tcp_client)
{
    // 将 tcp客户端 添加到 客户端数据库中管理
    this->tcp_client_db_mgr->AddClientToDB(tcp_client);

    // 根据服务器配置选择客户端处理模式
    if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
    {
        // 多线程模式: 为客户端创建独立线程进行并发处理
        this->CreateMultiThreadedClient(tcp_client);
    }
    else
    {
        // 单线程模式: 使用简单的事件监听机制处理客户端通信
        this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
    }
}

/**
 * @brief 根据 ip地址和端口号 处理 Tcp客户端断开连接
 *
 * @param ip_addr IP地址
 * @param port_no 端口号
 */
void TcpServerController::ProcessClientDelete(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client = this->tcp_client_db_mgr->RemoveClientFromDB(ip_addr, port_no);

    if (!tcp_client)
    {
        printf("Error : Such a client dont exist \n");
        return;
    }

    if (!tcp_client->client_thread)
    {
        this->tcp_client_svc_mgr->ClientFDStopListenSimple(tcp_client);
    }
}

/**
 * @brief 指定 TCP客户端, 处理 TCP客户端的删除和资源清理
 *
 * @param tcp_client 需要被删除和清理的TCP客户端对象指针
 */
void TcpServerController::ProcessClientDelete(TcpClient *tcp_client)
{
    // 增加一次引用计数
    tcp_client->Reference();

    // 处理被动连接端（服务端接受的连接）的清理
    if (tcp_client->IsStateSet(TCP_CLIENT_STATE_PASSIVE_OPENER))
        this->tcp_client_db_mgr->RemoveClientFromDB(tcp_client);

    // 停止客户端的多路复用监听（单线程模式）
    if (tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN))
        this->tcp_client_svc_mgr->ClientFDStopListenSimple(tcp_client);

    // 停止客户端的处理线程（多线程模式）
    if (tcp_client->client_thread)
        tcp_client->StopThread();

    // 处理主动连接端（客户端发起的连接）的清理
    if (tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER))
    {
        pthread_rwlock_wrlock(&this->connect_db_rwlock);

        // 从已建立连接列表中移除客户端
        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED))
        {
            this->establishedClient.remove(tcp_client);
            tcp_client->Dereference();
        }
        else
        // 从连接等待列表中移除客户端
        {
            this->connectpendingClients.remove(tcp_client);
            tcp_client->Dereference();
        }
        pthread_rwlock_unlock(&this->connect_db_rwlock);
    }

    // 减少引用计数
    tcp_client->Dereference();
}

/**
 * @brief 从客户端数据库中移除指定的TCP客户端
 *
 * @param tcp_client 需要从数据库中移除的TCP客户端对象指针
 */
void TcpServerController::RemoveClientFromDB(TcpClient *tcp_client)
{
    this->tcp_client_db_mgr->RemoveClientFromDB(tcp_client);
}

/**
 * @brief 启动TCP客户端的事件监听
 *
 * @param tcp_client 需要启动事件监听的TCP客户端对象指针
 *
 */
void TcpServerController::ClientFDStartListen(TcpClient *tcp_client)
{
    this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
}

/**
 * @brief 将客户端从单线程模式迁移到多线程模式
 *
 * @param ip_addr 要迁移的客户端IP地址（网络字节序）
 * @param port_no 要迁移的客户端端口号（主机字节序)
 */
void TcpServerController::ProcessClientMigrationToMultiThread(uint32_t ip_addr, uint16_t port_no)
{
    // 线程安全地查找指定IP和端口的客户端对象
    TcpClient *tcp_client = this->tcp_client_db_mgr->LookUpClientDB_ThreadSafe(ip_addr, port_no);

    if (!tcp_client)
        return;

    if (tcp_client->client_thread)
    {
        printf("Error: Client is already Multi-threaded\n");
        return;
    }

    // 停止客户端的简单监听事件(单线程模式)
    this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
    // 为客户端创建多线程模式
    this->CreateMultiThreadedClient(tcp_client);
}

/**
 * @brief 将客户端从多线程迁移到多路复用模式
 *
 * @param ip_addr 要迁移的客户端IP地址（网络字节序）
 * @param port_no 要迁移的客户端端口号（主机字节序）
 */
void TcpServerController::ProcessClientMigrationToMultiplex(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client = this->tcp_client_db_mgr->LookUpClientDB_ThreadSafe(ip_addr, port_no);

    if (!tcp_client)
        return;

    if (!tcp_client->client_thread)
    {
        printf("Error: client is already Multiplexed\n");
        return;
    }

    tcp_client->StopThread();
    this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
}

/**
 * @brief 设置 TCP 消息定界器类型
 *
 * @param msgd_type
 */
void TcpServerController::SetTcpMsgDemarcar(TcpMsgDemarcarType msgd_type)
{
    this->msgd_type = msgd_type;
}

/**
 * @brief 显示相关信息
 *
 */
void TcpServerController::Dispaly()
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    printf("Server Name : %s\n", this->name.c_str());

    if (!this->IsBitSet(TCP_SERVER_RUNNING))
    {
        printf("Not Running\n");
        return;
    }

    printf("Litening on : [%s, %d]\n", network_convert_ip_n_to_p(this->ip_addr, 0), this->port_no);

    printf("Falgs :  ");

    if (this->IsBitSet(TCP_SERVER_INITIALZED))
        printf("I");
    else
        printf("Un-I");

    if (this->IsBitSet(TCP_SERVER_RUNNING))
        printf("R");
    else
        printf("Not-R");

    if (this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
        printf("Not-Acc");
    else
        printf("Acc");

    if (this->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
        printf("Not-L");
    else
        printf("L");

    if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        printf("M");
    else
        printf("Not-M");

    printf("\n");

    this->tcp_client_db_mgr->DisplayClientDB();
    pthread_rwlock_rdlock(&this->connect_db_rwlock);

    for (it = this->connectpendingClients.begin(); it != connectpendingClients.end(); ++it)
    {
        tcp_client = *it;
        tcp_client->Display();
    }

    for (it = this->establishedClient.begin(); it != this->establishedClient.end(); ++it)
    {
        tcp_client = *it;
        tcp_client->Display();
    }

    pthread_rwlock_unlock(&this->connect_db_rwlock);
}

/**
 * @brief 处理 TCP 消息队列中的消息
 *
 * @param msg TCP服务器消息结构体指针
 */
void TcpServerController::ProcessMsgQMsg(TcpServerMsg_t *msg)
{
    TcpClient *tcp_client;

    tcp_client = (TcpClient *)msg->data;
    msg->data = nullptr;

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_DELETE)
    {
        this->ProcessClientDelete(tcp_client);
        return;
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_RECONNECT)
    {
        uint32_t server_ip_addr = tcp_client->server_ip_addr;
        uint16_t server_port_no = tcp_client->server_port_no;
        this->ProcessClientDelete(tcp_client);
        this->CreateActiveClient(server_ip_addr, server_port_no);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW)
    {
        this->tcp_client_db_mgr->AddClientToDB(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN)
    {
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN));
        this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MX_TO_MULTITHREADED)
    {
        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED))
            return;

        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN))
        {
            this->tcp_client_svc_mgr->ClientFDStopListenSimple(tcp_client);
        }

        this->CreateMultiThreadedClient(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_MULTITHREAD_TO_MX)
    {
        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN))
            return;

        if (tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED))
        {
            tcp_client->StopThread();
        }

        this->tcp_client_svc_mgr->ClientFDStartListenSimple(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED)
    {
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED));
        this->CreateMultiThreadedClient(tcp_client);
    }

    if (msg->code & CTRLR_ACTION_TCP_CLIENT_ACTIVE_CONNECT_SUCCESS)
    {
        tcp_server_msg_code_t ctrlr_code = (tcp_server_msg_code_t)0;
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECT_IN_PROGRESS));
        assert(tcp_client->IsStateSet(TCP_CLIENT_STATE_CONNECTED));
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_PASSIVE_OPENER));
        assert(tcp_client->IsStateSet(TCP_CLIENT_STATE_ACTIVE_OPENER));
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_MULTIPLEX_LISTEN));
        assert(!tcp_client->IsStateSet(TCP_CLIENT_STATE_THREADED));

        pthread_rwlock_wrlock(&this->connect_db_rwlock);
        if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        {
            this->CreateMultiThreadedClient(tcp_client);
        }
        else
        {
            this->ClientFDStartListen(tcp_client);
        }
    }

    free(msg);
}

/**
 * @brief 消息队列处理函数(线程函数)
 *
 */
void TcpServerController::MsgQProcessingThreadFn()
{
    TcpServerMsg_t *msg;
    while (true)
    {
        pthread_mutex_lock(&this->msgq_mutex);

        // 等待条件: 当消息队列为空时, 线程在此等待
        while (this->msgQ.empty())
        {
            pthread_cond_wait(&this->msgq_cv, &this->msgq_mutex);
        }

        // 批量处理队列中的所有消息，直到队列为空
        while (1)
        {
            if (this->msgQ.empty())
                break;
            msg = this->msgQ.front();
            this->msgQ.pop_front();
            this->ProcessMsgQMsg(msg);

            // 如果消息带有信号量，通知发送方消息已处理完成
            // 用于实现同步的消息处理模式
            if (msg->zero_sema)
            {
                sem_post(msg->zero_sema); // 信号量+1, 唤醒等待线程(传递消息线程)
            }
        }

        pthread_mutex_unlock(&this->msgq_mutex);
    }
}

/**
 * @brief 向消息队列传递消息(线程函数)
 *
 * @param code 消息类型代码
 * @param data 消息关联的数据
 * @param block_me 是否阻塞模式: true-同步模式(等待消息处理再返回), false-异步模式(投递消息之后立即返回)
 */
void TcpServerController::EnqueMsg(tcp_server_msg_code_t code, void *data, bool block_me)
{
    sem_t sem;
    TcpServerMsg_t *msg = (TcpServerMsg_t *)calloc(1, sizeof(TcpServerMsg_t));
    msg->code = code;
    msg->data = data;

    // 根据阻塞模式配置信号量
    if (block_me)
    {
        sem_init(&sem, 0, 0);
        msg->zero_sema = &sem;
    }
    else
    {
        msg->zero_sema = nullptr;
    }

    pthread_mutex_lock(&this->msgq_mutex);

    if (block_me)
        this->msgQ.push_front(msg); // 同步消息放到队列头部优先处理
    else
        this->msgQ.push_back(msg); // 异步消息放到队列尾部,按顺序处理

    pthread_cond_signal(&this->msgq_cv); // 通知消息线程有新消息到达
    pthread_mutex_unlock(&this->msgq_mutex);

    // 同步模式,等待消息完成(异步模式信号量设置nullptr, 这里直接跳过)
    if (block_me)
    {

        sem_wait(&sem); // 信号量-1, 等待其他线程完成(消息处理线程)
        sem_destroy(&sem);
    }
}

/**
 * @brief 创建主动连接的TCP客户端
 *
 * @param server_ip_addr 要连接的远程服务器IP地址（网络字节序）
 * @param server_port_no 要连接的远程服务器端口号（主机字节序）
 */
void TcpServerController::CreateActiveClient(uint32_t server_ip_addr, uint16_t server_port_no)
{
    tcp_server_msg_code_t ctrlr_code = (tcp_server_msg_code_t)0;

    // 设置客户端连接参数
    TcpClient *tcp_client = new TcpClient();
    tcp_client->server_ip_addr = server_ip_addr;
    tcp_client->server_port_no = server_port_no;
    tcp_client->ip_addr = this->ip_addr;
    tcp_client->port_no = 0; // Dynamically allocated
    tcp_client->tcp_ctrlr = this;

    // 初始化 线程操作完成 信号量
    sem_init(&tcp_client->wait_for_thread_operation_to_complete, 0, 0);

    // 配置客户端参数
    tcp_client->SetTcpMsgDemarcar(nullptr);
    tcp_client->SetState(TCP_CLIENT_STATE_ACTIVE_OPENER);
    tcp_client->conn.conn_type = tcp_conn_via_connect;

    // 将客户端添加到连接等待列表(线程安全)
    pthread_rwlock_wrlock(&this->connect_db_rwlock);
    this->connectpendingClients.push_back(tcp_client);
    pthread_rwlock_unlock(&this->connect_db_rwlock);

    // 增加计数
    tcp_client->Reference();

    // 尝试建立客户端连接(非阻塞模式)
    if (tcp_client->TryClientConnect(true) == 0)
    {
        // 连接成功, 更新客户端
        pthread_rwlock_wrlock(&this->connect_db_rwlock);
        {
            this->connectpendingClients.remove(tcp_client);
            this->establishedClient.push_back(tcp_client);
        }
        pthread_rwlock_unlock(&this->connect_db_rwlock);

        // 根据服务器配置(单线程/多线程)选择客户端模式
        if (this->IsBitSet(TCP_SERVER_CREATE_MULTI_THREADED_CLIENT))
        {
            this->CreateMultiThreadedClient(tcp_client);
        }
        else
        {
            this->ClientFDStartListen(tcp_client);
        }
    }
}

void TcpServerController::SetBit(uint32_t bit)
{

    this->state_flags |= bit;
}

void TcpServerController::UnSetBit(uint32_t bit)
{

    this->state_flags &= ~bit;
}

bool TcpServerController::IsBitSet(uint32_t bit)
{

    return (this->state_flags & bit);
}

/**
 * @brief
 *
 */
void TcpServerController::StopConnectionsAcceptorSvc()
{
    if (this->IsBitSet(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS))
        return;

    this->SetBit(TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS);
    this->tcp_new_conn_acc->Stop();
    this->tcp_new_conn_acc = nullptr;
}

/**
 * @brief
 *
 */
void TcpServerController::StopClientSvcMgr()
{
    if (this->IsBitSet(TCP_SERVER_NOT_LISTENING_CLIENT))
        return;

    this->tcp_client_svc_mgr->Stop();
    this->SetBit(TCP_SERVER_NOT_LISTENING_CLIENT);
    this->tcp_client_svc_mgr = nullptr;
}

/**
 * @brief
 *
 * @param list
 */
void TcpServerController::CopyAllClientsTolist(std::list<TcpClient *> *list)
{
    this->tcp_client_db_mgr->CopyAllClientsTolist(list);
}

/**
 * @brief 查找已建立的主动连接客户端
 *
 * @param ip_addr 远程服务器的IP地址（网络字节序）
 * @param port_no 远程服务器的端口号（主机字节序）
 * @return TcpClient* 找到的TCP客户端对象指针，如果未找到则返回nullptr
 */
TcpClient *TcpServerController::LookupActiveOpened(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    pthread_rwlock_rdlock(&this->connect_db_rwlock);
    {
        for (it = this->establishedClient.begin(); it != this->establishedClient.end(); ++it)
        {
            tcp_client = *it;
            if (tcp_client->server_ip_addr == ip_addr && tcp_client->server_port_no == port_no)
            {
                pthread_rwlock_unlock(&this->connect_db_rwlock);
                return tcp_client;
            }
        }
    }
    pthread_rwlock_unlock(&this->connect_db_rwlock);

    return nullptr;
}