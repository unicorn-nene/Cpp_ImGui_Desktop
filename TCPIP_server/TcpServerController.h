#ifndef _TCPSERVERCONTROLLER_H_
#define _TCPSERVERCONTROLLER_H_

#include <cstdint>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include "TcpMsgDemarcar.h"

class TcpNewConnectionAcceptor; // CAS = Connection Acceptor Service
class TcpClientServiceManager;  // DRS = Data Receive Service
class TcpClientDbManager;       // DBM = (Client) Database Manager
class TcpClient;

// Server States
#define TCP_SERVER_INITIALZED 1
#define TCP_SERVER_RUNNING 2
#define TCP_SERVER_NOT_ACCEPTING_NEW_CONNECTIONS 4
#define TCP_SERVER_NOT_LISTENING_CLIENT 8
#define TCP_SERVER_CREATE_MULTI_THREADED_CLIENT 16

typedef enum tcp_server_msg_code_
{
    CTRLR_ACTION_TCP_CLIENT_PROCESS_NEW = 1,             // 新客户端连接处理
    CTRLR_ACTION_TCP_CLIENT_MULTIPLEX_LISTEN = 2,        // 客户端加入多路复用监听
    CTRLR_ACTION_TCP_CLIENT_DELETE = 4,                  // 删除客户端
    CTRLR_ACTION_TCP_CLIENT_MX_TO_MULTITHREADED = 8,     // 从多路复用迁移到多线程
    CTRLR_ACTION_TCP_CLIENT_MULTITHREAD_TO_MX = 16,      // 从多线程迁移到多路复用
    CTRLR_ACTION_TCP_CLIENT_CREATE_THREADED = 32,        // 创建线程化客户端
    CTRLR_ACTION_TCP_CLIENT_ACTIVE_CONNECT_SUCCESS = 64, // 主动连接成功通知
    CTRLR_ACTION_TCP_CLIENT_RECONNECT = 128,             // 客户端需要重新连接
    CTRLR_ACTION_TCP_SERVER_OP_MAX = 256                 // 最大控制消息号
} tcp_server_msg_code_t;

typedef struct TcpServerMsg_
{
    tcp_server_msg_code_t code; // 控制操作类型
    void *data;                 // 对应的客户端或者其他数据
    sem_t *zero_sema;           // 传入线程阻塞等待消息完成
} TcpServerMsg_t;

class TcpServerController
{
private:
    TcpNewConnectionAcceptor *tcp_new_conn_acc;
    TcpClientDbManager *tcp_client_db_mgr;
    TcpClientServiceManager *tcp_client_svc_mgr;

    uint32_t state_flags; // 用于保存服务器当前状态位

    pthread_rwlock_t connect_db_rwlock;           // 保护客户端列表的读写锁
    std::list<TcpClient *> establishedClient;     // 已经建立连接的客户端列表
    std::list<TcpClient *> connectpendingClients; // 刚建立未加入系统的客户端列表

    // Server Msg Q
    std::list<TcpServerMsg_t *> msgQ;         // 消息队列
    pthread_mutex_t msgq_mutex;               // 队列锁
    pthread_cond_t msgq_cv;                   // 条件变量(当队列有消息)
    pthread_t msgQ_op_thread;                 // 后台线程,处理消息队列
    void ProcessMsgQMsg(TcpServerMsg_t *msg); // 消息处理具体逻辑

public:
    // State variables
    uint32_t ip_addr;             // IP 地址
    uint16_t port_no;             // 端口号
    std::string name;             // 服务器名称
    TcpMsgDemarcarType msgd_type; // 消息解包方式

    void (*client_connected)(const TcpServerController *, const TcpClient *);                            // 客户端连接成功回调
    void (*client_disconnected)(const TcpServerController *, const TcpClient *);                         // 客户端断开回调
    void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t); // 收到客户端消息回调
    void (*client_ka_pending)(const TcpServerController *, const TcpClient *);                           // 等待或失效回调

    // Constructors and Destructors
    TcpServerController(std::string ip_addr, uint16_t port_no, std::string name);
    ~TcpServerController();

    //
    void Start(); // 启动所有服务
    void Stop();  // 停止所有服务
    uint32_t GetStateFlags();

    void SetBit(uint32_t bit);
    void UnSetBit(uint32_t bit);
    bool IsBitSet(uint32_t bit);

    void SetClientCreationMode(bool);
    void SetServerNotifCallbacks(void (*client_connected)(const TcpServerController *, const TcpClient *),
                                 void (*client_disconnected)(const TcpServerController *, const TcpClient *),
                                 void (*client_msg_recvd)(const TcpServerController *, const TcpClient *, unsigned char *, uint16_t),
                                 void (*client_ka_pending)(const TcpServerController *, const TcpClient *));

    // process Client Migration, used by Application
    void ProcessClientMigrationToMultiThread(uint32_t ip_addr, uint16_t port_no);
    void ProcessClientMigrationToMultiplex(uint32_t ip_addr, uint16_t port_no);

    // Used by Acceptor Service
    void ProcessNewClient(TcpClient *tcp_client);

    // Used by Application
    void ProcessClientDelete(uint32_t ip_addr, uint16_t port_no);
    void ProcessClientDelete(TcpClient *);

    // Used by Multiplex Service
    void RemoveClientFromDB(TcpClient *);

    // To Pass the request to Multiplex Service Mgr, this is Synchronous.
    void ClientFDStartListen(TcpClient *tcp_client);

    // Used my Multiplex service for client migration
    void CreateMultiThreadedClient(TcpClient *);

    // Accept/No Accept of new Connections
    void StopConnectionsAcceptorSvc();
    void StartConnectionsAcceptorSvc();

    // Listen for Connected Clients
    void StopClientSvcMgr();
    void StartClientSvcMgr();

    void SetTcpMsgDemarcar(TcpMsgDemarcarType);

    // Print the Tcp Server Details
    void Dispaly();
    void MsgQProcessingThreadFn();
    void EnqueMsg(tcp_server_msg_code_t code, void *data, bool block_me);
    void CreateActiveClient(uint32_t server_ip_addr, uint16_t server_port_no);
    void CopyAllClientsTolist(std::list<TcpClient *> *list);
    TcpClient *LookupActiveOpened(uint32_t ip_addr, uint16_t port_no);
};

#endif /*TCPSERVERCONTROLLER*/