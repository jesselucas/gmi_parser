#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include <sys/queue.h>

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
 */

/* notes to self
 * use queue(3) macros
 * use strcep
 * use scanf
 */

enum linetype {
	TEXT,
	LINK,
	PRE_TOGGLE,
	PRE_TEXT,
	HEADING_1,
	HEADING_2,
	HEADING_3,
	LIST,
	QUOTE
};

struct line 		*line_new();
struct line_list 	*push_line(struct line_list *, enum linetype, int, char *);
struct gmi		*gmi_new();
void		 	 gmi_parse_line(struct gmi *, int, char *);
struct line		*line_parse(char *);

struct line {
	SLIST_ENTRY(line) 	 next;
	int 			 number;
	enum linetype 		 type;
	char 			*line;
	_Bool			 preformatted;
};
SLIST_HEAD(line_list, line);

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
push_line(struct line_list *ll, enum linetype type, int number, char *line)
{
	struct line *l;
	if (ll == NULL) {
		if ((ll = calloc(1, sizeof(*ll))) == NULL) {
			err(1, NULL);
		}
		SLIST_INIT(ll);
	}
	if ((l = calloc(1, sizeof(*l))) == NULL) {
		free(ll);
		err(1, NULL);
	}
	l->type = type;
	l->number = number;
	l->line = line;
	if(ll->slh_first == NULL) {
		SLIST_INSERT_HEAD(ll, l, next);
	} else {
		/* Find the last line and insert after */
		struct line *l1 = NULL;
		struct line *tail = NULL;
		SLIST_FOREACH(l1, ll, next)       /* Forward traversal. */
			tail = l1;	
		
		SLIST_INSERT_AFTER(tail, l, next);
	}

	return ll;
}

struct gmi {
	struct line_list 	*lines;
	_Bool 			 preformat_mode;
};

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
	g->lines = NULL;

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

	enum linetype line_type = TEXT;

	size_t len = strlen(line);	
	if (len <= 0) 
		return;

	if (g->preformat_mode == true) {
		if (len >= 3) {
			/* If preformat mode is on toggle it off when ``` */
			if(line[0] == '`' && line[1] == '`' && line[2] == '`') {
				/* TODO strip any text following ``` */
				g->preformat_mode = false;
				line_type = PRE_TOGGLE;
				goto done;
			}
		}

		line_type = PRE_TEXT;
		goto done;
	}

	switch (line[0]) {
		case '=':
			line_type = LINK;
			break;
		case '`':
			if (len >= 3 && line[1] == '`' && line[2] == '`') {
				g->preformat_mode = true;
				/* TODO Add alt text to preformat data */
				return;
			}
			break;
		case '#':
			line_type = HEADING_1;
			break;
		case '*':
			line_type = LIST;
			break;
		case '>':
			line_type = QUOTE;
			break;
	}

done:
	g->lines = push_line(g->lines, line_type, line_number, line);
}

/*
 * main currently used for testing
 */
int
main(void) 
{
	printf("nothing to see... %s\n", "yet");
	char *gem_test = strdup(
		"All the following examples are valid link lines:\n"
	       	"=> gemini://example.org/\n"
		"=> gemini://example.org/ An example link\n"
		"=> gemini://example.org/foo     Another example link at the same host"
		"=> foo/bar/baz.txt      A relative link\n"
		"=>      gopher://example.org:70/1 A gopher link\n"
		"```alt text\n"
		"some preformatted things\n"
		"=> preformatted link is not a link?\n"
		"# Or a header\n"
		"> Or paragraph\n"
		"```\n"
		"=> Links should be back\n"
		"# So should headers\n"
		"What about text?\n"
		"> Paragraphs working?\n"
		"> A verbose one huh?\n"
		"```\n"
		"Preformatting again\n"
		"# Isn't that enough\n"
		"``` yeah\n"
	);
	
	struct gmi *g = gmi_new();
	char *text_line;	
	int line_number = 0;
	while((text_line = strsep(&gem_test, "\n")) != NULL ) {
		gmi_parse_line(g, line_number, text_line);
		line_number++;
	}


	// gmi_parse_line(g, "some other line\n");
	// gmi_parse_line(g, "third line\n");
	
	struct line *l = NULL;
	SLIST_FOREACH(l, g->lines, next)       /* Forward traversal. */
		printf("%d: type:%u text:%s\n", l->number, l->type, l->line);

	return 0;
}
