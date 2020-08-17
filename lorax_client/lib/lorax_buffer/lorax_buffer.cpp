#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include "lorax_buffer.h"
#include <Arduino.h>


//TODO: Implement checks for illegal args - possibly add error codes
lorax_buffer *create_buffer(uint8_t element_size, uint16_t capacity)
{
    lorax_buffer *new_buffer = (lorax_buffer *)malloc(sizeof(lorax_buffer));

    //If allocation for buffer succeeds
    if (new_buffer != NULL)
    {

        uint8_t *data = (uint8_t *)malloc(capacity * element_size);

        //If allocation of the data portion suceeds
        if (data != NULL)
        {
            //Initialize buffer attributes
            new_buffer->__element_size = element_size;
            new_buffer->__head = new_buffer->__size = 0;
            new_buffer->__capacity = capacity;
            new_buffer->__data = data;
        }
        else
        {
            //Free previously allocated space because data allocation failed
            free(new_buffer);
        }
    }
    return new_buffer; //Will be null if allocation fails
}

bool store_sample(lorax_buffer *buffer, uint8_t *sample)
{
    if (is_full(buffer))
    {
        return false;
    }
    else
    {
        uint8_t element_size = buffer->__element_size;
        uint16_t index = (buffer->__head + buffer->__size++) % buffer->__capacity;
        uint8_t *address_of_new_element = buffer->__data + (index * element_size);
        for (uint8_t i = 0; i < element_size; i++)
        {
            address_of_new_element[i] = sample[i];
        }
        return true;
    }
}

bool peek_sample(lorax_buffer *buffer, uint8_t *removed_element)
{
    if (buffer->__size == 0)
    {
        return false;
    }
    else
    {
        uint8_t element_size = buffer->__element_size;
        uint8_t *address_of_head = buffer->__data + (buffer->__head * element_size);
        for (int i = 0; i < element_size; i++)
        {
            removed_element[i] = address_of_head[i];
        }
        return true;
    }
}

bool remove_sample(lorax_buffer *buffer, uint8_t *removed_element)
{
    if (buffer->__size == 0)
    {
        return false;
    }
    else
    {
        uint8_t element_size = buffer->__element_size;
        uint8_t *address_of_head = buffer->__data + (buffer->__head * element_size);
        for (int i = 0; i < element_size; i++)
        {
            removed_element[i] = address_of_head[i];
        }
        buffer->__size--;
        buffer->__head = (buffer->__head + 1) % buffer->__capacity;
        return true;
    }
}

bool is_full(lorax_buffer *buffer)
{
    return buffer->__size == buffer->__capacity;
}

bool resize(lorax_buffer *buffer, uint16_t new_size)
{
    uint16_t old_size = buffer->__size;
    if (new_size < old_size)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void print_buffer(lorax_buffer *buffer)
{
    uint8_t *data = buffer->__data, element_size = buffer->__element_size;
    uint16_t head = buffer->__head, size = buffer->__size, capacity = buffer->__capacity;

    for (uint16_t i = 0; i < size; i++)
    {
        uint16_t index = (head + i) % capacity;
        uint8_t *address = data + (index * element_size);
        uint8_t *element_bytes = (uint8_t *)alloca(element_size);
        for (uint8_t j = 0; j < element_size; j++)
        {
            element_bytes[j] = address[j];
        }
        const char *printf_string = "Element %d: ";
        Serial.printf(printf_string, i);
        Serial.print((char*) element_bytes);
        Serial.println();
    }
    Serial.println();
}

void free_buffer(lorax_buffer *buffer)
{
    free(buffer->__data);
    free(buffer);
}