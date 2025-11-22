#ifndef XMLVIEWERIFF_H
#define XMLVIEWERIFF_H

#include <dos/dos.h>

#include "xmlviewerexpat.h"

BOOL load_iff_tree(Object *obj, BPTR fileIFF, struct XMLTree *tree, char *error_buf, size_t error_buf_len);

#endif // XMLVIEWERIFF_H
