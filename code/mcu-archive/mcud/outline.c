/*
 * config.c
 */

#include <stdio.h>
#include <expat.h>

#define PARSER_BUFFER_SIZE	8192

static char parse_buffer[PARSER_BUFFER_SIZE];

typedef struct parser_state {
	int depth;
} parser_state_t;

XML_Parser parser;
parser_state_t state;

XML_Char *
find_attribute ( XML_Char * name, const XML_Char **attribute)
{
	int index;

	for (index = 0;attribute[index] != NULL;index += 2) {
		if ( strcmp ( name, attribute[index]) == 0 )
			return (XML_Char *) attribute[index + 1];
	}
	return NULL;
}

void
start_element (void *data, const XML_Char *element, const XML_Char **attribute)
{
	int i;
	parser_state_t * p_state = (parser_state_t *) XML_GetUserData(parser);

	for (i = 0; i < p_state->depth; i++)
		printf("  ");
	printf("%s", element);

	for (i = 0; attribute[i]; i += 2) {
		printf(" %s='%s'", attribute[i], attribute[i + 1]);
	}

	printf("\n");
	p_state->depth++;
}

void
end_element (void *data, const XML_Char *element)
{
	int i;
	parser_state_t * p_state = (parser_state_t *) XML_GetUserData(parser);
	
	p_state->depth--;

	for (i = 0; i < p_state->depth; i++)
		printf("  ");
	printf("%s\n", element);
}

int
main (int argc, char **argv)
{
	int done = 0;
	int len;
	int status;

	parser = XML_ParserCreate(NULL);
	if (parser == NULL) {
		fprintf (stderr, "Couldn't allocate memory for parser\n");
		exit (1);
	}

	state.depth = 0;
	XML_SetUserData(parser, &state);
	XML_SetElementHandler(parser, start_element, end_element);

	while (!done) {

		len = fread(parse_buffer, 1, sizeof(parse_buffer), stdin);

		if (ferror(stdin)) {
			fprintf(stderr, "Read error\n");
			exit (1);
		}
		done = feof(stdin);

		status =  XML_Parse(parser, parse_buffer, len, done);

		if (status == 0) {
			fprintf(stderr, "Parse error at line %d:\n%s\n", XML_GetCurrentLineNumber(parser), XML_ErrorString(XML_GetErrorCode(parser)));
			exit (1);
		}
	}
	XML_ParserFree (parser);

	return 0;
}

