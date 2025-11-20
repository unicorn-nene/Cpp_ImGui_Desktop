#ifndef TCPCLIENTSERVICEMANAGER_H_
#define TCPCLIENTSERVICEMANAGER_H_

#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <list>

#define MAX_CLIENT_SUPPTORTED 127

class TcpServerController;
class TcpClient;

/**
 * @brief 负责多路复用监听所有已连接 TcpClient 的 socket, 是服务器的 “数据接收与事件分发(DRS)” 服务模块.
 *
 * 1.管理所有客户端的 fd（socket）
 *
 * 2.使用 select() 监听多个客户端的数据到达事件
 *
 * 3.当有客户端可读/可写时，将事件通知 Controller 或回调
 *
 * 4.管理客户端的添加、删除、迁移（到多线程客户端）
 *
 * 5.提供简单模式 / 高级模式 两种监听方式
 *
 * 6.管理内部线程，独立执行 select 轮询
 *
 */
class TcpClientServiceManager
{
private:
    int max_fd;                           // 当前所有客户端的最大值
    int udp_fd;                           //  一个 UDP_fd
    std::list<TcpClient *> tcp_client_db; // 客户端服务器数据
    fd_set active_fd_set;                 // 当前使用的 fd_set
    fd_set backup_fd_set;                 // 备份的 fd_set

    int GetMaxFdSimple(); // 获取最大 fd (simple)
    int GetMaxFdAdv();    // 获取最大 fd (Advance)

    pthread_t *client_svc_mgr_thread;            // 多路复用服务线程
    sem_t wait_for_thread_operation_to_complete; // 信号量: 等待操作完成
    sem_t sem0_1, sem0_2;                        // 用于线程启动/线程停止
    pthread_rwlock_t rwlock;                     // tcp_client_db 读写锁

    void ForceUnblockSelect();                // 从 Select() 强制唤醒
    void TcpClientMigrate(TcpClient *);       // 将客户端迁移到多线程模式
    void Purge();                             // 清除所有客户端
    void CopyClientFDtoFDSet(fd_set *fd_set); // 将所有 client 的 Fd 填入fd_set

public:
    TcpServerController *tcp_ctrlr;

    TcpClientServiceManager(TcpServerController *);
    ~TcpClientServiceManager();

    // 启动 Service Manager 的监听线程(两种模式)
    void StartTcpClientServiceManagerThread();
    void StartTcpClientServiceManagerThreadInternalSimple();
    void StartTcpClientServiceManagerThreadInternal2();

    void StopTcpClientServiceManagerThread();                 // 停止监听线程
    void ClientFDStartListenSimple(TcpClient *);              // Simple 模式添加客户端到监听 FD 集合
    void ClientFDStartListenAdv(TcpClient *);                 // Advanced 模式添加客户端到监听 FD 集合
    void RemoveClientFromDB(TcpClient *);                     // 从 DB 移除到客户端
    void AddClientToDB(TcpClient *);                          // 向 DB 添加客户端
    TcpClient *ClientFDStopListListenAdv(uint32_t, uint16_t); // Advanced 模式停止添加客户端(通过 ip/port 查找)
    void ClientFDStopListenAdv(TcpClient *);                  // Advanced 模式停止监听客户端
    void ClientFDStopListenSimple(TcpClient *);               // Simple 模式停止添加客户端
    TcpClient *LookUpClientDB(uint32_t, uint16_t);            // 按 ip/port 查找客户端
    void Stop();                                              // 停止整个服务
};
#endif