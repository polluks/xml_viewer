/* xmlviewerexpat.c - XML metafomat viewer
** expat.library interface
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008 Michal Zukowski
*/

#include <proto/expat.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <mui/Listtree_mcc.h>
#include <clib/alib_protos.h>

#include "xmlviewerexpat.h"
#include "api_MUI.h"

/// default_hndl()
// przetwarzanie danych poza nawiasami
void SAVEDS default_hndl(void *userData, const XML_Char *s, int len)
{
    char *buffer;
    struct XMLTree *my_tree = (struct XMLTree  *)userData;
    struct xml_data *tmp;

    if (len>0)
    {
        if ((s[0]!='\n') && (s[0]!=0x20)  && (s[0]!='\t'))
        {
            if ((tmp = (struct xml_data*)AllocVec(sizeof(struct xml_data), MEMF_ANY)))
            {
             	if ((tmp->attr_list = (struct  MinList*)AllocVec( sizeof(struct MinList), MEMF_ANY)))
                {
	             // inicjujemy liste - ale pusta i tak bedzie
                     #ifdef __MORPHOS__
                     NewMinList(tmp->attr_list);
                     #else
                     NewList(tmp->attr_list);
                     #endif

                     if ((buffer=(char *)AllocVec(len+3, MEMF_ANY)))
                     {
	                 strcpy(buffer,"\033b");
	                 CopyMem(s, buffer+2, len);
	                 buffer[len+2]=0;
      	                 //KPrintF("------TXT_%s_%d\n", buffer, len);
                         tmp->type = XML_TEXT;
	                 my_tree->tn[my_tree->depth+1] = (struct MUIS_Listtree_TreeNode *)DoMethod(my_tree->drzewo, MUIM_Listtree_Insert, buffer,
                         			         (APTR)tmp, my_tree->tn[my_tree->depth], MUIV_Listtree_Insert_PrevNode_Tail, 0);
                         FreeVec(buffer);
                     }
                     else  // nic nie dodajemy ale lepiej liste usunac
                         FreeVec(tmp->attr_list);
                }
            	else
                    FreeVec(tmp);
            }
	}
    }
}

///
/// startElement()

void SAVEDS startElement(void *userData, const char *name, const char **atts)
{

    struct XMLTree *my_tree = (struct XMLTree  *)userData;
    struct xml_data *tmp;
    int i=0;
    struct DataNode *dn=NULL;

    if ((tmp = (struct xml_data*)AllocVec(sizeof(struct xml_data), MEMF_ANY)))
    {
    	// inicjujemy liste
    	if ((tmp->attr_list =  (struct  MinList*)AllocVec( sizeof( struct MinList), MEMF_ANY)))
    	{
	    // inicjujemy liste - ale pusta i tak bedzie
	    #ifdef __MORPHOS__
	    NewMinList(tmp->attr_list);
	    #else
	    NewList(tmp->attr_list);
            #endif

    	    //KPrintF("ATRIB %s\n", name);
    	    while(atts[i])
    	    {
            	if ((dn = (struct DataNode *)AllocVec(sizeof(struct DataNode), MEMF_ANY)))
            	{
            	    // alokacja pamiêci dla atrybutów oraz dodanie kolejnych do listy
            	    if((dn->atributes = (char *)AllocVec(strlen(atts[i])+1, MEMF_ANY)))
            	    	strcpy(dn->atributes, atts[i]);

	    	    // alokacja pamiêci dla wartosci oraz dodanie kolejnych do listy
            	    if((dn->values = (char *)AllocVec(strlen(atts[i+1])+1, MEMF_ANY)))
            	    	strcpy(dn->values, atts[i+1]);

            	    AddHead((struct List*)tmp->attr_list, (struct Node*)dn);
            	}
            	//KPrintF("ATRIBVALUEATRIBVALUE %s = %s \n", atts[i], atts[i+1]);
    	    	i+=2;
    	    }
            tmp->type = XML_VALUES;
    	    my_tree->tn[my_tree->depth+1] = (struct MUIS_Listtree_TreeNode *)DoMethod(my_tree->drzewo, MUIM_Listtree_Insert, name,
                                            (APTR)tmp, my_tree->tn[my_tree->depth], MUIV_Listtree_Insert_PrevNode_Tail, TNF_LIST);
            my_tree->depth++;
    	}
        else
            FreeVec(tmp);
    }
}

