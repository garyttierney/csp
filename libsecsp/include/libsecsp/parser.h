#ifndef LIBSECSP_PARSER_H
#define LIBSECSP_PARSER_H

#include <stdio.h>

struct secsp_parse_result;
struct secsp_node_list;

int secsp_parse_file(FILE *input, struct secsp_parse_result **result);

struct secsp_node_list *secsp_get_root_node(struct secsp_parse_result *result);

#endif
