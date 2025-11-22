#ifndef XMLVIEWERFILETYPE_H
#define XMLVIEWERFILETYPE_H

#include <exec/types.h>
#include <dos/dos.h>

#include <stddef.h>

enum XMLViewerFileType
{
    FILE_TYPE_XML = 0,
    FILE_TYPE_JSON,
    FILE_TYPE_YAML,
    FILE_TYPE_IFF,
    FILE_TYPE_UNKNOWN
};

struct FileTypeInfo
{
    enum XMLViewerFileType detected;
    BOOL has_utf8_bom;
    BOOL header_read_failure;
};

BOOL DetectFileType(BPTR file, const char *filename, struct FileTypeInfo *info, char *error_buf, size_t error_buf_len);
const char *GetFileTypeName(enum XMLViewerFileType type);

#endif // XMLVIEWERFILETYPE_H
