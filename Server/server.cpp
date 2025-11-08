#include <arpa/inet.h>    // socket(), bind(), accept(), listen()
#include <unistd.h>       // close(), write(), read()
#include <semaphore.h>    // sem_t() 信号量
#include <pthread.h>      // pthread()
#include <sys/stat.h>     // struct stat 是用于获取文件属性（大小、时间、权限等）的结构体
#include <fcntl.h>        // open()
#include <sys/sendfile.h> // sendfile()

#include <iostream>
#include <cstring>
#include <vector>

#include "server.h"

using namespace std;

#define client_message_SIZE 1024
#define PORT 8080
sem_t mutex;
int thread_count = 0;
std::vector<std::string> serverData{};

/**
 * @brief 从字符中提取指定分隔符前的子字符串
 *
 * @param sql 要处理的字符串
 * @param end 结束的分隔符
 * @return string
 */
string getStr(string sql, char end)
{
    int counter = 0;
    string retStr = "";

    while (sql[counter] != '\0')
    {
        if (sql[counter] == end)
            break;

        retStr += sql[counter];
        ++counter;
    }

    return retStr;
}

/**
 * @brief 根据文件扩展名,返回对应的 Content-Type
 *
 * @param fileEx  要查找的文件扩展名
 * @return std::string 对应的 Content-Type HTTP 头部字符串
 */
std::string findFileExt(std::string fileEx)
{
    for (int i = 0; i < sizeof(fileExtension) / sizeof(fileExtension[0]); ++i)
    {
        if (fileExtension[i] == fileEx)
            return ContentType[i];
    }

    printf("serveing .%s as html\n", fileEx.c_str());

    return "Content-Type: text/html\r\n\r\n";
}

/**
 * @brief 向客户端发送 HTTP 响应头和指定文件内容
 *
 * 该函数实现一个简单的 HTTP 文件响应：
 * 1. 发送 HTTP 响应头（例如 200 OK + Content-Type）
 * 2. 再发送指定路径的文件内容
 *
 * @param fd            客户端套接字文件描述符，用于 write/send
 * @param filePath      客户端请求的文件路径，例如 "/index.html"
 * @param headerFile    Content-Type 对应字符串，例如 "Content-Type: text/html\r\n\r\n"
 */
void send_message(int fd, string filePath, string headerFile)
{
    // ---------- [1] 构建 HTTP 响应头 ----------
    string header = Messages[HTTP_HEADER] + headerFile;
    if (filePath == "/" || filePath == "." || filePath.empty())
        filePath = "/index.html"; // 默认首页
    filePath = "./public" + filePath;

    struct stat stat_buf;

    // ---------- [2] 发送响应头 ----------
    ssize_t bytes_written = write(fd, header.c_str(), header.length());
    if (bytes_written < 0)
    {
        perror("[Error] Failed to send HTTP header");
        return;
    }
    else if ((size_t)bytes_written < header.length())
    {
        fprintf(stderr, "[Warn] HTTP header only partially sent (%zd/%zu bytes)\n", bytes_written, header.length());
    }

    // ---------- [3] 打开文件 ----------
    int fdimg = open(filePath.c_str(), O_RDONLY);
    if (fdimg < 0)
    {
        fprintf(stderr, "[Error] Cannot open file: %s (%s)\n", filePath.c_str(), strerror(errno));
        return;
    }

    // ---------- [4] 获取文件信息 ----------
    if (fstat(fdimg, &stat_buf) < 0)
    {
        fprintf(stderr, "[Error] fstat() failed for %s (%s)\n", filePath.c_str(), strerror(errno));
        close(fdimg);
        return;
    }
    // 检查是否是普通文件
    if (!S_ISREG(stat_buf.st_mode))
    {
        fprintf(stderr, "[Error] Not a regular file: %s\n", filePath.c_str());
        close(fdimg);
        return;
    }

    int img_total_size = stat_buf.st_size;
    int block_size = stat_buf.st_blksize;
    if (img_total_size <= 0)
    {
        fprintf(stderr, "[Warn] File is empty: %s\n", filePath.c_str());
        close(fdimg);
        return;
    }

    if (block_size <= 0)
    {
        block_size = 4096; // fallback 默认块大小
    }

    // ---------- [5] 发送文件内容 ----------
    if (fdimg >= 0)
    {
        size_t sent_size = 0;
        while (img_total_size > 0)
        {
            int send_bytes = ((img_total_size < block_size) ? img_total_size : block_size);
            int done_bytes = sendfile(fd, fdimg, nullptr, send_bytes);

            if (done_bytes < 0)
            {
                if (errno == EINTR)
                {
                    fprintf(stderr, "[Warn] sendfile interrupted, retrying...\n");
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    fprintf(stderr, "[Warn] sendfile temporarily unavailable, retrying...\n");
                    usleep(1000);
                    continue;
                }
                else
                {
                    fprintf(stderr, "[Error] sendfile() failed: %s\n", strerror(errno));
                    break;
                }
            }
            else if (done_bytes == 0)
            {
                fprintf(stderr, "[Warn] sendfile() returned 0, possible EOF reached early\n");
                break;
            }

            img_total_size -= done_bytes;
            sent_size += done_bytes;
        }

        if (sent_size >= 0)
            printf("sent file: %s (total %zu bytes)\n", filePath.c_str(), sent_size);
        else
            fprintf(stderr, "[Error] sendfile logic error (sent_size < 0)\n");

        close(fdimg);
    }
}

