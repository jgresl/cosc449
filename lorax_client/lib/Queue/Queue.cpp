#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Queue.h"

//Function definitions for private functions (functions only used here)
void __resize(Queue *pointerToQueue, short newCapacity);

/**
 * Create a queue stored in the heap based on the given size of each element and capacity. 
 * Note that if the queue becomes full (size == capacity), the queue will resize if the queue is not
 * at MAX_QUEUE_SIZE 
 */
Queue *createQueue(short sizeOfDataElement, short capacity)
{
    Queue *pointerToNewQueue = (Queue *)malloc(sizeof(Queue));
    pointerToNewQueue->head = pointerToNewQueue->size = 0;

    //size of data poition = capacity * size of data element
    pointerToNewQueue->data = malloc((pointerToNewQueue->capacity = capacity) * (pointerToNewQueue->sizeOfDataElement = sizeOfDataElement));
    return pointerToNewQueue;
}

/**
 * Adds an element to the end of the queue by calculating the address it should be stored at and then copying
 * the element to that location
 * 
 * If the queue is full, this function also resizes the queue according to the formula given by the macro 
 * calculateNewCapacity
 */
void enqueue(Queue *pointerToQueue, void *pointerToNewElement)
{
    //If the queue is full, resize it if possible (if not at the max size given by the macro
    // MAX_QUEUE_SIZE
    if (pointerToQueue->size == pointerToQueue->capacity)
    {
        __resize(pointerToQueue, calculateNewCapacity(pointerToQueue->capacity));
    }
    //If the queue is not full, add it to the end
    //The queue could be still be full even after resizing if the queue was already at max size
    if (pointerToQueue->size < MAX_QUEUE_SIZE)
    {
        //address = address of data + (((head + current size of queue) % capacity) * size of data element)
        void *addressToCopyTo = (byte*)pointerToQueue->data + ((pointerToQueue->head + pointerToQueue->size++) % pointerToQueue->capacity) * pointerToQueue->sizeOfDataElement;
        memcpy(addressToCopyTo, pointerToNewElement, pointerToQueue->sizeOfDataElement);
    }
}

/**
 * Removes the element at the beginning of the queue by copying the head element into the memory location and 
 * then moving head forward one unit
 */
void dequeue(Queue *pointerToQueue, void *memoryLocationForElementRemoved)
{
    //If there are elements to remove
    if (pointerToQueue->size > 0)
    {
        //Copy element at the front to the memory location given
        first(pointerToQueue, memoryLocationForElementRemoved);

        //Calculate the new index of head = (head + 1 % capacity)
        pointerToQueue->head = (pointerToQueue->head + 1) % pointerToQueue->capacity;
        pointerToQueue->size--;
    }
}

/**
 * Retrieves, but does not remove the first element in the queue by first calculating the address of the head 
 * element and copying this element to the location provided
 */
void first(Queue *pointerToQueue, void *memoryLocationForElementRetrieved)
{
    //address = address of data + (current head index * size of data element)
    void *addressOfHead = (byte*)pointerToQueue->data + pointerToQueue->head * pointerToQueue->sizeOfDataElement;

    //copy sizeOfDataElement bytes from address of head to the memory location prodvided
    memcpy(memoryLocationForElementRetrieved, addressOfHead, pointerToQueue->sizeOfDataElement);
}

/**
 * Resizes the given Queue's capacity to the new capacity if the new capacity is greater than or equal to the 
 * current size. If the capacity given is greater than the max possible size of a queue (given by 
 * MAX_QUEUE_SIZE), then the queue is resized to the max possible Queue size
 */
void __resize(Queue *pointerToQueue, short newCapacity)
{
    if (newCapacity >= pointerToQueue->size && newCapacity <= MAX_QUEUE_SIZE)
    {
        //Create a new queue with the desired size
        Queue *pointerToNewQueue = createQueue(pointerToQueue->sizeOfDataElement, newCapacity);

        //For each element in the old queue, place it in a new resized queue
        for (short i = pointerToQueue->head; i < pointerToQueue->head + pointerToQueue->size; i++)
        {
            void *currentAddress = (byte*)pointerToQueue->data + i * pointerToQueue->sizeOfDataElement;
            enqueue(pointerToNewQueue, currentAddress);
        }

        //Point to the new queue
        *pointerToQueue = *pointerToNewQueue;

        //Free memory from buffer for creating queue
        free(pointerToNewQueue);
    }
    else if (newCapacity > MAX_QUEUE_SIZE && pointerToQueue ->size < MAX_QUEUE_SIZE)
    {
        __resize(pointerToQueue, MAX_QUEUE_SIZE);
    }
}

void for_each(Queue *pointerToQueue, void (*functionToCall)(void *))
{
    for (short i = pointerToQueue->head; i < pointerToQueue->head + pointerToQueue->size; i++)
    {
        void *currentAddress = ((byte*)pointerToQueue->data + (i % pointerToQueue->capacity * pointerToQueue->sizeOfDataElement));
        functionToCall(currentAddress);
    }
}
