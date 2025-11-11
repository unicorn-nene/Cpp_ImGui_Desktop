#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include "network_utils.h"

/**
 * @brief 将网络字节序(IPv4地址,32位无符号整数)转换为点分十进制字符串形式,
 * 将 IPv4 地址 转换为 字符串形式(192.168.1.1)
 * @param ip_addr 网络字节序
 * @param output_buffer 外部字符串缓冲区
 * @return char* 指向点分十进制字符串的指针, 如果提供了外部字符串缓冲区,则返回外部缓冲区的指针
 */
char *
network_convert_ip_n_to_p(uint32_t ip_addr,
                          char *output_buffer)
{

    char *out = NULL;
    static char str_ip[16];
    out = !output_buffer ? str_ip : output_buffer;
    memset(out, 0, 16);
    ip_addr = htonl(ip_addr);
    inet_ntop(AF_INET, &ip_addr, out, 16);
    out[15] = '\0';
    return out;
}

/**
 * @brief 将点分十进制字符串转换为网络字节序(IPv4地址)32位无符号整数
 *
 * @param ip_addr 点分十进制字符串
 * @return uint32_t 转换后的网络字节序
 */
uint32_t
network_covert_ip_p_to_n(const char *ip_addr)
{

    uint32_t binary_prefix = 0;
    inet_pton(AF_INET, ip_addr, &binary_prefix);
    binary_prefix = htonl(binary_prefix);
    return binary_prefix;
}