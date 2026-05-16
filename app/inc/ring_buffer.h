#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	uint8_t *pdata;
	volatile uint16_t head;
	volatile uint16_t tail;
	uint16_t capacity;
} ring_buffer_t;

bool rb_init(ring_buffer_t *rb, uint8_t *data, uint16_t len);
bool rb_is_empty(ring_buffer_t *rb);
bool rb_is_full(ring_buffer_t *rb);
bool rb_get(ring_buffer_t *rb);
bool rb_put(ring_buffer_t *rb, uint8_t data);


#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H_ */