///
/// endElement()

void SAVEDS endElement(void *userData, const char *name)
{
    struct XMLTree *my_tree = (struct XMLTree  *)userData;
    my_tree->depth--;
}

///
/// comment_hndl()
void SAVEDS comment_hndl(void *userData, const XML_Char *data)
{
    struct XMLTree *my_tree = (struct XMLTree  *)userData;
    char *buffer;
    struct xml_data *tmp;
    int len=strlen(data);
   
    if ((tmp = (struct xml_data*)AllocVec(sizeof(struct xml_data), MEMF_ANY)))
    {
	if ((tmp->attr_list = (struct  MinList*)AllocVec( sizeof(struct MinList), MEMF_ANY)))
	{
	    // inicjujemy liste - ale pusta i tak bedzie
	    #ifdef __MORPHOS__
	    NewMinList(tmp->attr_list);
	    #else
	    NewList(tmp->attr_list);
            #endif

	    if ((buffer=(char *)AllocVec(len+3, MEMF_ANY)))
	    {
		strcpy(buffer,"\033i");
		CopyMem(data, buffer+2, len);
		buffer[len+2]=0;
		tmp->type = XML_COMMENTS;
		my_tree->tn[my_tree->depth+1] = (struct MUIS_Listtree_TreeNode *)DoMethod(my_tree->drzewo, MUIM_Listtree_Insert, buffer,
                         			         				    (APTR)tmp, my_tree->tn[my_tree->depth], MUIV_Listtree_Insert_PrevNode_Tail, 0);
		FreeVec(buffer);
	    }
	    else  // nic nie dodajemy ale lepiej liste usunac
		FreeVec(tmp->attr_list);
	}
	else
	    FreeVec(tmp);
    }

}
///
/// decl_hndl()
// przetwarzanie naglowka xmla
void SAVEDS decl_hndl(void *userData, const XML_Char  *version,
                const XML_Char *encoding, int standalone)
{
    struct XMLTree *my_tree = (struct XMLTree  *)userData;
    struct DataNode *dn=NULL;
    struct xml_data *tmp;

    if ((tmp = (struct xml_data*)AllocVec(sizeof(struct xml_data), MEMF_ANY)))
    {
    	// alokujemy liste
    	if ((tmp->attr_list = (struct  MinList*)AllocVec( sizeof(struct MinList), MEMF_ANY)))
    	{
	    // inicjujemy liste - ale pusta i tak bedzie
	    #ifdef __MORPHOS__
	    NewMinList(tmp->attr_list);
	    #else
	    NewList(tmp->attr_list);
            #endif

    	    if (version)
    	    {
    	    	if ((dn = (struct DataNode *)AllocVec(sizeof(struct DataNode), MEMF_ANY)))
	    	{
            	    if ((dn->atributes = (char *)AllocVec(strlen("version")+1, MEMF_ANY)))
	    	    	strcpy(dn->atributes, "version");

            	    if ((dn->values = (char *)AllocVec(strlen(version)+1, MEMF_ANY)))
	    	    	strcpy(dn->values, version);

            	    AddHead((struct List*)tmp->attr_list, (struct Node*)dn);
    	    	}
    	    }

    	    if (encoding)
    	    {
    	    	if ((dn = (struct DataNode *)AllocVec(sizeof(struct DataNode), MEMF_ANY)))
    	    	{
            	    if ((dn->atributes = (char *)AllocVec(strlen("encoding")+1, MEMF_ANY)))
	    	    	strcpy(dn->atributes, "encoding");

            	    if ((dn->values = (char *)AllocVec(strlen(encoding)+1, MEMF_ANY)))
	    	    	strcpy(dn->values, encoding);

            	    AddHead((struct List*)tmp->attr_list, (struct Node*)dn);
    	    	}
    	    }

            tmp->type = XML_VALUES|XML_HEADER;
    	    my_tree->tn[0] =  (struct MUIS_Listtree_TreeNode *)DoMethod(my_tree->drzewo, MUIM_Listtree_Insert, my_tree->filename, (LONG)tmp,  MUIV_Listtree_Insert_ListNode_Root, MUIV_Listtree_Insert_PrevNode_Tail, TNF_LIST);
	}
        else
            FreeVec(tmp);
    }
}

///
