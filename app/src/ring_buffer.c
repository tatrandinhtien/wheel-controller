/**
 * @file ring_buffer.c
 * @author Tien Ta (tien.ta.eswe@gmail.com)
 * @brief Basic ring buffer (get 1 byte - put 1 byte)
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "ring_buffer.h"


bool rb_init(ring_buffer_t *rb, uint8_t *data, uint16_t len) {
    if (rb == NULL || data == NULL) {
        return false;
    }

    if ((len & (len - 1)) != 0) {
        return false;
    }

    rb->pdata = data;
    rb->head = 0;
    rb->tail = 0;
    rb->capacity = len;

    return true;
}

bool rb_is_empty(ring_buffer_t *rb) {
    if (rb == NULL) {
        return false;
    }

    return rb->head == rb->tail;
}

bool rb_is_full(ring_buffer_t *rb) {
    if (rb == NULL) {
        return false;
    }

    return ((rb->head + 1) & (rb->capacity - 1)) == rb->tail;
}

bool rb_get(ring_buffer_t *rb, uint8_t *data) {
    if (rb == NULL) {
        return false;
    }

    if (rb->head == rb->tail) {
        return false;
    }

    *data = rb->pdata[rb->tail];
    rb->tail = ((rb->tail + 1) & (rb->capacity - 1));

    return true;
}

bool rb_put(ring_buffer_t *rb, uint8_t data) {
    if (rb == NULL) {
        return false;
    }

    if (((rb->head + 1) & (rb->capacity - 1)) == rb->tail) {
        return false;
    }

    rb->pdata[rb->head] = data;
    rb->head = (rb->head + 1) & (rb->capacity - 1);
    return true;
}