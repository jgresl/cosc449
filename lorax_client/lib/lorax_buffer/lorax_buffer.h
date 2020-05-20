/**
 * @file lorax_buffer.h
 * @author Ethan Godden
 * @brief A structure for storing data. This is used within sensor nodes to store samples using FIFO
 * @version 0.1
 * @date 2020-05-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef SERIAL_BYTE_QUEUE_H
#define SERIAL_BYTE_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Structure for storing sample data
 * 
 */
struct lorax_buffer
{
    uint8_t *__data;
    uint8_t __element_size; //Size of each element in bytes
    uint16_t __head;        // Index of next element. The address of this element is __data + (__head * element_size)
    uint16_t __size;        //Number of elements currently in the buffer. The number of bytes in the buffer is (__size * element_size)
    uint16_t __capacity;    //Max number of elements that can be stored in the buffer
};

/**
 * @brief Create a malloced structure for storing data during runtime
 * 
 * @param element_size the size of each element in bytes that will be stored in the buffer
 * @param capacity the maximum number of elements that can be stored in the buffer
 * @return lorax_buffer* a reference to the structure in the heap
 */
lorax_buffer *create_buffer(uint8_t element_size, uint16_t capacity);

bool store_sample(lorax_buffer *buffer, uint8_t *sample);

bool peek_sample(lorax_buffer *buffer, uint8_t *removed_element);

bool remove_sample(lorax_buffer *buffer, uint8_t *removed_element);

uint8_t* peek_next_sample(lorax_buffer *buffer);

bool is_full(lorax_buffer *buffer);

bool resize(lorax_buffer *buffer, uint16_t new_size);

void print_buffer(lorax_buffer *buffer);

void free_buffer(lorax_buffer *buffer);

#endif
