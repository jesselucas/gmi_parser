#ifndef GMI_PARSER_H
#define GMI_PARSER_H
enum linetype {
	TEXT,
	LINK,
	LINK_IMG,
	PRE_TOGGLE_BEGIN,
	PRE_TOGGLE_END,
	PRE_TEXT,
	HEADING_1,
	HEADING_2,
	HEADING_3,
	LIST,
	QUOTE
};

struct line {
	SLIST_ENTRY(line) 	 next;
	int 			 number;
	enum linetype 		 type;
	char 			*line;
};
SLIST_HEAD(line_list, line);

struct gmi {
	struct line_list 	*lines;
	_Bool 			 preformat_mode;
};

struct gmi		*gmi_new();
void		 	 gmi_parse_line(struct gmi *, int, char *);
void			 gmi_free(struct gmi *);
#endif /* GMI_PARSER_H */
