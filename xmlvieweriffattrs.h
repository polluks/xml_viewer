#ifndef XMLVIEWERIFFATTRS_H
#define XMLVIEWERIFFATTRS_H

#include <dos/dos.h>

#include "xmlviewerexpat.h"

BOOL ParseIffChunkAttributes(BPTR fileIFF, ULONG chunk_id, ULONG chunk_size, struct xml_data *node_data);

#endif // XMLVIEWERIFFATTRS_H
