#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <argp.h>

/* Structure with input file name, output file name, decode flag and verbose flag. */
struct arguments
{
	char *infile;  /* Input file name */
	char *outfile; /* Output file name, null if stdout */
	int decode;	   /* Should decode */
	int verbose;   /* Verbose */
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

	struct arguments arguments = {NULL, NULL, 0, 0};
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	/*Opening files*/

	FILE *infile = fopen(arguments.infile, "r");
	if (!infile)
	{
		perror(arguments.infile);
		return 1;
	}
	FILE *outfile;
	if (arguments.outfile)
	{
		outfile = fopen(arguments.outfile, "w");
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

	/*Closing files*/

	fclose(infile);
	if (arguments.outfile)
	{
		fclose(outfile);
	}
	return 0;
}
