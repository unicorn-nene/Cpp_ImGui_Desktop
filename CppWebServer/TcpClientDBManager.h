#ifndef __TCPCLIENTDBMANAGER__
#define __TCPCLIENTDBMANAGER__

#include <list>

class TcpClient;
class TcpServerController;

/**
 * @brief
 *
 */
class TcpClientDBManager
{
private:
    std::list<TcpClient *> tcp_client_db{};

public:
    TcpServerController *tcp_ctrlr{};

    TcpClientDBManager(TcpServerController *);
    ~TcpClientDBManager();

    void StartTcpClientDbMgrInit();
    void AddClientToDB(TcpClient *tcp_client);
    void DisplayClientDb();
};

#endif