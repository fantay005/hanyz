#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <stdbool.h>

typedef struct {
	char *buf;                 /*串口缓存的数组指针*/
	char *pin;                 /*当前收到的字符在缓存中的位置*/
	char *pout;                /*读取串口缓存首地址*/
	unsigned int bufferSize;   /*串口缓存数组的大小*/
	unsigned int size;         /*串口缓存数组中已有数据的大小*/
} RingBuffer;

inline void ringBufferInit(RingBuffer *ring, char *buf, int len)
{
	ring->pin = ring->pout = ring->buf = buf;
	ring->bufferSize = len;
	ring->size = 0;
}

inline void ringBufferAppendByte(RingBuffer *ring, char dat)
{
	*ring->pin++ = dat;
	if (ring->pin >= &ring->buf[ring->bufferSize]) ring->pin = ring->buf;

	if (ring->size >= ring->bufferSize) {
		*ring->pout++;
		if (ring->pout >= &ring->buf[ring->bufferSize]) ring->pout = ring->buf;
		return;
	}
	ring->size++;
}

inline bool ringBufferIsEmpty(RingBuffer *ring) {
	return (ring->size == 0);
}

inline int ringBufferGetByte(RingBuffer *ring)
{
	if (ring->size != 0) {
		unsigned char rc;
		rc = *ring->pout++;
		if (ring->pout >= &ring->buf[ring->bufferSize]) ring->pout = ring->buf;
		ring->size--;
		
		return rc;
	}
	return -1;	
}

#endif
