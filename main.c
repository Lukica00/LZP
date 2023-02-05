#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>
#include "queue.h"
/* Structure with input file name, output file name, decode flag and verbose flag. */
struct arguments
{
	char *infile;		  /* Input file name */
	char *outfile;		  /* Output file name, null if stdout */
	int decode;			  /* Should decode */
	__uint64_t search;	  /* Search buffer length */
	__uint64_t lookAhead; /* Look-ahead buffer length */
	int verbose;		  /* Verbose */
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
	case 's':
		__uint64_t searchValue;
		int searchStatus = sscanf(arg, "%llu", &searchValue);
		if (!searchStatus)
		{
			printf("Using the default value for search buffer length. ");
			if (arguments->verbose)
				printf("Provided value could not be parsed as unsigned integer.");
			printf("\n");
		}
		arguments->search = searchValue;
		break;
	case 'l':
		__uint64_t lookAheadValue;
		int lookAheadStatus = sscanf(arg, "%llu", &lookAheadValue);
		if (!lookAheadStatus)
		{
			printf("Using the default value for look-ahead buffer length. ");
			if (arguments->verbose)
				printf("Provided value could not be parsed as unsigned integer.");
			printf("\n");
		}
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

	struct arguments arguments = {NULL, NULL, 0, 1024, 16, 0};
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

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
	int numBytesToWrite = 1;
	if (arguments.search > __UINT32_MAX__)
	{
		numBytesToWrite = 8;
	}
	else if (arguments.search > __UINT16_MAX__)
	{
		numBytesToWrite = 4;
	}
	else if (arguments.search > __UINT8_MAX__)
	{
		numBytesToWrite = 2;
	}
	typedef struct hit
	{
		__uint64_t offset;
		__uint64_t length;
		__uint8_t value;
	};
	if (!arguments.decode)
	{
		/*Endode*/
		struct queue *search = initalizeQueue(arguments.search);
		struct queue *lookAhead = initalizeQueue(arguments.lookAhead);

		do
		{
			if (isLeftAligned(lookAhead) && !isEmpty(lookAhead))
			{
				enqueue(search, dequeue(lookAhead));
			}
			__uint8_t element;
			if (fread(&element, sizeof(element), 1, infile))
			{
				enqueue(lookAhead, element);
			}
			else
			{
				shiftLeft(lookAhead);
			}
		} while (!isEmpty(lookAhead));

		freeQueue(search);
		freeQueue(lookAhead);
	}
	else
	{
		/*Decode*/
	}
	/*Closing files*/

	fclose(infile);
	if (arguments.outfile)
	{
		fclose(outfile);
	}
	return 0;
}
