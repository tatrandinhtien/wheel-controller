/**
 * @file    ring_buffer.c
 * @author  DinhTien (tien.ta.eswe@gmail.com)
 * @brief   Basic ring buffer (get 1 byte - put 1 byte) implementation.
 * @version 1.0
 * @date    2026-05-17
 * * @copyright Copyright (c) 2026
 * */

#include "ring_buffer.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

bool rb_init(ring_buffer_t *rb, uint8_t *data, uint16_t len) {
    if (rb == NULL || data == NULL) {
        return false;
    }

    /* Bitwise Check: Validate if size format conforms strictly to Power-of-2 dimensions */
    if ((len & (len - 1)) != 0) {
        return false;
    }

    /* Map hardware tracking metrics boundaries */
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

    /* Check if next incremental write slot wraps onto current read tracker positions */
    return ((rb->head + 1) & (rb->capacity - 1)) == rb->tail;
}

bool rb_get(ring_buffer_t *rb, uint8_t *data) {
    if (rb == NULL || data == NULL) {
        return false;
    }

    if (rb->head == rb->tail) {
        return false;
    }

    /* Fetch byte sequence straight out from current read tracking index slot */
    *data = rb->pdata[rb->tail];
    
    /* Incremental wrap shift using optimal bit mask instead of modulo commands */
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

    /* Insert source byte sequence directly onto current write trailing edge */
    rb->pdata[rb->head] = data;
    
    /* Shift write index forward inside boundary masks safely */
    rb->head = (rb->head + 1) & (rb->capacity - 1);
    
    return true;
}