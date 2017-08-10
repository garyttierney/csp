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
	char *type;
	char *name;
	struct secsp_node *rval;
};

struct secsp_context_node {
	struct secsp_node tag;
	struct secsp_node *user;
	struct secsp_node *role;
	struct secsp_node *type;
	struct secsp_node *levelrange;
};

struct secsp_level_node {
	struct secsp_node tag;
	struct secsp_node *sensitivity;
	struct secsp_node *categories;
};

struct secsp_range_node {
	struct secsp_node tag;
	struct secsp_node *low;
	struct secsp_node *high;
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
					const char *type,
					struct secsp_node *rval)
{
	struct secsp_decl_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_DECL;
	node->type = strdup(type);
	node->name = strdup(name);
	node->rval = rval;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT const char *secsp_decl_get_name(struct secsp_decl_node *node)
{
	return node->name;
}

LIBSECSP_EXPORT const char *secsp_decl_get_type(struct secsp_decl_node *node)
{
	return node->type;
}

LIBSECSP_EXPORT int secsp_decl_has_initializer(struct secsp_decl_node *node)
{
	return node->rval != NULL;
}

LIBSECSP_EXPORT struct secsp_node *secsp_decl_get_initializer(struct secsp_decl_node *node)
{
	return node->rval;
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

LIBSECSP_EXPORT int secsp_context_node_new(struct secsp_context_node **out,
					   struct secsp_node *user,
					   struct secsp_node *role,
					   struct secsp_node *type,
					   struct secsp_node *levelrange)
{
	struct secsp_context_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_CONTEXT_EXPR;
	node->user = user;
	node->role = role;
	node->type = type;
	node->levelrange = levelrange;

	*out = node;
	return 0;
}

LIBSECSP_EXPORT struct secsp_node* secsp_context_get_user(struct secsp_context_node *node)
{
	return node->user;
}
LIBSECSP_EXPORT struct secsp_node* secsp_context_get_role(struct secsp_context_node *node)
{
	return node->role;
}
LIBSECSP_EXPORT struct secsp_node* secsp_context_get_type(struct secsp_context_node *node)
{
	return node->type;
}

LIBSECSP_EXPORT int secsp_context_has_levelrange(struct secsp_context_node *node)
{
	return node->levelrange != NULL;
}

LIBSECSP_EXPORT struct secsp_node* secsp_context_get_levelrange(struct secsp_context_node *node)
{
	return node->levelrange;
}

LIBSECSP_EXPORT int secsp_range_node_new(struct secsp_range_node **out,
				  enum secsp_node_flavor type,
				  struct secsp_node *low,
				  struct secsp_node *high)
{
	struct secsp_range_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = type;
	node->low = low;
	node->high = high;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT struct secsp_node *secsp_range_get_low(struct secsp_range_node *node)
{
	return node->low;
}

LIBSECSP_EXPORT struct secsp_node *secsp_range_get_high(struct secsp_range_node *node)
{
	return node->high;
}

LIBSECSP_EXPORT int secsp_level_node_new(struct secsp_level_node **out,
					 struct secsp_node *sensitivity,
					 struct secsp_node *categories)
{
	struct secsp_level_node *node = calloc(1, sizeof *node);
	if (!node) {
		return -ENOMEM;
	}

	node->tag.flavor = SECSP_LEVEL_EXPR;
	node->sensitivity = sensitivity;
	node->categories = categories;
	*out = node;

	return 0;
}

LIBSECSP_EXPORT struct secsp_node *secsp_level_get_sensitivity(struct secsp_level_node *node)
{
	return node->sensitivity;
}

LIBSECSP_EXPORT int secsp_level_has_categories(struct secsp_level_node *node)
{
	return node->categories != NULL;
}

LIBSECSP_EXPORT struct secsp_node *secsp_level_get_categories(struct secsp_level_node *node)
{
	return node->categories;
}
