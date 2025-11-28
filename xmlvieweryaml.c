#include "xmlvieweryaml.h"

#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <dos/dos.h>
#include <yaml.h>

#include "xmlviewerdata.h"
#include "xmlviewerlocale.h"

extern struct Catalog *Cat;
extern Object *window;

static BOOL AttachYamlScalar(struct xml_data *data, yaml_node_t *node)
{
    char *buffer;

    if (!data || !node || node->type != YAML_SCALAR_NODE)
        return FALSE;

    if ((buffer = (char *)AllocVec(node->data.scalar.length + 1, MEMF_ANY)))
    {
        CopyMem(node->data.scalar.value, buffer, node->data.scalar.length);
        buffer[node->data.scalar.length] = '\0';
        AddAttribute(data, "value", buffer);
        FreeVec(buffer);
        return TRUE;
    }

    return FALSE;
}

static BOOL InsertYamlNode(struct XMLTree *tree, yaml_document_t *document, yaml_node_t *node, const char *name)
{
    struct xml_data *node_data;
    LONG flags = 0;
    BOOL success = FALSE;
    BOOL pushed_depth = FALSE;

    if (tree->depth + 1 >= (int)(sizeof(tree->tn) / sizeof(tree->tn[0])))
        return FALSE;

    if (node->type == YAML_MAPPING_NODE || node->type == YAML_SEQUENCE_NODE)
        flags = TNF_LIST;

    if (!(node_data = AllocXmlData(XML_VALUES)))
        return FALSE;

    tree->tn[tree->depth + 1] = InsertTreeNode(tree, name, node_data, tree->tn[tree->depth], flags);
    if (!tree->tn[tree->depth + 1])
    {
        FreeXmlData(node_data);
        return FALSE;
    }

    if (node->type == YAML_MAPPING_NODE)
    {
        yaml_node_pair_t *pair;

        pushed_depth = TRUE;
        tree->depth++;

        for (pair = node->data.mapping.pairs.start; pair < node->data.mapping.pairs.top; ++pair)
        {
            yaml_node_t *key_node = yaml_document_get_node(document, pair->key);
            yaml_node_t *value_node = yaml_document_get_node(document, pair->value);
            char key_buffer[64];
            const char *key_name = "<mapping>";

            if (key_node && key_node->type == YAML_SCALAR_NODE && key_node->data.scalar.length < sizeof(key_buffer))
            {
                CopyMem(key_node->data.scalar.value, key_buffer, key_node->data.scalar.length);
                key_buffer[key_node->data.scalar.length] = '\0';
                key_name = key_buffer;
            }

            if (value_node)
            {
                if (!InsertYamlNode(tree, document, value_node, key_name))
                    goto cleanup;
            }
        }

        success = TRUE;
    }
    else if (node->type == YAML_SEQUENCE_NODE)
    {
        yaml_node_item_t *item;
        int idx = 0;
        char idx_name[16];

        pushed_depth = TRUE;
        tree->depth++;

        for (item = node->data.sequence.items.start; item < node->data.sequence.items.top; ++item)
        {
            yaml_node_t *child = yaml_document_get_node(document, *item);
            snprintf(idx_name, sizeof(idx_name), "[%d]", idx++);
            if (child)
            {
                if (!InsertYamlNode(tree, document, child, idx_name))
                    goto cleanup;
            }
        }

        success = TRUE;
    }
    else if (node->type == YAML_SCALAR_NODE)
    {
        success = AttachYamlScalar(node_data, node);
    }

cleanup:
    if (pushed_depth && tree->depth > 0)
        tree->depth--;

    return success;
}

