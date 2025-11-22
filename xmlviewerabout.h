/* xmlviewerabout.c - XML metafomat viewer
** about window object header
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008 Michal Zukowski
*/
#ifndef XMLVIEWERABOUT_H
#define XMLVIEWERABOUT_H

#ifdef __MORPHOS__
#define USE_HYPERLINK_MCC
#include <mui/Aboutbox_mcc.h>
#include <mui/urltext_mcc.h>
#endif

Object *CreateAboutWindow();

#endif