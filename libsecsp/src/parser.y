%locations
%lex-param {void *scanner}
%parse-param {void *scanner}{struct secsp_parse_result *result}

%define api.pure full
%define api.token.prefix {TOK_}
%define parse.trace
%define parse.error verbose
%define parse.lac full

%{
#include <stdio.h>
#include "parser.tab.h"
#include "lexer.yy.h"
#include <libsecsp/ast.h>

#define NODE (struct secsp_node *)
#define ABORT_IF_NULL(val) \
	if (val == NULL) { \
		YYABORT;   \
	}

void yyerror(YYLTYPE *loc, yyscan_t scanner, struct secsp_parse_result *result, char const *msg);
int secsp_parser_finish(struct secsp_parse_result *result, struct secsp_node_list *root);


static struct secsp_context_node *
create_context_node(struct secsp_node *user, struct secsp_node *role,
			struct secsp_node *type, struct secsp_node *levelrange);

static struct secsp_level_node *
create_level_node(struct secsp_node *sensitivity, struct secsp_node *categories);

static struct secsp_block_node *
create_block_node(const char *name, int is_abstract,
                  struct secsp_node_list *body,
                  struct secsp_node_list *inherits);

static struct secsp_decl_node *
create_decl_node(const char *name, enum secsp_node_flavor flavor,
                 struct secsp_node *rval);

static struct secsp_sym_node *create_sym_node(const char *val);
static struct secsp_range_node *create_range_node(enum secsp_node_flavor flavor,
						  struct secsp_node *low,
						  struct secsp_node *high);

static struct secsp_setexpr_node *
create_setexpr_node(struct secsp_node *lhs, enum secsp_operator op,
		    struct secsp_node *rhs);
%}

%code requires {
#include <libsecsp/ast.h>
#include <libsecsp/parser.h>
}

%union {
	char *name;
	long number;
	enum secsp_operator operator;
	struct secsp_node *node;
	struct secsp_node_list *node_list;
}

%token <number>	NUMBER "number"
%token <name>	NAME "identifier"

%token MINUS "-"
%token EOF 0 "end of file"
%token LPAREN "("
%token RPAREN ")"
%token LBRACE "{"
%token RBRACE "}"
%token COMMA ","
%token COLON ":"
%token DOT "."
%token <operator> AND "&"
%token <operator> NOT "~"
%token <operator> XOR "^"
%token <operator> OR "|"
%token EQUALS "="
%token SEMICOLON ";"

%token <name> ABSTRACT "abstract"
%token <name> BLOCK "block"
%token <name> INHERITS "inherits"

%type <node_list> statement_list;
%type <node_list> identifier_list;
%type <node> statement block_statement decl_statement;
%type <node> expression category_range_expr level_expr level_range_expr context_expr reference_expr;
%type <node> level_or_id level_range_or_id
%type <node> set_expression;
%type <operator> set_operator;
%type <name> identifier;
%start module

%left OR
%left AND
%left XOR
%left NOT

%%

module
		: statement_list[stmts] { secsp_parser_finish(result, $stmts); }
		;

statement_list
		: { $$ = NULL; }
		| statement_list statement {
			struct secsp_node_list *head = $1;
			if (secsp_node_list_insert(&head, $2) < 0) {
				YYABORT;
			}

			$$ = head;
		}
		;

statement
		: block_statement
		| decl_statement
		| error SEMICOLON { yyerrok; }
		;

block_statement
		: ABSTRACT BLOCK identifier[id] LBRACE statement_list[stmts] RBRACE {
			$$ = NODE create_block_node($id, 1, $stmts, NULL);
			ABORT_IF_NULL($$);
		  }
		| BLOCK identifier[id] LBRACE statement_list[stmts] RBRACE {
			$$ = NODE create_block_node($id, 0, $stmts, NULL);
			ABORT_IF_NULL($$)
		  }
		;
decl_statement
		: identifier[type] identifier[id] SEMICOLON {
			$$ = NODE create_decl_node($id, SECSP_TYPE_DECL, NULL);
			ABORT_IF_NULL($$);
		  }
		| identifier[type] identifier[id] EQUALS expression[expr] SEMICOLON {
			$$ = NODE create_decl_node($id, SECSP_TYPE_DECL, $expr);
			ABORT_IF_NULL($$);
		  };
expression
	  : context_expr
	  | level_range_expr
	  | level_expr
	  | category_range_expr
	  | reference_expr;

reference_expr
		: identifier[id] {
			$$ = NODE create_sym_node($id);
			ABORT_IF_NULL($$);
		  };

