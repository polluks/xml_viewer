/* xmlviewertree.h - XML metafomat viewer
** listree hooks header
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008-2009 Michal Zukowski
*/


#include <clib/alib_protos.h>
#include <intuition/classusr.h>
#include "api_MUI.h"

#define MUIM_LTree_Save OM_Dummy+1200
#define MUIM_LTree_Paste OM_Dummy+1201

M_HOOK_h(active);
struct MUIP_LTreeFile {ULONG methodid; ULONG file; ULONG node; ULONG mode;};   // gdzie file to uchwyt do pliku
struct MUI_CustomClass *CreatexmlviewertreeClass (void);
