#include <stdio.h>
#include <stdlib.h>
#define FNV_OFFSET 0xcbf29ce484222325
#define FNV_PRIME 0x100000001b3
struct hashtableentry
{
	__uint64_t pointer;
	__uint8_t is;
};
struct hashtable
{
	struct hashtableentry *buffer;
	__uint64_t size;
};
struct hashtableentry getElement(struct hashtable *table, __uint64_t index)
{
	return table->buffer[index % table->size];
}
void setElement(struct hashtable *table, __uint64_t index, struct hashtableentry element)
{
	table->buffer[index % table->size] = element;
}
__uint64_t hashFunction(struct queue *queue, __uint64_t size)
{
	// fnv-1 hash
	__uint64_t hash = FNV_OFFSET;
	for (int i = 0; i < size; i++)
	{
		hash = hash * FNV_PRIME;
		hash = hash ^ queue->buffer[(queue->right - size + 1 + i) % queue->length];
	}
	return hash;
}
struct hashtable *initializeHashtable(__uint64_t size)
{
	struct hashtable *table = malloc(sizeof(*table));
	table->buffer = calloc(sizeof(*table->buffer), size);
	table->size = size;
	return table;
}
void freeHashtable(struct hashtable *table)
{
	free(table->buffer);
	free(table);
}