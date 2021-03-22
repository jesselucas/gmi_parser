#import "stdio.h"

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
 */


/* Representations of gemini text document */
typedef struct {
} gmi;

int
main(void) 
{
	printf("nothing to see... %s\n", "yet");
	return 0;
}
