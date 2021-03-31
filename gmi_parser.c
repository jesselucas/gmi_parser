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
parse_line(enum linetype type, int number, char *line) {
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
	if (g->preformat_mode == true) {
		if (len >= 3) {
			/* If preformat mode is on toggle it off when ``` */
			if(line[0] == '`' && line[1] == '`' && line[2] == '`') {
				/* TODO strip any text following ``` */
				g->preformat_mode = false;
				l = parse_line(PRE_TOGGLE, line_number, line);
				goto done;
			}
		}

		l = parse_line(PRE_TEXT, line_number, line);
		goto done;
	}

	switch (line[0]) {
		case '=':
			/* TODO check for image extension
			 * and use LINK_IMG */
			l = parse_line(LINK, line_number, line);
			break;
		case '`':
			if (len >= 3 && line[1] == '`' && line[2] == '`') {
				g->preformat_mode = true;
				return;
			}
			break;
		case '#':
			l = parse_line(HEADING_1, line_number, line);
			break;
		case '*':
			l = parse_line(LIST, line_number, line);
			break;
		case '>':
			l = parse_line(QUOTE, line_number, line);
			break;
		default:
			l = parse_line(TEXT, line_number, line);
			break;
	}

done:
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
	SLIST_FOREACH(l, g->lines, next)
		SLIST_REMOVE_HEAD(g->lines, next);
		line_free(l);
		*/

	free(g->lines);
	g->lines = NULL;
	free(g);
}
