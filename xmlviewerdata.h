#ifndef XMLVIEWERDATA_H
#define XMLVIEWERDATA_H

#include <exec/types.h>
#include <mui/Listtree_mcc.h>

#include "xmlviewerexpat.h"

struct xml_data *AllocXmlData(long type);
void FreeXmlData(struct xml_data *data);
BOOL AddAttribute(struct xml_data *data, const char *attr, const char *value);
struct MUIS_Listtree_TreeNode *InsertTreeNode(struct XMLTree *tree, const char *name, struct xml_data *node_data, APTR parent, LONG flags);

#endif // XMLVIEWERDATA_H
