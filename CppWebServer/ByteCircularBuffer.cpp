#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ByteCircularBuffer.h"

/**
 * @brief 创建一个新的 ByteCircularBuffer
 *
 * @param size buffer 大小
 * @return ByteCircularBuffer_t* 分配并初始化完成的 buffer 对象
 */
ByteCircularBuffer_t *BCBCreateNew(uint16_t size)
{
    ByteCircularBuffer_t *bcb = (ByteCircularBuffer_t *)calloc(1, sizeof(ByteCircularBuffer_t));

    bcb->buffer_size = size;
    bcb->buffer = (unsigned char *)calloc(size, sizeof(unsigned char));
    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;

    return bcb;
}

/**
 * @brief 释放ByteCircularBuffer对象
 *
 * @param bcb 要释放的 ByteCircularBuffer 对象指针
 */
void BCBFree(ByteCircularBuffer_t *bcb)
{
    free(bcb->buffer);
    free(bcb);
}

/**
 * @brief 计算 ByteCircularBuffer当前剩余可用字节数
 *
 * @param bcb ByteCircularBuffer指针
 * @return uint16_t 剩余可用字节数
 */
uint16_t BCBAvailableSize(ByteCircularBuffer_t *bcb)
{
    return bcb->buffer_size - bcb->current_size;
}

/**
 * @brief 向 ByteCircularBuffer写入数据
 *
 * @param bcb 要写入的 ByteCircularBuffer
 * @param data 要写入的字节数组
 * @param data_size 写入的字节数
 * @return uint16_t 实际写入的字节数
 */
uint16_t BCBWrite(ByteCircularBuffer_t *bcb, unsigned char *data, uint16_t data_size)
{
    if (BCBIsFull(bcb))
        return 0;
    if (BCBAvailableSize(bcb) < data_size)
        return 0;

    // 情况1：没有环绕（front < rear），可直接连续写入
    if (bcb->front < bcb->rear)
    {
        memcpy(BCB(bcb, bcb->front), data, data_size);
        bcb->front += data_size;

        // 如果写入到结尾, 循环到 0
        if (bcb->front == bcb->buffer_size)
            bcb->front = 0;
        bcb->current_size += data_size;

        return data_size;
    }

    // 情况2: front >= rear 可能需要两段写入
    uint16_t leading_space = bcb->buffer_size - bcb->front;

    // 2.1 如果当前一段空间足够, 直接写入
    if (data_size <= leading_space)
    {
        memcpy(BCB(bcb, bcb->front), data, data_size);
        bcb->front += data_size;

        if (bcb->front == bcb->buffer_size)
            bcb->front = 0;
        bcb->current_size += data_size;

        return data_size;
    }

    // 2.2 当前一段空间不够,先写到结尾,再从 0(下一段) 开始写剩余内容
    memcpy(BCB(bcb, bcb->front), data, leading_space);
    memcpy(BCB(bcb, 0), data + leading_space, data_size - leading_space);

    bcb->front += data_size - leading_space;
    bcb->current_size += data_size;

    return data_size;
}

/**
 * @brief  从 ByteCircularBuffer读取数据
 *
 * @param bcb ByteCircularBuffer 指针
 * @param buffer 外部缓冲区指针
 * @param data_size 读取的字节数
 * @param remove_read_bytes 是否在读取之后删除以读取字节
 * @return uint16_t 已读取的字节数
 */
uint16_t BCBRead(ByteCircularBuffer_t *bcb, unsigned char *buffer, uint16_t data_size, bool remove_read_bytes)
{
    if (bcb->current_size < data_size)
        return 0;

    if (bcb->rear < bcb->front)
    {
        memcpy(buffer, BCB(bcb, bcb->rear), data_size);

        if (remove_read_bytes)
        {
            bcb->rear += data_size;
            if (bcb->rear == bcb->current_size)
                bcb->rear = 0;
            bcb->current_size -= data_size;
        }

        return data_size;
    }

    // 情况2：rear >= front，可能需要分两段读
    uint16_t leading_space = bcb->buffer_size - bcb->rear;

    // 2.1 一段足够读完
    if (data_size <= leading_space)
    {
        memcpy(buffer, BCB(bcb, bcb->rear), data_size);

        if (remove_read_bytes)
        {
            bcb->rear += data_size;
            if (bcb->rear == bcb->buffer_size)
                bcb->rear = 0;
            bcb->current_size -= data_size;
        }
        return data_size;
    }

    // 2.2 两端读取
    memcpy(buffer, BCB(bcb, bcb->rear), leading_space);
    memcpy(buffer + leading_space, BCB(bcb, 0), data_size - leading_space);
    if (remove_read_bytes)
    {
        bcb->rear = data_size - leading_space;
        bcb->current_size -= data_size;
    }

    return data_size;
}

/**
 * @brief  判断 ByteCircularBuffer是否已满
 *
 * @param bcb
 * @return true
 * @return false
 */
bool BCBIsFull(ByteCircularBuffer_t *bcb)
{
    return bcb->current_size == bcb->buffer_size;
}

/**
 * @brief 重置缓冲区
 *
 * @param bcb
 */
void BCBReset(ByteCircularBuffer_t *bcb)
{
    bcb->current_size = 0;
    bcb->front = 0;
    bcb->rear = 0;
}

void BCBPrintSnapshot(ByteCircularBuffer_t *bcb)
{
}