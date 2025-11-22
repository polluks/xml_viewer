#include "xmlviewerfiletype.h"

#include <string.h>
#include <ctype.h>

#include <proto/dos.h>
#include <proto/utility.h>
#include <libraries/iffparse.h>

static enum XMLViewerFileType GuessFromExtension(const char *filename)
{
    const char *ext;

    if (!filename)
        return FILE_TYPE_UNKNOWN;

    if ((ext = strrchr(filename, '.')))
    {
        if (Stricmp(ext, ".xml") == 0)
            return FILE_TYPE_XML;
        if (Stricmp(ext, ".json") == 0)
            return FILE_TYPE_JSON;
        if (Stricmp(ext, ".yaml") == 0 || Stricmp(ext, ".yml") == 0)
            return FILE_TYPE_YAML;
        if (Stricmp(ext, ".iff") == 0)
            return FILE_TYPE_IFF;
    }

    return FILE_TYPE_UNKNOWN;
}

static BOOL IsXmlHeader(const UBYTE *buffer, LONG length, LONG offset)
{
    while (offset < length && isspace(buffer[offset]))
        offset++;

    if (length - offset < 5)
        return FALSE;

    return strncmp((const char *)(buffer + offset), "<?xml", 5) == 0;
}

static BOOL IsJsonHeader(const UBYTE *buffer, LONG length, LONG offset)
{
    while (offset < length && isspace(buffer[offset]))
        offset++;

    if (offset >= length)
        return FALSE;

    return buffer[offset] == '{' || buffer[offset] == '[';
}

static BOOL IsYamlHeader(const UBYTE *buffer, LONG length, LONG offset)
{
    while (offset < length && isspace(buffer[offset]))
        offset++;

    if (length - offset >= 5 && strncmp((const char *)(buffer + offset), "%YAML", 5) == 0)
        return TRUE;

    if (length - offset >= 3 && buffer[offset] == '-' && buffer[offset + 1] == '-' && buffer[offset + 2] == '-')
        return TRUE;

    return offset < length && buffer[offset] == '-';
}

static BOOL IsIffHeader(const UBYTE *buffer, LONG length, LONG offset)
{
    ULONG id;

    if (length - offset < 4)
        return FALSE;

    id = MAKE_ID(buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3]);

    return id == ID_FORM || id == ID_CAT;
}

static BOOL CheckHeaderForType(enum XMLViewerFileType type, const UBYTE *buffer, LONG length, LONG offset)
{
    switch (type)
    {
        case FILE_TYPE_XML:
            return IsXmlHeader(buffer, length, offset);
        case FILE_TYPE_JSON:
            return IsJsonHeader(buffer, length, offset);
        case FILE_TYPE_YAML:
            return IsYamlHeader(buffer, length, offset);
        case FILE_TYPE_IFF:
            return IsIffHeader(buffer, length, offset);
        default:
            break;
    }

    return FALSE;
}

static void BuildOrder(enum XMLViewerFileType preferred, enum XMLViewerFileType *order, int *count)
{
    int idx = 0;
    int i;
    enum XMLViewerFileType types[] = {preferred, FILE_TYPE_XML, FILE_TYPE_JSON, FILE_TYPE_YAML, FILE_TYPE_IFF};

    for (i = 0; i < (int)(sizeof(types) / sizeof(types[0])); ++i)
    {
        BOOL already_added = FALSE;
        int j;

        if (types[i] == FILE_TYPE_UNKNOWN)
            continue;

        for (j = 0; j < idx; ++j)
        {
            if (order[j] == types[i])
            {
                already_added = TRUE;
                break;
            }
        }

        if (!already_added)
            order[idx++] = types[i];
    }

    *count = idx;
}

BOOL DetectFileType(BPTR file, const char *filename, struct FileTypeInfo *info, char *error_buf, size_t error_buf_len)
{
    UBYTE buffer[16];
    LONG len;
    LONG offset = 0;
    enum XMLViewerFileType preferred;
    enum XMLViewerFileType order[5];
    int order_count = 0;
    int i;

    if (!file || !info)
        return FALSE;

    preferred = GuessFromExtension(filename);
    info->has_utf8_bom = FALSE;
    info->header_read_failure = FALSE;
    info->detected = FILE_TYPE_UNKNOWN;

    len = Read(file, buffer, sizeof(buffer));
    Seek(file, 0, OFFSET_BEGINNING);

    if (len <= 0)
    {
        info->header_read_failure = TRUE;
        return FALSE;
    }

    info->has_utf8_bom = (len >= 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF);
    if (info->has_utf8_bom)
        offset = 3;

    BuildOrder(preferred, order, &order_count);

    for (i = 0; i < order_count; ++i)
    {
        if (CheckHeaderForType(order[i], buffer, len, offset))
        {
            info->detected = order[i];
            return TRUE;
        }
    }

    if (preferred != FILE_TYPE_UNKNOWN)
    {
        info->detected = preferred;
        return TRUE;
    }

    info->detected = FILE_TYPE_UNKNOWN;

    return FALSE;
}

const char *GetFileTypeName(enum XMLViewerFileType type)
{
    switch (type)
    {
        case FILE_TYPE_XML:
            return "XML";
        case FILE_TYPE_JSON:
            return "JSON";
        case FILE_TYPE_YAML:
            return "YAML";
        case FILE_TYPE_IFF:
            return "IFF";
        default:
            break;
    }

    return "Unknown";
}
