#include <stdio.h>
#include <stdlib.h>
struct bitbuffer
{
	__uint64_t left;
	__uint64_t right;
	__uint8_t length;
};
struct bitbuffer *initializeBitbuffer()
{
	struct bitbuffer *buffer = malloc(sizeof(*buffer));
	buffer->left = 0;
	buffer->right = 0;
	buffer->length = 0;
}
__uint64_t readBits(struct bitbuffer *buffer, __uint8_t number_of_bits)
{
	// if (number_of_bits > buffer->length)
	// 	number_of_bits = buffer->length;
	if (number_of_bits > 64)
		number_of_bits = 64;
	__uint64_t return_value = buffer->left >> (64 - number_of_bits);
	buffer->left = (buffer->left << number_of_bits) + (buffer->right >> (64 - number_of_bits));
	buffer->right = buffer->right << number_of_bits;
	buffer->length -= number_of_bits;
	if (buffer->length > 128)
		buffer->length = 0;
	return return_value;
}

__uint8_t writeBits(struct bitbuffer *buffer, __uint64_t data, __uint8_t number_of_bits)
{
	if (number_of_bits > 64)
		number_of_bits = 64;
	if (number_of_bits > 128 - buffer->length)
		number_of_bits = 128 - buffer->length;
	if (number_of_bits < 64 - buffer->length)
		buffer->left += data << (64 - buffer->length - number_of_bits);
	else if (buffer->length > 64)
		buffer->right += data << (128 - buffer->length - number_of_bits);
	else
	{
		buffer->left += data >> (number_of_bits - 64 + buffer->length);
		buffer->right = data << (128 - buffer->length - number_of_bits);
	}
	buffer->length += number_of_bits;
	return number_of_bits;
}