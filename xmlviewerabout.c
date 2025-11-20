/* xmlviewerabout.c - XML metafomat viewer
** about window object
**
** Part of Plot.mcc MUI custom class package.
** (c) 2025 Michal Zukowski
*/
#define USE_INLINE_STDARG

#define USE_HYPERLINK_MCC

#include <proto/exec.h>
#include <proto/muimaster.h>
#include "xmlviewerabout.h"

#ifdef __MORPHOS__

#include <mui/Aboutbox_mcc.h>
extern ULONG logo2[];

#endif

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

/// CreateAboutWindow()

#ifdef __MORPHOS__
Object *CreateAboutWindow()
{

    Object *obj = MUI_NewObject(MUIC_Aboutbox,
          	    MUIA_Aboutbox_LogoData, logo2,
		    MUIA_Aboutbox_Credits, "\33b\33l%i\33n Maciej Zukowski",
                    MUIA_Aboutbox_LogoFallbackMode, MAKE_ID('I' , 'E' , 'D' , '\0')   ,
		  /*  WindowContents, MUI_NewObject (MUIC_Group,
                    	MUIA_Group_Horiz, TRUE,
                        MUIA_Group_Child, MUI_NewObject (MUIC_Group,
                   	    MUIA_Group_Child, MUI_NewObject (MUIC_Text,
                   		MUIA_Text_Contents, "\33bProject homapage:",
                   	    TAG_END),
                            MUIA_Group_Child, UrltextObject, MUIA_Urltext_Text,  "Homepage",
                   		MUIA_Urltext_Url,  "http://brain.umcs.lublin.pl/~rzookol", End,
                	    TAG_END),

                            MUIA_Group_Child, MUI_NewObject (MUIC_Rectangle, TAG_END),
                	TAG_END),
	*/
		    MUIA_Window_ID, 0x41424F55,
		    TAG_END);
    return obj;
}
#else

Object *CreateAboutWindow()
{
Object *ok_b;
Object *obj = MUI_NewObject (MUIC_Window,
       		MUIA_Window_Title, "About Xml Viewer",
       		MUIA_Window_ID, MAKE_ID('A','B','T','_'),
       		MUIA_Window_RootObject, MUI_NewObject (MUIC_Group,
         	MUIA_Group_Horiz, FALSE,
          	    MUIA_Group_Child, MUI_NewObject (MUIC_Group,
               		MUIA_Frame, MUIV_Frame_Group,
               	        MUIA_Background, MUII_GroupBack,
               		MUIA_FrameTitle, "About",
               		MUIA_Group_Child, MUI_NewObject (MUIC_Group,
               		    MUIA_Group_Child, MUI_NewObject (MUIC_Group,
                   		MUIA_Group_Child, MUI_NewObject (MUIC_Text,
                                    MUIA_Font, MUIV_Font_Big,
                   		    MUIA_Text_Contents, "\33b\33cXml Viewer",
                   	    	TAG_END),
                                MUIA_Group_Child, MUI_NewObject (MUIC_Text,
                   		    MUIA_Text_Contents, "\33cMichal 'rzookol' Zukowski\n\n",
                                    MUIA_Font,  MUIV_Font_Tiny,
                   	    	TAG_END),
                                MUIA_Group_Child, MUI_NewObject (MUIC_Text,
                   		    MUIA_Text_Contents, "\33cversion 0.9 (Amigaos)\n",
                                    MUIA_Font,  MUIV_Font_Tiny,
                   	    	TAG_END),
      	        	    TAG_END),
          		TAG_END),
                        MUIA_Group_Child, ok_b = MUI_NewObject (MUIC_Text,
                   		MUIA_Frame, MUIV_Frame_Button,
                   		MUIA_Background, MUII_ButtonBack,
                   		MUIA_Font, MUIV_Font_Button,
                   		MUIA_Text_Contents, (long)"\33cOK",
                   		MUIA_InputMode, MUIV_InputMode_RelVerify,
               		TAG_END),
       		    TAG_END),
    		TAG_END),
     		TAG_END);
    DoMethod(ok_b, MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, MUIM_Set, MUIA_Window_Open, FALSE );
    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,  obj,  3, MUIM_Set, MUIA_Window_Open, FALSE );

 return obj;
}
#endif

///