/**
 * @brief 解析 HTTP 请求中的数据参数(GET / POST / Cookie)
 *
 * 根据请求类型（GET 或 POST）从客户端请求报文中提取参数数据，
 * 并将解析出的键值对（例如 "username=Tom"）存入全局容器 serverData。
 *
 * @param requestType       请求类型，例如 "GET" 或 "POST"
 * @param client_message    客户端发送的完整 HTTP 请求报文
 */
void getData(const string &requestType, const string &client_message)
{
    string data;        // 用于存储参数
    serverData.clear(); // 清空上一次的数据

    // ---------- [1] 处理 GET 请求 ----------
    if (requestType == "GET")
    {
        // 提取请求行
        size_t first_space = client_message.find(' ');
        size_t second_space = client_message.find(' ', first_space + 1);
        if (first_space != string::npos && second_space != string::npos)
        {
            // 取出路径部分，例如 "/index.html?username=Tom&age=18"
            string path = client_message.substr(first_space + 1, second_space - first_space - 1);

            // 查找 query 字符串
            size_t qmark = path.find('?');
            if (qmark != string::npos)
                data = path.substr(qmark + 1); // 提取 "username=Tom&age=18"
        }
        // 否则 data 保持为空
    }

    // ---------- [2] 处理 POST 请求 ----------
    else if (requestType == "POST")
    {
        // 查找 HTTP 消息体（简单做法：从末尾找到第一段非空白字符）
        size_t body_start = client_message.rfind("\r\n\r\n");
        if (body_start != string::npos && body_start + 4 < client_message.size())
            data = client_message.substr(body_start + 4);

        // 如果没有 '=', 说明没有有效参数
        if (data.find('=') == string::npos)
            data.clear();
    }

    // ---------- [3] 检查 Cookie ----------
    size_t cookie_pos = client_message.find("Cookie:");
    if (cookie_pos != string::npos)
    {
        size_t line_end = client_message.find("\r", cookie_pos);
        string cookie_str;
        if (line_end != string::npos)
            cookie_str = client_message.substr(cookie_pos + 7, line_end - (cookie_pos + 7));
        else
            cookie_str = client_message.substr(cookie_pos + 7);

        if (!data.empty())
            data += "&";
        data += cookie_str;
    }

    // ---------- [4] 拆分 data 中的参数 ----------
    size_t pos = 0;
    while (!data.empty())
    {
        pos = data.find('&');
        string extract;
        if (pos != string::npos)
        {
            extract = data.substr(0, pos);
            data.erase(0, pos + 1);
        }
        else
        {
            extract = data;
            data.clear();
        }

        if (!extract.empty())
            serverData.push_back(extract);
    }
}

