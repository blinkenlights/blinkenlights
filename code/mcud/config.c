/*
 * config.c
 */

#include <stdio.h>
#include <string.h>
#include <expat.h>

#include <blib/bprotocol.h>

#include "mcud.h"
#include "mcubuf.h"

#define TRUE 1
#define FALSE 0

#define PARSER_BUFFER_SIZE	   8192

#define SETUP_ELEMENT          "mcu-setup"
#define SETUP_HEIGHT_ATTRIBUTE "height"
#define SETUP_WIDTH_ATTRIBUTE  "width"
#define SETUP_CHANNELS_ATTRIBUTE  "channels"

#define LAMP_ELEMENT           "lamp"
#define LAMP_ID_ATTRIBUTE      "id"
#define LAMP_ROW_ATTRIBUTE     "row"
#define LAMP_COLUMN_ATTRIBUTE  "column"

enum level {
	LEVEL_ROOT,
	LEVEL_SETUP,
	LEVEL_LAMP
};

typedef struct parser_state {
	int position;
	mcu_setup_t * p_mcu_setup;
} parser_state_t;

static char parse_buffer[PARSER_BUFFER_SIZE];

static XML_Parser parser;

static XML_Char *
config_find_attribute ( XML_Char * name, const XML_Char **attributes, int mandatory)
{
	int index;

	for (index = 0;attributes[index] != NULL;index += 2) {
		if ( strcmp ( name, attributes[index]) == 0 )
			return (XML_Char *) attributes[index + 1];
	}
	if (mandatory) {
		fprintf (stderr, "Missing attribute %s at line %d\n", name, XML_GetCurrentLineNumber(parser));
	}
	return NULL;
}

static void
config_start_element (void *data, const XML_Char *element, const XML_Char **attributes)
{
	XML_Char * p_attribute;
	
	parser_state_t * p_state = (parser_state_t *) XML_GetUserData(parser);

	if (p_state->position == LEVEL_ROOT && strcmp(element, SETUP_ELEMENT) == 0) {

		p_state->position = LEVEL_SETUP;

		p_attribute = config_find_attribute ( SETUP_HEIGHT_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		p_state->p_mcu_setup->header.height = atoi(p_attribute);

		p_attribute = config_find_attribute ( SETUP_WIDTH_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		p_state->p_mcu_setup->header.width = atoi(p_attribute);

		p_attribute = config_find_attribute ( SETUP_CHANNELS_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		p_state->p_mcu_setup->header.channels = atoi(p_attribute);

		return;
	}

	if (p_state->position == LEVEL_SETUP && strcmp(element, LAMP_ELEMENT) == 0) {
		int id, row, column;

		p_state->position = LEVEL_LAMP;
		
		p_attribute = config_find_attribute ( LAMP_ID_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		id = atoi(p_attribute);

		p_attribute = config_find_attribute ( LAMP_ROW_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		row = atoi(p_attribute);

		p_attribute = config_find_attribute ( LAMP_COLUMN_ATTRIBUTE, attributes, TRUE);
		if (p_attribute == NULL)
			return;
		column = atoi(p_attribute);

		p_state->p_mcu_setup->body.pixel[id].row = row;
		p_state->p_mcu_setup->body.pixel[id].column = column;

		return;
	}

}

static void
config_end_element (void *data, const XML_Char *element)
{
	parser_state_t * p_state = (parser_state_t *) XML_GetUserData(parser);

	if (p_state->position == LEVEL_LAMP) {
		p_state->position = LEVEL_SETUP;
		return;
	}
	if (p_state->position == LEVEL_SETUP) {
		p_state->position = LEVEL_ROOT;
		return;
	}
}

int
process_config_file (char *path, mcu_setup_t * p_mcu_setup)
{
	int done = 0;
	int len;
	int status;
	FILE *config_file;
	static parser_state_t state;

	config_file = fopen (path, "r");
	if (config_file == NULL) {
		fprintf (stderr, "Can't open %s for reading.\n", path);
		return -1;
	}

	parser = XML_ParserCreate(NULL);
	if (parser == NULL) {
		fprintf (stderr, "Couldn't allocate memory for parser\n");
		exit (1);
	}

	state.p_mcu_setup = p_mcu_setup;
	state.position = LEVEL_ROOT;
	XML_SetUserData(parser, &state);
	XML_SetElementHandler(parser, config_start_element, config_end_element);

	while (!done) {

		len = fread(parse_buffer, 1, sizeof(parse_buffer), config_file);

		if (ferror(config_file)) {
			fprintf(stderr, "Read error\n");
			exit (1);
		}
		done = feof(config_file);

		status =  XML_Parse(parser, parse_buffer, len, done);

		if (status == 0) {
			fprintf(stderr, "Parse error at line %d:\n%s\n",
				XML_GetCurrentLineNumber(parser), XML_ErrorString(XML_GetErrorCode(parser)));
			exit (1);
		}
	}
	XML_ParserFree (parser);

	fclose (config_file);

	return 0;
}

