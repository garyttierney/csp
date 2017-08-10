#include <libsecsp/ast.h>
#include <libsecsp/parser.h>

static void print_cil_node(struct secsp_node *node);
static void print_cil_node_wrapped(struct secsp_node *node, int wrap_symbols);

static void print_block_inherits(struct secsp_node *node)
{
	enum secsp_node_flavor flav = secsp_node_get_flavor(node);
	if (flav != SECSP_SYMBOL) {
		return;
	}

	struct secsp_sym_node *sym = (struct secsp_sym_node *) node;
	printf("(blockinherits %s)\n", secsp_sym_value(sym));
}

static void print_block_node(struct secsp_block_node *block_node) {
	const char *name = secsp_block_get_name(block_node);
	int is_abstract = secsp_block_is_abstract(block_node);

	printf("(block %s\n", secsp_block_get_name(block_node));
	if (is_abstract) {
		printf("(blockabstract %s)\n", name);
	}

	struct secsp_node_list *inherits = secsp_block_get_parents(block_node);
	secsp_node_list_iterate(inherits, print_block_inherits);

	printf(")\n");
}

static void print_decl_node(struct secsp_decl_node *node)
{
	const char *name = secsp_decl_get_name(node);
	const char *type = secsp_decl_get_type(node);

	printf("(%s %s", type, name);

	if (secsp_decl_has_initializer(node)) {
		printf(" ");
		print_cil_node(secsp_decl_get_initializer(node));
	}

	printf(")\n");
}

static void print_level_node(struct secsp_level_node *node)
{
	struct secsp_node *sensitivity = secsp_level_get_sensitivity(node);
	printf("(");
	print_cil_node(sensitivity);

	if (secsp_level_has_categories(node)) {
		print_cil_node(secsp_level_get_categories(node));
	}

	printf(")");
}


static void print_category_range_node(struct secsp_range_node *node)
{
	struct secsp_node *low = secsp_range_get_low(node);
	struct secsp_node *high = secsp_range_get_high(node);

	printf("(range ");
	print_cil_node(low);
	printf(" ");
	print_cil_node(high);
	printf(")");
}

static void print_level_range_node(struct secsp_range_node *node)
{
	struct secsp_node *low = secsp_range_get_low(node);
	struct secsp_node *high = secsp_range_get_high(node);

	printf("(");
	print_cil_node(low);
	printf(" ");
	print_cil_node(high);
	printf(")");
}

static void print_context_node(struct secsp_context_node *node)
{
	struct secsp_node *user = secsp_context_get_user(node);
	struct secsp_node *role = secsp_context_get_role(node);
	struct secsp_node *type = secsp_context_get_type(node);

	printf("(");
	print_cil_node(user);
	printf(" ");
	print_cil_node(role);
	printf(" ");
	print_cil_node(type);

	if (secsp_context_has_levelrange(node)) {
		printf(" ");
		print_cil_node(secsp_context_get_levelrange(node));
	}

	printf(")");
}

static void print_sym_node(struct secsp_sym_node *node, int wrap)
{
	if (wrap == 1) {
		printf("(%s)", secsp_sym_value(node));
	} else {
		printf("%s", secsp_sym_value(node));
	}
}

static void print_cil_node_wrapped(struct secsp_node *node, int wrap_symbols)
{
	enum secsp_node_flavor flav = secsp_node_get_flavor(node);
	switch (flav) {
	case SECSP_BLOCK:
		print_block_node((struct secsp_block_node *)node);
		break;
	case SECSP_DECL:
		print_decl_node((struct secsp_decl_node *)node);
		break;
	case SECSP_CONTEXT_EXPR:
		print_context_node((struct secsp_context_node *)node);
		break;
	case SECSP_LEVEL_EXPR:
		print_level_node((struct secsp_level_node *)node);
		break;
	case SECSP_LEVEL_RANGE_EXPR:
		print_level_range_node((struct secsp_range_node *)node);
		break;
	case SECSP_CATEGORY_RANGE_EXPR:
		print_category_range_node((struct secsp_range_node *)node);
		break;
	case SECSP_SYMBOL:
		print_sym_node((struct secsp_sym_node *) node, wrap_symbols);
		break;
	default:
		return;
	}
}

static void print_cil_node(struct secsp_node *node)
{
	print_cil_node_wrapped(node, 0);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: secspc <filename>\n");
		return -1;
	}

	FILE *file = fopen(argv[1], "r");
	struct secsp_parse_result *result = NULL;
	if (secsp_parse_file(file, &result) < 0) {
		return -1;
	}
	fclose(file);

	struct secsp_node_list *root = secsp_get_root_node(result);
	if (root == NULL) {
		printf("no input\n");
		return 1;
	}

	secsp_node_list_iterate(root, print_cil_node);
	return 0;
}