void *connection_handler(void *socket_desc)
{
    if (socket_desc == nullptr)
    {
        perror("null socket_desc");
        pthread_exit(nullptr);
    }

    int newSock = *((int *)socket_desc);                                 // 客户端套接字描述符
    char client_message[client_message_SIZE] = {0};                      // 缓冲区初始化
    int request = read(newSock, client_message, sizeof(client_message)); // 读取 HTTP 请求
    string message = client_message;

    if (request < 0)
    {
        perror("read() failed");
        close(newSock);
        pthread_exit(nullptr);
    }

    if (request == 0)
    {
        puts("Client disconnected unexpectedly");
        close(newSock);
        pthread_exit(nullptr);
    }

    // === [1] 增加线程计数 ===
    sem_wait(&mutex);
    thread_count++;
    printf("thread counter %d\n", thread_count);
    if (thread_count > 20)
    {
        write(newSock, Messages[BAD_REQUEST].c_str(), Messages[BAD_REQUEST].length());
        thread_count--;
        close(newSock);
        sem_post(&mutex);
        pthread_exit(nullptr);
    }
    sem_post(&mutex);

    // === [2] 检查 multipart/form-data 文件上传请求 ===
    string mess = client_message;
    if (mess.find("multipart/form-data") != string::npos)
    {
        size_t found = mess.find("Content-Length:");
        if (found != string::npos)
        {
            mess.erase(0, found + 16);
            int length = stoi(getStr(mess, ' '));

            found = mess.find("filename=");
            if (found != string::npos)
            {
                mess.erase(0, found + 10);
                string newf = getStr(mess, '"');
                newf = "./public/downloads/" + newf;

                found = mess.find("Content-Type:");
                if (found != string::npos)
                {
                    mess.erase(0, found + 15);
                    mess.erase(0, getStr(mess, '\n').length() + 3);
                }

                int fd = open(newf.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                    perror(("cannot open file for upload: " + newf).c_str());
                else
                {
                    write(fd, mess.c_str(), mess.size());
                    printf("filesize: %d\n", length);

                    char client_mess[client_message_SIZE];
                    int counter = 0;
                    while (length > 0)
                    {
                        int req = read(newSock, client_mess, sizeof(client_mess));
                        if (req <= 0)
                            break;
                        if (write(fd, client_mess, req) < 0)
                        {
                            perror("write upload failed");
                            break;
                        }
                        length -= req;
                        counter += req;
                        printf("remains: %d, received: %d, total: %d\n", length, req, counter);
                        if (req < 1000)
                            break;
                    }
                    close(fd);
                }
            }
        }
    }

    // === [3] 解析 HTTP 请求 (GET / POST) ===
    string requestType = getStr(message, ' ');
    message.erase(0, requestType.length() + 1);
    string requestFile = getStr(message, ' ');

    // ---------- 【修改部分：清理路径并处理根目录】 ----------
    // 去掉结尾控制符
    while (!requestFile.empty() && (requestFile.back() == '\r' || requestFile.back() == '\n'))
        requestFile.pop_back();

    // 根路径映射到 index.html
    if (requestFile.empty() || requestFile == "/" || requestFile == "." || requestFile == "./")
        requestFile = "/index.html";

    // ---------- 提取扩展名 ----------
    string fileExt;
    size_t dotPos = requestFile.rfind('.');
    if (dotPos != string::npos)
        fileExt = requestFile.substr(dotPos + 1);
    else
        fileExt = "";

    // ---------- 处理 GET / POST 请求 ----------
    if (requestType == "GET" || requestType == "POST")
    {
        if (fileExt == "php")
            getData(requestType, client_message);

        sem_wait(&mutex);
        send_message(newSock, requestFile, findFileExt(fileExt));
        sem_post(&mutex);
    }

    printf("\n---- exiting server thread ----\n");

    close(newSock);
    sem_wait(&mutex);
    thread_count--;
    sem_post(&mutex);

    pthread_exit(nullptr);
    return nullptr;
}

int main(int argc, char *argv[])
{
    int server_fd, client_sock, c;
    struct sockaddr_in server, client;
    pthread_t thread_id;

    // === [1] 初始化信号量 ===
    if (sem_init(&mutex, 0, 1) != 0)
    {
        perror("sem_init failed");
        return 1;
    }

    // === [2] 创建 socket ===
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Could not create socket");
        return 1;
    }
    puts("Socket created");

    // === [3] 设置服务器地址 ===
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // === [4] 绑定 socket ===
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        return 1;
    }
    puts("bind done");

    // === [5] 开始监听 ===
    if (listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);

    // === [6] 主循环：接受客户端 ===
    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        printf("Connected to %s\n", inet_ntoa(client.sin_addr));

        int *new_sock = new int;
        *new_sock = client_sock;

        if (pthread_create(&thread_id, nullptr, connection_handler, (void *)new_sock) < 0)
        {
            perror("could not create thread");
            delete new_sock;
            continue;
        }

        pthread_detach(thread_id); // 自动回收资源
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        close(server_fd);
        return 1;
    }

    close(server_fd);
    sem_destroy(&mutex);

    return 0;
}
