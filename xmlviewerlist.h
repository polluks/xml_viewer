/* xmlviewerlist.h - XML metafomat viewer
**
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008-2009 Michal Zukowski
*/

#include <proto/exec.h>
#include <proto/muimaster.h>
#include "api_MUI.h"

struct Data
{
    char name[128];
    char value[128];
};

M_HOOK_h(DataDestructor)
M_HOOK_h(DataConstructor)
M_HOOK_h(DataDisplayer)
