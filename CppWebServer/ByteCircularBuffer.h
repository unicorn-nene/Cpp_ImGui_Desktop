#ifndef BYTE_CIRCULAR_BUFFER
#define BYTE_CIRCULAR_BUFFER

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Byte Circluar Buffer
 * 字符级环形缓冲区
 *
 */
typedef struct ByteCircularBuffer_
{
    unsigned char *buffer; // 指向数据缓冲区的指针
    uint16_t buffer_size;  // 缓冲区长度
    uint16_t front;        // 写指针: 下一个数据将要写入的位置索引
    uint16_t rear;         // 读指针: 下一个数据将要读取的位置索引
    uint16_t current_size; // 当前缓冲区中已存储的数据字节数
} ByteCircularBuffer_t;

/**
 * @brief
 * 获取缓冲区第 n 个元素的地址
 */
#define BCB(_bcb, n) (&_bcb->buffer[n])

ByteCircularBuffer_t *BCBCreateNew(uint16_t size);

void BCBFree(ByteCircularBuffer_t *bcb);

uint16_t BCBWrite(ByteCircularBuffer_t *bcb, unsigned char *data, uint16_t data_size);

uint16_t BCBRead(ByteCircularBuffer_t *bcb,
                 unsigned char *buffer,
                 uint16_t data_size,
                 bool remove_read_bytes);

bool BCBIsFull(ByteCircularBuffer_t *bcb);

uint16_t BCBAvailableSize(ByteCircularBuffer_t *bcb);

void BCBReset(ByteCircularBuffer_t *bcb);

void BCBPrintSnapshot(ByteCircularBuffer_t *bcb);

#endif