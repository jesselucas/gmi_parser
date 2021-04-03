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
struct line 		*line_new(void);
struct line_list 	*push_line(struct line_list *, struct line *);
void			 line_free(struct line *);
struct line		*parse_line(enum linetype, int, char *);

static char *image_ext[4] = {"gif", "jpeg", "jpg", "png"};

/*
 * Create a new line struct to represent
 * each of the line types. Typically stored on
 * a gmi struct
 */
struct line *
line_new(void)
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
	if(l == NULL)
		return;
	l->line = NULL;
	l->number = 0;
	free(l);
	l = NULL;
}

struct line *
parse_line(enum linetype type, int number, char *line)
{
	if (line == NULL)
		err(1, "line is NULL");

	switch (type) {
		case TEXT:
			break;
		case PRE_TOGGLE_END:
			/* strip any text following ``` */
		case PRE_TOGGLE_BEGIN:
			break;
		case PRE_TEXT:
			break;
		case LINK:
			break;
		case LINK_IMG:
			break;
		case HEADING_1:
			break;
		case HEADING_2:
			break;
		case HEADING_3:
			break;
		case LIST:
			break;
		case QUOTE:
			break;
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
			if(len < 2 || line[1] != '>')
				goto done;

			type = LINK;

			/* Check for image extension
			 * Only supports small set of extensions
			 * https://www.iana.org/assignments/media-types/media-types.xhtml#image
			 * TODO: reverse search to only look for extenstions
			 */
			char *ext = strchr(line, '.');
			if (ext == NULL)
				goto done;

			int extlen = sizeof(image_ext)/sizeof(*image_ext);
			for(int i = 0; i < extlen; ++i) {
				if(strcmp(ext + 1, image_ext[i]) == 0)
					type = LINK_IMG;
			}

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
	if (g == NULL)
		return;

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
