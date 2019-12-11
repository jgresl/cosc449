#define MAX_QUEUE_SIZE 1000                                   //Max number of elements that can be stored in the queue before the program crashes
#define calculateNewCapacity(oldCapacity) 2 * oldCapacity + 1 //Formula for resizing the queue when full
#define byte char
typedef struct
{
    short head, size, capacity, sizeOfDataElement;
    void *data;
} Queue;

/**
 * Creates a malloced Queue with an inital capacity which can store elements of size sizeOfDataElement
 */
Queue *createQueue(short sizeOfDataElement, short capacity);
/**
 * Adds the value pointed to by pointerToNewElement to the end of the queue
 */
void enqueue(Queue *pointerToQueue, void *pointerToNewElement);
/**
 * Removes the value at the beginning of the queue and stores it in the memory location 
 * memoryLocationForElementRemoved
 */
void dequeue(Queue *pointerToQueue, void *memoryLocationForElementRemoved);
/**
 * Retrieves, but does not remove, the value at the beginning of the queue and stores it in the memory location 
 * memoryLocationForElementRemoved
 */
void first(Queue *pointerToQueue, void *memoryLocationForElementRetrieved);

/**
 * Invokes the function on each element of the queue from the beginning to the end of the queue
 */
void for_each(Queue *pointerToQueue, void (*functionToCall)(void *));