category_range_expr
		: reference_expr[low] DOT reference_expr[high] {
			$$ = NODE create_range_node(SECSP_CATEGORY_RANGE_EXPR, $low, $high);
			ABORT_IF_NULL($$);
		  };

level_expr
		: reference_expr[sensitivity] COLON category_range_expr[categories] {
			$$ = NODE create_level_node($sensitivity, $categories);
			ABORT_IF_NULL($$);
		  }
		;

level_or_id: level_expr | reference_expr;

level_range_expr
		: level_or_id[low] MINUS level_or_id[high] {
			$$ = NODE create_range_node(SECSP_LEVEL_RANGE_EXPR, $low, $high);
			ABORT_IF_NULL($$);
		  }
		;

level_range_or_id : level_range_expr | reference_expr;

context_expr
		: reference_expr[user] COLON reference_expr[role] COLON reference_expr[type] COLON level_range_or_id[range] {
			$$ = NODE create_context_node($user, $role, $type, $range);
			ABORT_IF_NULL($$);
		  }
		| reference_expr[user] COLON reference_expr[role] COLON reference_expr[type] {
			$$ = NODE create_context_node($user, $role, $type, NULL);
			ABORT_IF_NULL($$);
		  };

set_operator
		: AND { $$ = SECSP_SET_AND; }
		| OR { $$ = SECSP_SET_OR; }
		| XOR { $$ = SECSP_SET_XOR; }
		;

set_expression
		: identifier[id] set_operator[op] set_expression[rhs] {
			struct secsp_node *lhs = NODE create_sym_node($1);
			ABORT_IF_NULL(lhs);

			$$ = NODE create_setexpr_node(lhs, $op, $rhs);
		  }
		| identifier {
			$$ = NODE create_sym_node($1);
			ABORT_IF_NULL($$);
		  }
		;

identifier: NAME;

identifier_list
		: identifier_list[list] COMMA identifier[id] {
			struct secsp_node *sym = NODE create_sym_node($id);
			ABORT_IF_NULL(sym);

			if (secsp_node_list_insert(&$list, sym) < 0) {
				YYABORT;
			}
		  }
		| identifier {
			struct secsp_node *sym = NODE create_sym_node($1);
			ABORT_IF_NULL(sym);

			struct secsp_node_list *list;
			if (secsp_node_list_insert(&list, sym) < 0) {
				YYABORT;
			}

			$$ = list;
		  }
		;
%%

static struct secsp_context_node *
create_context_node(struct secsp_node *user, struct secsp_node *role,
			struct secsp_node *type, struct secsp_node *levelrange)
{
	struct secsp_context_node *node = NULL;
	if (secsp_context_node_new(&node, user, role, type, levelrange) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_level_node *
create_level_node(struct secsp_node *sensitivity, struct secsp_node *categories)
{
	struct secsp_level_node *node = NULL;
	if (secsp_level_node_new(&node, sensitivity, categories) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_block_node *
create_block_node(const char *name, int is_abstract,
                  struct secsp_node_list *body,
                  struct secsp_node_list *inherits)
{
	struct secsp_block_node *node = NULL;
	if (secsp_block_node_new(&node, name, is_abstract, body,
				 inherits) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_decl_node *create_decl_node(const char *name,
						enum secsp_node_flavor flavor,
						struct secsp_node *rval)
{
	struct secsp_decl_node *node = NULL;
	if (secsp_decl_node_new(&node, name, flavor, rval) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_range_node *create_range_node(enum secsp_node_flavor flavor,
						  struct secsp_node *low,
						  struct secsp_node *high)
{
	struct secsp_range_node *node = NULL;
	if (secsp_range_node_new(&node, flavor, low, high) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_sym_node *create_sym_node(const char *val)
{
	struct secsp_sym_node *node = NULL;
	if (secsp_sym_node_new(&node, val) < 0) {
		return NULL;
	}

	return node;
}

static struct secsp_setexpr_node *create_setexpr_node(struct secsp_node *lhs,
						      enum secsp_operator op,
						      struct secsp_node *rhs)
{
	struct secsp_setexpr_node *node = NULL;
	if (secsp_setexpr_node_new(&node, lhs, op, rhs) < 0) {
		return NULL;
	}

	return node;
}

void yyerror(YYLTYPE *loc, yyscan_t scanner, struct secsp_parse_result *result,
	     char const *msg)
{
	printf("Error at line %d, column %d-%d: %s\n", loc->first_line,
	       loc->first_column, loc->last_column, msg);
}
