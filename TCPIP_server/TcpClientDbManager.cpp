#include <assert.h>

#include "TcpClientDbManager.h"
#include "TcpServerController.h"
#include "TcpClient.h"

TcpClientDbManager::TcpClientDbManager(TcpServerController *tcp_ctrlr)
{

    this->tcp_ctrlr = tcp_ctrlr;
    pthread_rwlock_init(&this->rwlock, nullptr);
}

TcpClientDbManager::~TcpClientDbManager()
{
    assert(this->tcp_client_db.empty());
}

/**
 * @brief 在客户端数据库中查找指定 IP + port 的客户端
 *
 * @param ip_addr 客户端 IP 地址
 * @param port_no 客户端 port
 * @return TcpClient* 找到的客户端对象指针，
 */
TcpClient *TcpClientDbManager::LookUpClientDB(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client;
    std::list<TcpClient *>::iterator it;

    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        if (tcp_client->ip_addr == ip_addr && tcp_client->port_no == tcp_client->port_no)
            return tcp_client;
    }

    return nullptr;
}

/**
 * @brief 在客户端数据库中查找指定 IP + port 的客户端(线程安全)
 *
 * @param ip_addr 客户端 IP 地址
 * @param port_no 客户端 port
 * @return TcpClient* 找到的客户端对象指针
 */
TcpClient *TcpClientDbManager::LookUpClientDB_ThreadSafe(uint32_t ip_addr, uint16_t prot_no)
{
    TcpClient *tcp_client = nullptr;

    pthread_rwlock_rdlock(&this->rwlock);
    tcp_client = this->LookUpClientDB(ip_addr, prot_no);
    pthread_rwlock_unlock(&this->rwlock);

    return tcp_client;
}

/**
 * @brief 从数据库中移除一个客户端(线程安全)
 *
 * @param tcp_client 要移除的 TcpClient 指针
 */
void TcpClientDbManager::RemoveClientFromDB(TcpClient *tcp_client)
{
    pthread_rwlock_wrlock(&this->rwlock);
    this->tcp_client_db.remove(tcp_client);
    tcp_client->Dereference();
    pthread_rwlock_unlock(&this->rwlock);
}

/**
 * @brief 根据 IP + Port 从数据库中移除一个客户端(线程安全)
 *
 * @param ip_addr 客户端 IP 地址
 * @param port_no 客户端 port
 * @return TcpClient* 移除的客户端对象指针
 */
TcpClient *TcpClientDbManager::RemoveClientFromDB(uint32_t ip_addr, uint16_t port_no)
{
    TcpClient *tcp_client;

    pthread_rwlock_wrlock(&this->rwlock);
    tcp_client = this->LookUpClientDB(ip_addr, port_no);

    if (!tcp_client)
    {
        pthread_rwlock_unlock(&this->rwlock);
        return nullptr;
    }

    this->tcp_client_db.remove(tcp_client);
    tcp_client = tcp_client->Dereference();

    pthread_rwlock_unlock(&this->rwlock);
    return tcp_client;
}

void TcpClientDbManager ::UpdateClient(TcpClient *tcp_client)
{
}

/**
 * @brief 清空整个客户端数据库(线程安全)
 *
 */
void TcpClientDbManager::Purge()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client, *next_tcp_client;

    pthread_rwlock_wrlock(&this->rwlock);

    for (it = this->tcp_client_db.begin(), tcp_client = *it; it != this->tcp_client_db.end(); tcp_client = next_tcp_client)
    {
        next_tcp_client = *(++it);

        if (tcp_client_db.remove(tcp_client))
            tcp_client->StopThread();

        this->tcp_client_db.remove(tcp_client);
        tcp_client->Dereference();
    }

    pthread_rwlock_unlock(&this->rwlock);
}

/**
 * @brief 打印当前客户端信息
 *
 */
void TcpClientDbManager::DisplayClientDB()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    pthread_rwlock_rdlock(&this->rwlock);

    for (it = tcp_client_db.begin(); it != tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        tcp_client->Display();
    }

    pthread_rwlock_unlock(&this->rwlock);
}

/**
 * @brief 拷贝当前所有客户端到外部 list
 *
 * @param list 外部 list
 */
void TcpClientDbManager::CopyAllClientsTolist(std::list<TcpClient *> *list)
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    pthread_rwlock_rdlock(&this->rwlock);
    for (it = this->tcp_client_db.begin(); it != this->tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        list->push_back(tcp_client);
        tcp_client->Reference();
    }

    pthread_rwlock_unlock(&this->rwlock);
}