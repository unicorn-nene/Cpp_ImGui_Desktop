#include <list>
#include "TcpServerController.h"
#include "TcpClientDBManager.h"
#include "TcpClient.h"

TcpClientDBManager::TcpClientDBManager(TcpServerController *tcp_ctrlr)
{
    this->tcp_ctrlr = tcp_ctrlr;
}

TcpClientDBManager::~TcpClientDBManager()
{
}

void TcpClientDBManager::StartTcpClientDbMgrInit()
{
}
void TcpClientDBManager::AddClientToDB(TcpClient *tcp_client)
{
    this->tcp_client_db.push_back(tcp_client);
}

void TcpClientDBManager::DisplayClientDb()
{
    std::list<TcpClient *>::iterator it;
    TcpClient *tcp_client;

    for (it = tcp_client_db.begin(); it != tcp_client_db.end(); ++it)
    {
        tcp_client = *it;
        tcp_client->Display();
    }
}