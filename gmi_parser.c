#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include <sys/queue.h>

#include "gmi_parser.h"

/*
 * Simple text/gemini parser
 * https://gitlab.com/gemini-specification/gemini-text/-/blob/master/specification.gmi
 * Based on V0.15.0, March 5th, 2021
 *
 * Four core line types (in any order)
 * Text lines
 * Link lines =>
 * Preformatting toggle lines ```
 * Preformatted text lines 
 *
 * Three advanced line types
 * Heading lines #, ##, ###
 * Unordered list items *
 * Quote lines >
 *
 * TODO
 * Convert to library
 * Write proper tests
 * Write example 
 */
struct line 		*line_new();
struct line_list 	*push_line(struct line_list *, struct line *);
void			 line_free(struct line *);
struct line		*parse_line(enum linetype, int, char *);

/*
 * Create a new line struct to represent
 * each of the line types. Typically stored on
 * a gmi struct
 */
struct line *
line_new()
{
	struct line *l;
	if ((l = calloc(1, sizeof(struct line))) == NULL)
		err(1, NULL);

	return l;
}

struct line_list *
push_line(struct line_list *ll, struct line *l)
{
	if (ll == NULL) {
		if ((ll = calloc(1, sizeof(*ll))) == NULL) {
			err(1, NULL);
		}
		SLIST_INIT(ll);
	}

	if (l == NULL)
		err(1, "l is NULL");

	/* Check to see if the line list has a head */
	if(ll->slh_first == NULL) {
		SLIST_INSERT_HEAD(ll, l, next);
	} else {
		/* Find the last line and insert after */
		struct line *l1 = NULL;
		struct line *tail = NULL;
		SLIST_FOREACH(l1, ll, next)	/* Forward traversal. */
			tail = l1;
		
		SLIST_INSERT_AFTER(tail, l, next);
	}
	return ll;
}

void
line_free(struct line *l)
{
	l->line = NULL;
	l->number = 0;
	free(l);
}

struct line *
parse_line(enum linetype type, int number, char *line)
{
	switch (type) {
		case TEXT:
			break;
		case PRE_TOGGLE_BEGIN:
			break;
		case PRE_TOGGLE_END:
			/* strip any text following ``` */
			break;
		case PRE_TEXT:
			break;
		case LINK:
			/* TODO check for image extension
			* and use LINK_IMG */
			break;
		case HEADING_1:
			/* TODO determine heading level */
			break;
		case LIST:
			break;
		case QUOTE:
			break;
		default: {
			errx(1, "line type: %d is unsupported.\nline:%s\n", type, line);
		}
	}
	struct line *l = line_new();
	l->type = type;
	l->number = number;
	l->line = line;

	return l;
}


/*
 * Create a new gmi struct to represent the entirity
 * of a single .gmi document
 */
struct gmi *
gmi_new()
{
	struct gmi *g;
	if ((g = calloc(1, sizeof(struct gmi))) == NULL)
		err(1, NULL);

	return g;
}

/* Parse line to gmi struct */
void
gmi_parse_line(struct gmi *g, int line_number, char *line)
{
	if(g == NULL)
		err(1, NULL);

	if (line == NULL)
		err(1, NULL);

	size_t len = strlen(line);
	if (len <= 0)
		return;

	struct line *l = NULL;
	enum linetype type = TEXT;
	if (g->preformat_mode == true) {
		if (len >= 3) {
			/* If preformat mode is on toggle it off when ``` */
			if(line[0] == '`' && line[1] == '`' && line[2] == '`') {
				g->preformat_mode = false;
				type = PRE_TOGGLE_END;
				goto done;
			}
		}

		type = PRE_TEXT;
		goto done;
	}

	switch (line[0]) {
		case '=':
			type = LINK;
			break;
		case '`':
			if (len >= 3 && line[1] == '`' && line[2] == '`') {
				g->preformat_mode = true;
				type = PRE_TOGGLE_BEGIN;
			}
			break;
		case '#':
			/* Determing header level */
			type = HEADING_1;
			if (len >= 2 && line[1] == '#') {
				type = HEADING_2;
			}
			if (len >= 3 && line[1] == '#' && line[2] == '#') {
				type = HEADING_3;
			}
			break;
		case '*':
			type = LIST;
			break;
		case '>':
			type = QUOTE;
			break;
	}

done:
	l = parse_line(type, line_number, line);
	g->lines = push_line(g->lines, l);
}

/* Free any allocated memory associated with a gmi struct */
void
gmi_free(struct gmi *g)
{
	struct line *l = NULL;
	while (!SLIST_EMPTY(g->lines)) {
		l = SLIST_FIRST(g->lines);
		SLIST_REMOVE_HEAD(g->lines, next);
		line_free(l);
	}
	/*
	SLIST_FOREACH(l, g->lines, next) {
		SLIST_REMOVE_HEAD(g->lines, next);
		line_free(l);
		}
	*/

	free(g->lines);
	g->lines = NULL;
	free(g);
}
