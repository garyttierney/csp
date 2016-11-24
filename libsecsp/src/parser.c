#include <libsecsp/parser.h>
#include "libsecsp-private.h"
#include "parser.tab.h"
#include "lexer.yy.h"

struct secsp_parse_error {
	struct secsp_parse_error *next;
};

struct secsp_parse_result {
	struct secsp_parse_error *errors;
	struct secsp_node_list *root;
};

LIBSECSP_EXPORT struct secsp_node_list *
secsp_get_root_node(struct secsp_parse_result *result)
{
	return result->root;
}

LIBSECSP_EXPORT int secsp_parse_file(FILE *input,
				     struct secsp_parse_result **result)
{
	struct secsp_parse_result *tmp = calloc(1, sizeof *tmp);
	if (tmp == NULL) {
		return -ENOMEM;
	}

	yyscan_t sc;
	int res;

	yylex_init(&sc);
	yyset_in(input, sc);
	res = yyparse(sc, tmp);
	yylex_destroy(sc);
	*result = tmp;

	return res;
}

int secsp_parser_finish(struct secsp_parse_result *result,
			struct secsp_node_list *body)
{
	result->root = body;
	return 0;
}
