#ifndef __TCPCLIENTSERVICEMANAGER__
#define __TCPCLIENTSERVICEMANAGER__
#include <sys/select.h>
class TcpServerController;
class TcpClient;

class TcpClientServiceManager
{
private:
    // fd_set 是一个位图结构(bitset), 表示要监听的 fd 集合
    int max_fd;                           // 当前监控的文件描述符的最大值, 用于 select()
    fd_set active_fd_set;                 // 当前激活的文件描述符集合,用于 select()
    fd_set backup_fd_set;                 // 文件描述符的 备份集合
    pthread_t *client_svc_mgr_thread;     // 客户端服务器管理线程
    std::list<TcpClient *> tcp_client_db; // 客户端集合

    void CopyClientFDtoFDSet(fd_set *fdset);
    int GetMaxFd();

public:
    TcpServerController *tcp_ctrlr{};

    TcpClientServiceManager(TcpServerController *);
    ~TcpClientServiceManager();

    void StartTcpClientServiceManagerThread();
    void StartTcpClientServiceManagerThreadInternal();
    void StopTcpClientServiceManagerThread();
    void ClientFDStartListen(TcpClient *tcp_client);
    void AddClientToDB(TcpClient *);
};
#endif