#include <sys/socket.h>
#include <netinet/in.h> // for IPPROTO_TCP
#include <memory.h>
#include "TcpServerController.h"
#include "TcpNewConnectionAcceptor.h"
#include "network_utils.h"
#include "TcpClient.h"
#include "TcpMsgFixedSizeDemarcar.h"

/**
 * @brief Construct a new Tcp New Connection Acceptor:: Tcp New Connection Acceptor object
 *
 * 1.创建一个监听套接字, 用于等待客户端连接
 *
 * 2.分配一个线程对象指针
 *
 * 3.记录所属的服务器控制器指针(TcpServerController*)
 * @param tcp_ctrlr
 *
 */
TcpNewConnectionAcceptor::TcpNewConnectionAcceptor(TcpServerController *tcp_ctrlr)
{
    this->accept_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (this->accept_fd < 0)
    {
        printf("Error: Could not create Accept FD\n");
        exit(0);
    }

    this->accept_new_conn_thread = (pthread_t *)calloc(1, sizeof(pthread_t)); // calloc () 会将 内存初始化为 0
    this->tcp_ctrlr = tcp_ctrlr;
}

TcpNewConnectionAcceptor::~TcpNewConnectionAcceptor()
{
}

/**
 * @brief 线程入口函数
 *
 * @param arg 线程函数的参数
 * @return void*
 */
static void *tcp_listen_for_new_connections(void *arg)
{
    TcpNewConnectionAcceptor *tcp_new_conn_acc = (TcpNewConnectionAcceptor *)arg;

    tcp_new_conn_acc->StartTcpNewConnectionAcceptorThreadInternal();

    return nullptr;
}

/**
 * 1.Start a thread
 * 2.Create an infinite loop
 * 3.invoke accept() to accept new connections
 * 4.Notify the application for new connections
 */

/**
 * @brief 启动监听线程
 *
 * 1.创建一个独立的 POSIX 线程
 *
 * 2.在新线程中执行 tcp_listen_for_new_connections() 函数
 *
 * 3.这个线程负责监听 socket, 处理客户端连接
 *
 */
void TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThread()
{
    if (pthread_create(this->accept_new_conn_thread, nullptr, tcp_listen_for_new_connections, (void *)this))
    {
        printf("%s() Thread Creation failed, error=%d\n", __FUNCTION__, errno);
        exit(0);
    }

    printf("Service Started: TcpNewConnectionAcceptorThread\n");
}

/**
 * @brief 监听函数:实现监听与接收逻辑的内部函数
 *
 * 1.设置 socket 选项 SO_REUSEADDR, SO_REUSEPORT
 *
 * 2.bind() 绑定socket 与本地 IP/端口
 *
 * 3.listen() 开始监听
 *
 * 4.进入循环, accept() 等待客户端连接
 *
 * 5.每当有连接,就打印出客户端IP与端口
 *
 */
void TcpNewConnectionAcceptor::StartTcpNewConnectionAcceptorThreadInternal()
{
    int opt = 1;
    struct sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->tcp_ctrlr->port_no);
    server_addr.sin_addr.s_addr = htonl(this->tcp_ctrlr->ip_addr);

    // 设置允许服务器程序重启时，立即重新绑定(bind) 到同一个端口，而不会报错
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setSockopt Failed\n");
        exit(0);
    }
    //  设置允许多个 socket（进程或线程）同时绑定同一个端口
    if (setsockopt(this->accept_fd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("setSockopt Failed\n");
        exit(0);
    }

    // let us bind the socket now
    if (bind(this->accept_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("Error: Acceptor socket bind failed [%s(0x%x).%d].error = %d\n",
               network_convert_ip_n_to_p(this->tcp_ctrlr->ip_addr, 0),
               this->tcp_ctrlr->ip_addr,
               this->tcp_ctrlr->port_no,
               errno);
        exit(0);
    }

    if (listen(this->accept_fd, 5) < 0)
    {
        printf("listen failed\n");
        exit(0);
    }

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int comm_sock_fd;

    while (true)
    {
        comm_sock_fd = accept(this->accept_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (comm_sock_fd < 0)
        {
            printf("Error in Accepting New Connections\n");
            continue;
        }

        TcpClient *tcp_client = new TcpClient(client_addr.sin_addr.s_addr, client_addr.sin_port);

        tcp_client->tcp_ctrlr = this->tcp_ctrlr;
        tcp_client->comm_fd = comm_sock_fd;

        if (this->tcp_ctrlr->client_connected)
        {
            this->tcp_ctrlr->client_connected(this->tcp_ctrlr, tcp_client);
        }

        tcp_client->msgd = new TcpMsgFixedSizeDemarcar(27);

        // tell the TCP Controller. to furtjer process the Client
        this->tcp_ctrlr->ProcessNewClient(tcp_client);

        printf("Connection Acceptted from Client [%s.%d]\n",
               network_convert_ip_n_to_p(htonl(client_addr.sin_addr.s_addr), 0),
               htons(client_addr.sin_port));
    }
}
