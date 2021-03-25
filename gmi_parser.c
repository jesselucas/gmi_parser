#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/queue.h>

/*
 * Simple text/gemini parser
 * gemini://gemini.circumlunar.space:1965/docs/specification.gmi
 * Based on v0.14.3, November 29th 2020
 *
 * Four core line types (in any order)
 * Text lines
 * Link lines
 * Preformatting toggle lines
 * Preformatted text lines
 *
 * Three advanced line types
 * Heading lines
 * Unordered list items
 * Quote lines 
 */

/* notes to self
 * use queue(3) macros
 * use strcep
 * use scanf
 */

enum linetype { TEXT, LINK, PRE_TOGGLE, PRE_TEXT, HEADING, LIST, QUOTE };
struct line {
	SLIST_ENTRY(line) 	 next;
	int 			 number;
	enum 			 linetype type;
	char 			*line;
};
SLIST_HEAD(line_list, line);

struct line 		*line_new();
struct line_list 	*push_line(struct line_list *ll, char *list);
struct gmi		*gmi_new();
int		 	 gmi_parseline(struct gmi *,  char *);

/*
 * create a new line struct to represent
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
push_line(struct line_list *ll, char *line)
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
	l->line = line;
	SLIST_INSERT_HEAD(ll, l, next);

	return ll;
}

struct {
	struct line_list 	*lines;
} gmi;

/*
 * create a new gmi struct to represent the entirity 
 * of a single .gmi document
 */
struct gmi *
gmi_new()
{
	struct gmi *g;
	if ((g = calloc(1, sizeof(gmi))) == NULL)
		err(1, NULL);

	return g;
}

/* Parse line to gmi struct */
int
gmi_parseline(struct gmi *g, char *line) 
{
	return 0;		
}


/*
 * main currently used for testing
 */
int
main(void) 
{
	printf("nothing to see... %s\n", "yet");
	struct gmi *g = gmi_new();

	struct line_list *ll = NULL;
       	ll = push_line(ll, "some line\n");
       	ll = push_line(ll, "some other line\n");
       	ll = push_line(ll, "a third line\n");
	
	struct line *l = NULL;
	SLIST_FOREACH(l, ll, next)       /* Forward traversal. */
		printf("%s", l->line);

	return 0;
}
