#ifndef LIBSECSP_AST_H
#define LIBSECSP_AST_H

/**
 * @brief The different flavors of <code>secsp_node</code>s
 * that can appear in the AST.
 */
enum secsp_node_flavor {
	SECSP_SYMBOL,
	SECSP_BLOCK,
	SECSP_DECL_START,
	SECSP_TYPE_DECL,
	SECSP_TYPE_ATTRIBUTE_DECL,
	SECSP_ROLE_DECL,
	SECSP_ROLE_ATTRIBUTE_DECL,
	SECSP_USER_DECL,
	SECSP_CONTEXT_DECL,
	SECSP_DECL_END,
	SECSP_SET_EXPR
};

enum secsp_operator {
	SECSP_SET_AND,
	SECSP_SET_XOR,
	SECSP_SET_NOT,
	SECSP_SET_OR
};

struct secsp_node;

enum secsp_node_flavor secsp_node_get_flavor(struct secsp_node *);

struct secsp_sym_node;

int secsp_sym_node_new(struct secsp_sym_node **node, const char *val);

const char *secsp_sym_value(struct secsp_sym_node *node);

/**
 * @brief A list of AST nodes.
 */
struct secsp_node_list;

int secsp_node_list_insert(struct secsp_node_list **list,
			   struct secsp_node *node);

void secsp_node_list_iterate(struct secsp_node_list *list,
			     void (*iterator)(struct secsp_node *));
/*
 * @brief An AST node representing a namespace block.
 */
struct secsp_block_node;

/**
 * Create a new namespace with the given <code>name</code>.
 *
 * \in name The name of the block.
 * \in abstract A flag indicating if other namespaces can inherit from this
 * one.
 * \in statements A list of statements contained in this namespace.
 * \in parents A list of namespaces this block inherits from.
 */
int secsp_block_node_new(struct secsp_block_node **out, const char *name,
			 int is_abstract, struct secsp_node_list *statements,
			 struct secsp_node_list *parents);

int secsp_block_is_abstract(struct secsp_block_node *);

const char *secsp_block_get_name(struct secsp_block_node *);

struct secsp_node_list *secsp_block_get_statements(struct secsp_block_node *);

struct secsp_node_list *secsp_block_get_parents(struct secsp_block_node *);

/**
 * @brief An AST node representing a declaration;
 */
struct secsp_decl_node;

/**
 * @brief Create a new named symbol declaration.
 *
 * Allocates and returns a new <code>secsp_decl_node</code>.  The
 * <code>flavor</code> given must be one of the valid declaration flavors:
 * @code{.c}
 * enum secsp_node_flavor {
 *	// ...
 *	SECSP_TYPE_DECL,
 *	SECSP_TYPE_ATTRIBUTE_DECL,
 *	SECSP_ROLE_DECL,
 *	SECSP_ROLE_ATTRIBUTE_DECL,
 *	SECSP_CONTEXT_DECL
 *	// ...
 * };
 * @endcode
 *
 * @param name The name of the symbol being declared.
 * @param flavor The flavor of the declaration node.
 * @param rval The initial (or constant) value of the symbol being declared.
 */
int secsp_decl_node_new(struct secsp_decl_node **node, const char *name,
			enum secsp_node_flavor flavor, struct secsp_node *rval);

struct secsp_setexpr_node;

int secsp_setexpr_node_new(struct secsp_setexpr_node **node,
			    struct secsp_node *lhs,
			    enum secsp_operator oper,
			    struct secsp_node *rhs);

#endif
