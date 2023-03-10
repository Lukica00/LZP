#include <stdio.h>
struct queue
{
	__uint8_t *buffer;
	__uint64_t length;
	__uint64_t left;
	__uint64_t right;
	__uint64_t offset;
	__uint64_t elements;
};
int isLeftAligned(struct queue *queue)
{
	return queue->offset == 0;
}
int isEmpty(struct queue *queue)
{
	return queue->elements == 0;
}
int isFull(struct queue *queue)
{
	return queue->elements == queue->length;
}
void shiftLeft(struct queue *queue)
{
	if (queue->offset)
		queue->offset--;
}

void enqueue(struct queue *queue, __uint8_t element)
{
	queue->right = (queue->right + 1) % queue->length;
	if (queue->left == -1)
		queue->left = queue->right;
	if (isFull(queue))
		queue->left = (queue->right + 1) % queue->length;
	queue->buffer[queue->right] = element;
	queue->elements++;
	if (queue->offset)
		queue->offset--;
}
__uint8_t dequeue(struct queue *queue)
{
	__uint8_t element = queue->buffer[queue->left];
	queue->left = (queue->left + 1) % queue->length;
	queue->elements--;
	queue->offset++;
	return element;
}
struct queue *initalizeQueue(__uint64_t length)
{
	struct queue *queue = malloc(sizeof(*queue));
	queue->length = length;
	queue->offset = queue->length;
	queue->left = -1;
	queue->right = -1;
	queue->elements = 0;
	queue->buffer = malloc(sizeof(*queue->buffer) * queue->length);
	return queue;
}

void freeQueue(struct queue *queue)
{
	free(queue->buffer);
	free(queue);
}