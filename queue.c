#include <stdio.h>
struct queue
{
	__uint8_t *buffer;
	unsigned long length;
	unsigned long left;
	unsigned long right;
	unsigned long offset;
	unsigned long elements;
};
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
void shiftLeft(struct queue *queue)
{
	if (queue->offset)
		queue->offset--;
}
__uint8_t dequeue(struct queue *queue)
{
	if (isEmpty(queue) || !isLeftAligned(queue))
		return 0;
	__uint8_t element = queue->buffer[queue->left];
	queue->left = (queue->left + 1) % queue->length;
	queue->elements--;
	queue->offset++;
	return element;
}
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