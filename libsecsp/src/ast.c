#include "libsecsp-private.h"
#include <libsecsp/ast.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct secsp_node {
	enum secsp_node_flavor flavor;
};

struct secsp_sym_node {
	struct secsp_node tag;
	char *val;
};

struct secsp_node_list {
	struct secsp_node *node;
	struct secsp_node_list *next;
};

struct secsp_block_node {
	struct secsp_node tag;
	char *name;
	int abstract;
	struct secsp_node_list *statements;
	struct secsp_node_list *parents;
};

struct secsp_decl_node {
	struct secsp_node tag;
	char *name;
	struct secsp_node *rval;
};

struct secsp_setexpr_node {
	struct secsp_node tag;
	struct secsp_node *lhs;
	enum secsp_operator operator;
	struct secsp_node *rhs;
};

LIBSECSP_EXPORT enum secsp_node_flavor
secsp_node_get_flavor(struct secsp_node *node)
{
	return node->flavor;
}

LIBSECSP_EXPORT int secsp_sym_node_new(struct secsp_sym_node **out,
				       const char *val)
{
	char *cpy = strdup(val);
	if (cpy == NULL) {
		return -ENOMEM;
	}

	struct secsp_sym_node *node = calloc(1, sizeof *node);
	if (node == NULL) {
		free(cpy);
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_SYMBOL;
	node->val = cpy;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT const char *secsp_sym_value(struct secsp_sym_node *sym)
{
	return sym->val;
}

LIBSECSP_EXPORT int secsp_node_list_insert(struct secsp_node_list **list,
					   struct secsp_node *node)
{
	if (*list == NULL) {
		struct secsp_node_list *tmp = calloc(1, sizeof *tmp);
		tmp = calloc(1, sizeof *tmp);
		if (!tmp) {
			return -ENOMEM;
		}

		tmp->node = node;
		*list = tmp;
		return 0;
	}

	struct secsp_node_list *tmp = *list;
	while (tmp->next != NULL) {
		tmp = tmp->next;
	}

	struct secsp_node_list *new = calloc(1, sizeof *new);
	if (!new) {
		return -ENOMEM;
	}

	new->node = node;
	tmp->next = new;

	return 0;
}

LIBSECSP_EXPORT void
secsp_node_list_iterate(struct secsp_node_list *list,
			void (*iterator)(struct secsp_node *))
{
	while (list != NULL) {
		iterator(list->node);
		list = list->next;
	}
}

LIBSECSP_EXPORT int secsp_block_node_new(struct secsp_block_node **out,
					 const char *name, int abstract,
					 struct secsp_node_list *statements,
					 struct secsp_node_list *parents)
{
	struct secsp_block_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_BLOCK;
	node->name = strdup(name);
	node->abstract = abstract;
	node->statements = statements;
	node->parents = parents;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT int secsp_block_is_abstract(struct secsp_block_node *node)
{
	return node->abstract;
}

LIBSECSP_EXPORT const char *secsp_block_get_name(struct secsp_block_node *node)
{
	return node->name;
}

LIBSECSP_EXPORT struct secsp_node_list *
secsp_block_get_parents(struct secsp_block_node *node)
{
	return node->parents;
}

LIBSECSP_EXPORT struct secsp_node_list *
secsp_blocp_get_statements(struct secsp_block_node *node)
{
	return node->statements;
}

LIBSECSP_EXPORT int secsp_decl_node_new(struct secsp_decl_node **out,
					const char *name,
					enum secsp_node_flavor flavor,
					struct secsp_node *rval)
{
	if (flavor <= SECSP_DECL_START || flavor >= SECSP_DECL_END) {
		return -1;
	}

	struct secsp_decl_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = flavor;
	node->name = strdup(name);
	node->rval = rval;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT int secsp_setexpr_node_new(struct secsp_setexpr_node **out,
					   struct secsp_node *lhs,
					   enum secsp_operator op,
					   struct secsp_node *rhs)
{
	struct secsp_setexpr_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_SET_EXPR;
	node->lhs = lhs;
	node->operator= op;
	node->rhs = rhs;
	*out = node;

	return 0;
}
