#include <libsecsp/ast.h>
#include <libsecsp/parser.h>

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

static void print_node(struct secsp_node *node)
{
	enum secsp_node_flavor flav = secsp_node_get_flavor(node);
	switch (flav) {
	case SECSP_BLOCK:
		print_block_node((struct secsp_block_node *)node);
		break;
	default:
		return;
	}
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

	secsp_node_list_iterate(root, print_node);
	return 0;
}
