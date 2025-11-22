#ifndef XMLVIEWERYAML_H
#define XMLVIEWERYAML_H

#include <dos/dos.h>

#include "xmlviewerexpat.h"

BOOL YamlToTree(struct XMLTree *tree, const char *filename, const char *yaml_text, size_t text_len, char *error_buf, size_t error_buf_len);
BOOL load_yaml_tree(Object *obj, BPTR fileXML, struct XMLTree *tree, char *error_buf, size_t error_buf_len);

#endif // XMLVIEWERYAML_H
