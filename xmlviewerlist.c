/* xmlviewerlist.c - XML metafomat viewer
**
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008-2009 Michal Zukowski
*/

#include <stdio.h>
#include <ctype.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>

#include "xmlviewerlist.h"
#include "xmlviewerlocale.h"
#include "api_MUI.h"
#include "resources.h"

// locales
extern struct Catalog *Cat;

/// DataConstructor()
M_HOOK(DataConstructor, APTR mempool, APTR dana)
{
    struct Data * krotka  = (struct Data *)dana;
    struct Data *t_copy = NULL;

    if ((t_copy = AllocPooled (mempool, sizeof (struct Data))))
    {
	strcpy(t_copy->name, krotka->name);
	strcpy(t_copy->value, krotka->value);
    }

    return (LONG)t_copy;
}
///
/// DataDestructor()
M_HOOK(DataDestructor, APTR mempool, APTR dana)
{
    struct Data * krotka  = (struct Data *)dana;
    if (krotka) FreePooled (mempool, krotka, sizeof (struct Data));

    return 0;
}


///
/// DataDisplayer()
M_HOOK(DataDisplayer, APTR mempool, APTR dana)
{
    struct Data * krotka  = (struct Data *)dana;
    char ** strings = (char ** )mempool;
    static char x[128], y[128];
    int i = (int)strings[-1];

#ifdef __MORPHOS__
// zebra
    strings[-9] = (i & 1) ? 0 : 10;
#endif

    //labels
    if (!krotka)
    {
        strings[0]   = GetCatalogStr(Cat, MSG_LIST_ATTR, "Attribute");
        strings[1]   = GetCatalogStr(Cat, MSG_LIST_VAL, "Value");
    }
    else
    {
    //	  snprintf (x, 128, "\033P[FFFFFF]%s", krotka->name);       kolor bia³y
        snprintf (x, 128, "%s", krotka->name);
	strings[0] = x;

    	snprintf (y, 128, "%s", krotka->value);
    	strings[1] = y;
    }

    return 0;
}
///
