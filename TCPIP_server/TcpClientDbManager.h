#ifndef TCPCLIENTDBMANAGER_H_
#define TCPCLIENTDBMANAGER_H_

#include <list>
#include <semaphore.h>
#include <stdint.h>
#include <pthread.h>
#include <vector>

class TcpClient;
class TcpServerController;

/**
 * @brief 管理所有已连接客户端的“客户端数据库（Client DB）”管理器
 *
 * 负责维护当前所有 TcpClient 的列表，包括：
 *
 * 1. 添加 / 删除 / 更新客户端
 *
 * 2. 根据 IP + 端口查找客户端
 *
 * 3. 提供线程安全版本的查询函数
 *
 * 4. 支持一次复制所有当前客户端（用于遍历或其他操作）
 *
 * 这是服务器中用于追踪“在线客户端”的核心模块。
 *
 * 使用 pthread_rwlock 保护客户端列表，使得读操作可并行，写操作独占。
 */
class TcpClientDbManager
{
private:
    pthread_rwlock_t rwlock;              // 客户端数据库的读写锁
    std::list<TcpClient *> tcp_client_db; // 所有 TcpClient 对象的数据库(链表)

public:
    TcpServerController *tcp_ctrlr;

    TcpClientDbManager(TcpServerController *);
    ~TcpClientDbManager();

    // client DB manager functions
    void Purge();                                                             // 清空整个客户端数据
    void AddClientToDB(TcpClient *);                                          // 向数据库中加入一个客户端
    void RemoveClientFromDB(TcpClient *);                                     // 从数据库移除一个客户端
    TcpClient *RemoveClientFromDB(uint32_t ip_addr, uint16_t port_no);        // 通过 Ip/端口 删除客户端
    void UpdateClient(TcpClient *);                                           // 更新客户端信息
    TcpClient *LookUpClientDB(uint32_t, uint16_t);                            // 根据 IP / 端口 查找客户端 (非线程安全版本)
    TcpClient *LookUpClientDB_ThreadSafe(uint32_t ip_addr, uint16_t port_no); // 根据 IP/端口 查找客户端(线程安全版本)
    void DisplayClientDB();                                                   // 展示客户端数据库
    void CopyAllClientsTolist(std::list<TcpClient *> *list);                  // 当前所有客户端复制到另一个 list 中(用于遍历)
};

#endif