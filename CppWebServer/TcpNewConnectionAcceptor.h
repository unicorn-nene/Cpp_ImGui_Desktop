#ifndef __TPCNEWCONNECTIONACCEPTOR__
#define __TPCNEWCONNECTIONACCEPTOR__
#include <pthread.h>
#include "TcpServerController.h"

class TcpServerController;

class TcpNewConnectionAcceptor
{
private:
    int accept_fd{};
    pthread_t *accept_new_conn_thread{};

public:
    TcpServerController *tcp_ctrlr{};

    TcpNewConnectionAcceptor(TcpServerController *);
    ~TcpNewConnectionAcceptor();

    void StartTcpNewConnectionAcceptorThread();
    void StartTcpNewConnectionAcceptorThreadInternal();
};

#endif