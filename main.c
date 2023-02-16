#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include <math.h>
#include "queue.h"
#include "hashtable.h"
#include "bitbuffer.h"
// TODO pocisti inkludovi, testriaj stdin kompresiju
/* Structure with input file name, output file name, decode flag and verbose flag. */
struct arguments
{
	char *infile;				/* Input file name */
	char *outfile;				/* Output file name, null if stdout */
	int decode;					/* Should decode */
	__uint64_t search;			/* Search buffer length */
	__uint64_t lookAhead;		/* Look-ahead buffer length */
	int verbose;				/* Verbose */
	__uint64_t predictionBytes; /*Number of bytes used for prediction*/
};
const char *argp_program_version =
	"lzp 0.1";

const char *argp_program_bug_address =
	"<lukica@elfak.rs> or <cirkovic.stefan@elfak.rs>";

static struct argp_option options[] =
	{
		{"verbose", 'v', 0, 0, "Produce verbose output"},
		{"decode", 'd', 0, 0, "Decode instead of encoding file"},
		{"output", 'o', "OUTPUT_FILE", 0, "Output to OUTPUT_FILE instead of to STDOUT"},
		{"search", 's', "SEARCH_LENGTH", 0, "Length of search buffer, default 1024"},
		{"lookahead", 'l', "LOOKAHEAD_LENGTH", 0, "Length of look-ahead buffer, default 16"},
		{"prediction", 'p', "PREDICTION_BYTES", 0, "Number of bytes used for prediction, default 2"},
		{0}};

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key)
	{
	case 'v':
		arguments->verbose = 1;
		break;
	case 'd':
		arguments->decode = 1;
		break;
	case 'o':
		arguments->outfile = arg;
		break;
	case 'p':
		__uint64_t predictionValue;
		int predictionStatus;
		if (arg[0] == '-')
			predictionStatus = 0;
		else
			predictionStatus = sscanf(arg, "%llu", &predictionValue);
		if (!predictionStatus)
		{
			printf("Using the default value for number of prediction bytes. ");
			if (arguments->verbose)
				printf("%s could not be parsed as unsigned integer. ", arg);
			printf("\n");
		}
		else
			arguments->predictionBytes = predictionValue;
		break;
	case 's':
		__uint64_t searchValue;
		int searchStatus;
		if (arg[0] == '-')
			searchStatus = 0;
		else
			searchStatus = sscanf(arg, "%llu", &searchValue);
		if (!searchStatus)
		{
			printf("Using the default value for search buffer length. ");
			if (arguments->verbose)
				printf("%s could not be parsed as unsigned integer. ", arg);
			printf("\n");
		}
		else
			arguments->search = searchValue;
		break;
	case 'l':
		__uint64_t lookAheadValue;
		int lookAheadStatus;
		if (arg[0] == '-')
			lookAheadStatus = 0;
		else
			lookAheadStatus = sscanf(arg, "%llu", &lookAheadValue);
		if (!lookAheadStatus)
		{
			printf("Using the default value for look-ahead buffer length. ");
			if (arguments->verbose)
				printf("%s could not be parsed as unsigned integer. ", arg);
			printf("\n");
		}
		else
			arguments->lookAhead = lookAheadValue;
		break;
	case ARGP_KEY_ARG:
		if (state->arg_num >= 1)
		{
			argp_usage(state);
		}
		arguments->infile = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 1)
		{
			argp_usage(state);
		}
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Arguments description*/
static char args_doc[] = "INPUT_FILE";

/* Program description*/
static char doc[] = "LZP -- Lempel-Ziv + Prediction";

static struct argp argp = {options, parse_opt, args_doc, doc};

/*
   The main function.
*/

