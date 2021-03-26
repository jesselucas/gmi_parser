#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/queue.h>

/*
 * Simple text/gemini parser
 * https://gitlab.com/gemini-specification/gemini-text/-/blob/master/specification.gmi
 * Based on V0.15.0, March 5th, 2021
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
	enum linetype 		 type;
	char 			*line;
	void			*data;
};
SLIST_HEAD(line_list, line);

struct line 		*line_new();
struct line_list 	*push_line(struct line_list *ll, char *list);
struct gmi		*gmi_new();
void		 	 gmi_parse_line(struct gmi *,  char *);

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

struct gmi {
	struct line_list 	*lines;
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
gmi_parse_line(struct gmi *g, char *line) 
{
	/* TODO create an return gmi struct ? */
	if(g == NULL) 
		err(1, NULL);

	/* Parse line to determine type */

	/* Push to line list */
       	g->lines = push_line(g->lines, line); 
}

/*
 * main currently used for testing
 */
int
main(void) 
{
	char *gem_test = "All the following examples are valid link lines:\n" 
	       	"=> gemini://example.org/\n"
		"=> gemini://example.org/ An example link\n"
		"=> gemini://example.org/foo     Another example link at the same host"
		"=> foo/bar/baz.txt      A relative link\n"
		"=>      gopher://example.org:70/1 A gopher link\n";
		
		

	printf("nothing to see... %s\n", "yet");
	struct gmi *g = gmi_new();

       	gmi_parse_line(g, "some line\n");
       	gmi_parse_line(g, "some other line\n");
       	gmi_parse_line(g, "third line\n");
	
	struct line *l = NULL;
	SLIST_FOREACH(l, g->lines, next)       /* Forward traversal. */
		printf("%s", l->line);

	return 0;
}