BOOL YamlToTree(struct XMLTree *tree, const char *filename, const char *yaml_text, size_t text_len, char *error_buf, size_t error_buf_len)
{
    yaml_parser_t parser;
    yaml_document_t document;
    yaml_node_t *root;
    struct xml_data *root_data;
    BOOL rc = FALSE;

    if (!tree || !filename || !yaml_text)
        return FALSE;

    if (!yaml_parser_initialize(&parser))
    {
        if (error_buf && error_buf_len)
            snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_INIT_ERROR, "Unable to initialize YAML parser"));
        return FALSE;
    }

    yaml_parser_set_input_string(&parser, (const unsigned char *)yaml_text, text_len);

    if (!yaml_parser_load(&parser, &document))
    {
        if (error_buf && error_buf_len)
            snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_PARSE_ERROR, "Error parsing YAML file"));
        yaml_parser_delete(&parser);
        return FALSE;
    }

    if (!(root = yaml_document_get_root_node(&document)))
    {
        if (error_buf && error_buf_len)
            snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_EMPTY, "YAML file is empty"));
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return FALSE;
    }

    if (!(root_data = AllocXmlData(XML_VALUES)))
    {
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return FALSE;
    }

    tree->depth = 0;
    tree->tn[0] = InsertTreeNode(tree, filename, root_data, MUIV_Listtree_Insert_ListNode_Root, TNF_LIST);

    if (!tree->tn[0])
    {
        FreeXmlData(root_data);
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return FALSE;
    }

    if (root->type == YAML_MAPPING_NODE)
    {
        yaml_node_pair_t *pair;

        for (pair = root->data.mapping.pairs.start; pair < root->data.mapping.pairs.top; ++pair)
        {
            yaml_node_t *key_node = yaml_document_get_node(&document, pair->key);
            yaml_node_t *value_node = yaml_document_get_node(&document, pair->value);
            char key_buffer[64];
            const char *key_name = "<mapping>";

            if (key_node && key_node->type == YAML_SCALAR_NODE && key_node->data.scalar.length < sizeof(key_buffer))
            {
                CopyMem(key_node->data.scalar.value, key_buffer, key_node->data.scalar.length);
                key_buffer[key_node->data.scalar.length] = '\0';
                key_name = key_buffer;
            }

            if (value_node)
            {
                if (!InsertYamlNode(tree, &document, value_node, key_name))
                    goto cleanup_root;
            }
        }

        rc = TRUE;
    }
    else if (root->type == YAML_SEQUENCE_NODE)
    {
        yaml_node_item_t *item;
        int idx = 0;
        char idx_name[16];

        for (item = root->data.sequence.items.start; item < root->data.sequence.items.top; ++item)
        {
            yaml_node_t *child = yaml_document_get_node(&document, *item);
            snprintf(idx_name, sizeof(idx_name), "[%d]", idx++);
            if (child)
            {
                if (!InsertYamlNode(tree, &document, child, idx_name))
                    goto cleanup_root;
            }
        }

        rc = TRUE;
    }
    else if (root->type == YAML_SCALAR_NODE)
    {
        rc = AttachYamlScalar(root_data, root);
    }

cleanup_root:

    yaml_document_delete(&document);
    yaml_parser_delete(&parser);

    return rc;
}

BOOL load_yaml_tree(Object *obj, BPTR fileXML, struct XMLTree *tree, char *error_buf, size_t error_buf_len)
{
    char *yaml_buf = NULL;
    BOOL success = FALSE;
    const char *input_ptr;
    size_t input_len;

	struct FileInfoBlock *fib;
	
    if ((fib = AllocDosObject(DOS_FIB, NULL)))
    {
		if (ExamineFH(fileXML, fib))
        {
			LONG file_size = fib->fib_Size;

            if ((yaml_buf = (char *)AllocVec(file_size + 1, MEMF_ANY)))
            {
                if (Read(fileXML, yaml_buf, file_size) == file_size)
                {
                    yaml_buf[file_size] = '\0';

                    input_ptr = yaml_buf;
                    input_len = file_size;

                    if (tree->has_utf8_bom && input_len >= 3 && (UBYTE)input_ptr[0] == 0xEF && (UBYTE)input_ptr[1] == 0xBB && (UBYTE)input_ptr[2] == 0xBF)
                    {
                        input_ptr += 3;
                        input_len -= 3;
                    }

                    if (YamlToTree(tree, tree->filename, input_ptr, input_len, error_buf, error_buf_len))
                    {
                        success = TRUE;
                    }
                    else if (error_buf && !error_buf[0])
                    {
                        snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_PARSE_ERROR, "Error parsing YAML file"));
                    }
                }
                else
                {
                    snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_READ_ERROR, "Error reading YAML file"));
                }

                FreeVec(yaml_buf);
            }
            else
            {
                snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_ALLOC_ERROR, "Unable to allocate buffer for YAML file"));
            }
        }
		FreeDosObject(DOS_FIB, fib);
    }
    else
    {
        snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_YAML_EMPTY, "YAML file is empty"));
    }

    if (!success)
    {
        MUI_RequestA(obj, window, 0, GetCatalogStr(Cat, MSG_ERROR_TITLE, "XML Viewer Error"), "*OK", error_buf, NULL);
    }

    return success;
}
