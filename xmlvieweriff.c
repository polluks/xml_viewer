#include "xmlvieweriff.h"

#include <string.h>

#include <proto/dos.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>

#include "xmlviewerdata.h"
#include "xmlviewerlocale.h"

extern struct Catalog *Cat;
extern Object *window;

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

static ULONG ReadBE32(const UBYTE *buffer)
{
    return ((ULONG)buffer[0] << 24) | ((ULONG)buffer[1] << 16) | ((ULONG)buffer[2] << 8) | (ULONG)buffer[3];
}

static void IdToString(ULONG id, char *out, size_t len)
{
    if (len < 5)
        return;

    out[0] = (char)((id >> 24) & 0xFF);
    out[1] = (char)((id >> 16) & 0xFF);
    out[2] = (char)((id >> 8) & 0xFF);
    out[3] = (char)(id & 0xFF);
    out[4] = '\0';
}

static BOOL ParseIffLevel(BPTR fileIFF, ULONG level_end, struct XMLTree *tree)
{
    ULONG current_offset;

    while ((current_offset = Seek(fileIFF, 0, OFFSET_CURRENT)) < level_end)
    {
        UBYTE header[8];
        LONG read_bytes;
        ULONG chunk_id;
        ULONG chunk_size;
        struct xml_data *node_data;
        LONG flags = 0;
        BOOL is_container = FALSE;
        char node_name[16];

        if (tree->depth + 1 >= (int)(sizeof(tree->tn) / sizeof(tree->tn[0])))
            return FALSE;

        read_bytes = Read(fileIFF, header, sizeof(header));
        if (read_bytes != sizeof(header))
            return FALSE;

        chunk_id = MAKE_ID(header[0], header[1], header[2], header[3]);
        chunk_size = ReadBE32(header + 4);
        IdToString(chunk_id, node_name, sizeof(node_name));

        if (level_end < current_offset + sizeof(header) || chunk_size > level_end - current_offset - sizeof(header))
            return FALSE;

        if (!(node_data = AllocXmlData(XML_VALUES)))
            return FALSE;

        if (chunk_id == ID_FORM || chunk_id == ID_CAT || chunk_id == ID_LIST)
        {
            flags = TNF_LIST;
            is_container = TRUE;
        }

        tree->tn[tree->depth + 1] = InsertTreeNode(tree, node_name, node_data, tree->tn[tree->depth], flags);
        if (!tree->tn[tree->depth + 1])
        {
            FreeXmlData(node_data);
            return FALSE;
        }

        if (is_container)
        {
            UBYTE type_id[4];
            ULONG content_end;

            if (chunk_size < 4)
                return FALSE;

            if (Read(fileIFF, type_id, sizeof(type_id)) != sizeof(type_id))
                return FALSE;

            chunk_size -= 4;
            IdToString(MAKE_ID(type_id[0], type_id[1], type_id[2], type_id[3]), node_name, sizeof(node_name));
            AddAttribute(node_data, "type", node_name);

            if (chunk_size > 0)
            {
                tree->depth++;
                content_end = Seek(fileIFF, 0, OFFSET_CURRENT) + chunk_size;
                if (!ParseIffLevel(fileIFF, content_end, tree))
                {
                    tree->depth--;
                    return FALSE;
                }
                tree->depth--;
                Seek(fileIFF, content_end + (chunk_size & 1), OFFSET_BEGINNING);
            }
        }
        else
        {
            ULONG skip = chunk_size + (chunk_size & 1);
            if (Seek(fileIFF, skip, OFFSET_CURRENT) == -1)
                return FALSE;
        }
    }

    return TRUE;
}

BOOL load_iff_tree(Object *obj, BPTR fileIFF, struct XMLTree *tree, char *error_buf, size_t error_buf_len)
{
    LONG file_size;
    struct xml_data *root_data;
    BOOL success = FALSE;

    if (error_buf && error_buf_len)
        error_buf[0] = '\0';

    if ((file_size = Seek(fileIFF, 0, OFFSET_END)) > 0)
    {
        if (Seek(fileIFF, 0, OFFSET_BEGINNING) != -1)
        {
            if ((root_data = AllocXmlData(XML_VALUES)))
            {
                tree->depth = 0;
                tree->tn[0] = InsertTreeNode(tree, tree->filename, root_data, MUIV_Listtree_Insert_ListNode_Root, TNF_LIST);

                if (tree->tn[0])
                {
                    success = ParseIffLevel(fileIFF, file_size, tree);
                }
                else
                {
                    FreeXmlData(root_data);
                }
            }

            if (!success)
                snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_IFF_PARSE_ERROR, "Unable to parse IFF structure"));
        }
        else
        {
            snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_IFF_READ_ERROR, "Unable to rewind IFF file"));
        }
    }
    else
    {
        snprintf(error_buf, error_buf_len, "%s", GetCatalogStr(Cat, MSG_IFF_READ_ERROR, "Unable to rewind IFF file"));
    }

    if (!success)
    {
        MUI_RequestA(obj, window, 0, GetCatalogStr(Cat, MSG_ERROR_TITLE, "XML Viewer Error"), "*OK", error_buf, NULL);
    }

    return success;
}