int main(int argc, char **argv)
{
	/* Parsing arguments */

	struct arguments arguments = {NULL, NULL, 0, 1024, 16, 0, 2};
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	if (arguments.lookAhead > arguments.search)
	{
		printf("Using the search buffer length for look-ahead buffer length. ");
		if (arguments.verbose)
		{
			printf("Length of look-ahead buffer (%llu) cannot be longer than of search buffer(%llu). ", arguments.lookAhead, arguments.search);
		}
		arguments.lookAhead = arguments.search;
	}
	if (arguments.predictionBytes > arguments.search)
	{
		printf("Using the search buffer length for number of prediction bytes. ");
		if (arguments.verbose)
		{
			printf("Number of prediction bytes (%llu) cannot be larger than length of search buffer(%llu). ", arguments.predictionBytes, arguments.search);
		}
		arguments.predictionBytes = arguments.search;
	}
	/*Opening files*/

	FILE *infile = fopen(arguments.infile, "rb");
	if (!infile)
	{
		perror(arguments.infile);
		return 1;
	}
	FILE *outfile;
	if (arguments.outfile)
	{
		outfile = fopen(arguments.outfile, "wb");
		if (!outfile)
		{
			perror(arguments.outfile);
			return 1;
		}
	}
	else
	{
		outfile = stdout;
	}

	/*Logic*/
	int numBitsForLength = ceil(log2(arguments.lookAhead));
	if (!arguments.decode)
	{
		/*Endode*/
		struct queue *search = initalizeQueue(arguments.search);
		struct queue *lookAhead = initalizeQueue(arguments.lookAhead);
		struct hashtable *hashtable = initializeHashtable(arguments.search);
		struct bitbuffer *bitbuffer = initializeBitbuffer();
		__uint64_t length = 0;
		do
		{
			__uint8_t elements[lookAhead->length];
			int bytesRead = fread(&elements, sizeof(*elements), length + 1, infile);
			if (bytesRead)
			{
				for (int i = 0; i < bytesRead; i++)
					enqueue(lookAhead, elements[i]);
			}
			else
			{
				shiftLeft(lookAhead);
			}
			length = 0;
			if (isLeftAligned(lookAhead))
			{
				if (search->elements >= arguments.predictionBytes)
				{
					__uint64_t hash = hashFunction(search, arguments.predictionBytes);
					struct hashtableentry entry = getElement(hashtable, hash);
					if (entry.is)
					{
						__uint64_t searchTemp, searchTempOrigin = (entry.pointer + 1) % search->length;
						__uint64_t lookAheadTemp = lookAhead->left;
						searchTemp = searchTempOrigin;
						while (lookAhead->buffer[lookAheadTemp] == search->buffer[searchTemp] && length < (lookAhead->elements - 1))
						{
							length++;
							lookAheadTemp = (lookAheadTemp + 1) % lookAhead->length;
							if (searchTemp == search->right)
								searchTemp = searchTempOrigin;
							else
							{
								searchTemp = (searchTemp + 1) % search->length;
							}
						}
					}
					for (int i = 0; i < length; i++)
					{
						enqueue(search, dequeue(lookAhead));
					}
					entry.is = 1;
					entry.pointer = search->right;
					setElement(hashtable, hash, entry);
					if (length)
					{
						writeBits(bitbuffer, 1, 1);
						writeBits(bitbuffer, length, numBitsForLength);
					}
					else
					{
						writeBits(bitbuffer, 0, 1);
					}
				}
				__uint8_t value = dequeue(lookAhead);
				enqueue(search, value);
				writeBits(bitbuffer, value, 8);
				while (bitbuffer->length >= 8)
				{
					__uint8_t data = readBits(bitbuffer, 8);
					fwrite(&data, sizeof(data), 1, outfile);
				}
			}
		} while (!isEmpty(lookAhead) || length);
		while (bitbuffer->length != 0)
		{
			__uint8_t data = readBits(bitbuffer, 8);
			fwrite(&data, sizeof(data), 1, outfile);
		}
		freeHashtable(hashtable);
		freeQueue(search);
		freeQueue(lookAhead);
	}
	else
	{
		/*Decode*/
		struct queue *search = initalizeQueue(arguments.search);
		struct hashtable *hashtable = initializeHashtable(arguments.search);
		__uint8_t element;
		while (search->elements < arguments.predictionBytes && fread(&element, sizeof(element), 1, infile))
		{
			enqueue(search, element);
			fwrite(&element, sizeof(element), 1, outfile);
		}
		struct bitbuffer *bitbuffer = initializeBitbuffer();
		__uint8_t inData = 0;
		__uint64_t length = 0;
		__uint8_t phase = 0; // 0 ceka se, 1 procitan flag, 2 procitana duzina, 3 procitan podatak
		while (1)
		{
			int bytesRead = 0;
			if (bitbuffer->length <= 120)
				bytesRead = fread(&inData, sizeof(inData), 1, infile);
			if (!bytesRead && bitbuffer->length < 8 && phase == 0)
				break;
			if (bytesRead)
				writeBits(bitbuffer, inData, 8);
			switch (phase)
			{
			case 0:
				if (bitbuffer->length >= 1)
				{
					__uint8_t flag = readBits(bitbuffer, 1);
					length = 0;
					if (flag)
						phase = 1;
					else
						phase = 2;
				}
				break;
			case 1:
				if (bitbuffer->length >= numBitsForLength)
				{
					length = readBits(bitbuffer, numBitsForLength);
					phase = 2;
				}
				break;
			case 2:
				if (bitbuffer->length >= 8)
				{
					element = 0;
					element = readBits(bitbuffer, 8);
					phase = 3;
				}
				break;
			case 3:
				__uint64_t hash = hashFunction(search, arguments.predictionBytes);
				struct hashtableentry entry = getElement(hashtable, hash);
				__uint64_t searchTempOrigin = (entry.pointer + 1) % search->length, searchTemp = searchTempOrigin, searchRightTemp = search->right;

				struct queue *queueTemp = initalizeQueue(arguments.search);
				for (__uint64_t i = 0; i < length; i++)
				{
					__uint8_t repeatedElement = search->buffer[searchTemp];
					enqueue(queueTemp, repeatedElement);
					fwrite(&repeatedElement, sizeof(repeatedElement), 1, outfile);
					searchTemp = (searchTemp + 1) % search->length;
					if (searchTemp == ((searchRightTemp + 1) % search->length))
					{
						searchTemp = searchTempOrigin;
					}
				}
				for (__uint64_t i = 0; i < length; i++)
				{
					enqueue(search, dequeue(queueTemp));
				}

				entry.pointer = search->right;
				setElement(hashtable, hash, entry);
				enqueue(search, element);
				fwrite(&element, sizeof(element), 1, outfile);
				phase = 0;
				freeQueue(queueTemp);
				break;
			}
		}
		freeHashtable(hashtable);
		freeQueue(search);
	}
	/*Closing files*/

	fclose(infile);
	if (arguments.outfile)
	{
		fclose(outfile);
	}
	return 0;
}
