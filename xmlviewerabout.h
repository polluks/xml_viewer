/* xmlviewerabout.c - XML metafomat viewer
** about window object header
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008 Michal Zukowski
** gcc -noixemul -c *.c -I. -DYAML_VERSION_MAJOR=0 -DYAML_VERSION_MINOR=2 -DYAML_VERSION_PATCH=5 -DYAML_VERSION_STRING="1.0"
*/

#ifndef XMLVIEWERABOUT_H
#define XMLVIEWERABOUT_H

#ifdef __MORPHOS__
#define USE_HYPERLINK_MCC
#include <mui/Aboutbox_mcc.h>

#endif

Object *CreateAboutWindow();

#endif