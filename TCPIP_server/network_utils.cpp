#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include "network_utils.h"

/**
 * @brief 将 IP 地址从网络序整数（N）转换为点分十进制字符串（P）
 *
 * @param ip_addr IPv4 地址
 * @param output_buffer 输入输出缓冲区
 * @return char* 返回存放转换结果的字符串(点分十进制IPv4地址)指针
 */
char *network_convert_ip_n_to_p(uint32_t ip_addr, char *output_buffer)
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
 * @brief 将点分十进制 IP 字符串（P）转换为网络序整数（N）
 *
 * @param ip_addr 点分十进制格式的 IPv4 地址字符串
 * @return uint32_t 转换后的 IPv4 地址（网络字节序）
 *
 */
uint32_t network_convert_ip_p_to_n(const char *ip_addr)
{
    uint32_t binary_prefix = 0;
    inet_pton(AF_INET, ip_addr, &binary_prefix);
    binary_prefix = htonl(binary_prefix);
    return binary_prefix;
}