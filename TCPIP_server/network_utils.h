#ifndef NETWORK_UTILS_H_
#define NETWORK_UTILS_H_
#include <stdint.h>

char *network_convert_ip_n_to_p(uint32_t ip_addr, char *output_buffer);

uint32_t network_convert_ip_p_to_n(const char *ip_addr);

#endif