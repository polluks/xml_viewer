/* xmlviewertree.c - XML metafomat viewer
** listree hooks & subclass
**
** Part of Plot.mcc MUI custom class package.
** (c) 2008-2009 Michal Zukowski
*/

#include <stdio.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <mui/Listtree_mcc.h>
#include <clib/alib_protos.h>
#include <datatypes/textclass.h>

#include "xmlviewertree.h"
#include "xmlviewerexpat.h"
#include "xmlviewerlist.h"
#include "xmlviewerlocale.h"
#include "api_MUI.h"
#include "resources.h"

#define MODE_SAVE 1
#define MODE_LOAD 2
#define MODE_SAVEFORMATTED 3
#define MODE_SAVECLIPBOARD 4

#define MENU_CUT    1
#define MENU_COPY   2
#define MENU_PASTE  3

extern Object *list;
extern XML_Parser parser;
extern struct XMLTree m_tree;
extern struct Catalog *Cat;
char buffer [1024]; 

struct Hook des_hook;
struct Hook con_hook;

/// xmlviewertree_Data

struct xmlviewertree_Data
{
    int data;
    Object *menu;
    struct MUIS_Listtree_TreeNode *tn;
    struct IFFHandle *iffh;
    struct MUI_EventHandlerNode EHNode;

};

///
/// xmlviewertreeDispatcher

DISPATCHER_DEF(xmlviewertree)

///
/// CreatexmlviewertreeClass()

struct MUI_CustomClass *CreatexmlviewertreeClass (void)
{
    struct MUI_CustomClass *cl;

    cl = MUI_CreateCustomClass(NULL, MUIC_Listtree, NULL, sizeof(struct xmlviewertree_Data), (APTR)DISPATCHER_REF(xmlviewertree));
    return cl;
}

///
/// active_hook

M_HOOK(active, APTR obj, APTR dana)
{
    struct MUIS_Listtree_TreeNode *active;
    struct Data tmp;
    struct DataNode *dn, *d2;
    struct MinList *tmp_list;
    struct xml_data *tmp_xml;

    // clear attribute list object
    DoMethod(list, MUIM_List_Clear);

    // get pointer to the clicked tree list object
    if((active = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry, NULL, MUIV_Listtree_GetEntry_Position_Active, 0)))
    {
        if((tmp_xml = (struct xml_data*)(active->tn_User)))
        {
            if(tmp_xml->type & XML_VALUES)
            {
                // fetch the attribute list
        	if((tmp_list = tmp_xml->attr_list))
        	{
            	    //DBG KPrintF("Dupa %x\n", tmp_list);
            	    DoMethod(list, MUIM_Set, MUIA_List_Quiet, TRUE);

	    	    for (dn = (struct DataNode*)tmp_list->mlh_TailPred;  d2 = (struct DataNode*)dn->Node.mln_Pred; dn=d2)
	    	    {
            	    	strcpy(tmp.name, dn->atributes);
		    	strcpy(tmp.value, dn->values);
              	    	//KPrintF("ddd**ddd** %s\n", dn->atributes);
  		    	DoMethod(list, MUIM_List_InsertSingle, &tmp, MUIV_List_Insert_Bottom);
            	    }
    	    	    DoMethod(list, MUIM_Set, MUIA_List_Quiet, FALSE);
            	}
    	    }
    	}
    }

    return(0);
}

///
/// con_hook

M_HOOK(con, APTR obj, APTR dana)
{
    struct xml_data *msg =  (struct xml_data*)dana;
    return( (LONG)msg );
}

///
/// des_hook

M_HOOK(des, APTR obj, APTR dana)
{
    struct xml_data *msg =  (struct xml_data*)dana;

    if (msg)
    {
        if (msg->attr_list)
        {
            struct DataNode *dn;

            while ((dn = (struct DataNode*)RemHead(msg->attr_list)))
	    {
            	if (dn->values)
            	    FreeVec(dn->values);

            	if (dn->atributes)
            	    FreeVec(dn->atributes);

	    	FreeVec(dn);
            }
            FreeVec(msg->attr_list);
	}

        FreeVec(msg);
	msg = NULL;
    }

    return 0;
}

///
/// xmlviewertree_New()

ULONG xmlviewertree_New(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct xmlviewertree_Data *data;
    int rc;

    obj= DoSuperNew(cl, obj,
	MUIA_Frame,                    MUIV_Frame_ReadList,
	MUIA_CycleChain,               TRUE,
	MUIA_Listtree_ConstructHook,   (APTR) &con_hook,
	MUIA_Listtree_DestructHook,    (APTR) &des_hook,
	MUIA_Listtree_EmptyNodes,      TRUE,
	MUIA_Listtree_MultiSelect,     MUIV_Listview_MultiSelect_Default,
        MUIA_ContextMenu,              1, // potrzebne do MUIM_ContextMenuBuild
    TAG_DONE);

    data = INST_DATA(cl, obj);
    data->menu = NULL;

    // clipboard handling
    rc = FALSE;
    if ((data->iffh = AllocIFF()))
    {
	if ((data->iffh->iff_Stream = (IPTR)OpenClipboard(0)))
	{
	    InitIFFasClip(data->iffh);
	    rc = TRUE;
	}
       
    	   
    }

    return (ULONG)obj;
}

///
/// xmlviewertree_MenuBuild()

ULONG xmlviewertree_MenuBuild(struct IClass *cl, Object *obj, struct MUIP_ContextMenuBuild *msg)
{
    struct MUIS_Listtree_TestPos_Result res;
    struct MUIS_Listtree_TreeNode *tn;
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);

    // dispose the old context_menu if it still exists
    if(data->menu)
    {
    	MUI_DisposeObject(data->menu);
    	data->menu = NULL;
    }

    // w zaleznosci nad jaka pozycja listy kliknelismy budujemy odpowiednie menu contextowe
    if (DoMethod(obj, MUIM_Listtree_TestPos, msg->mx, msg->my, &res))
    {
        tn =  res.tpr_TreeNode;
        if (tn)
        {
    	    //KPrintF("%s", tn->tn_Name);
            //KPrintF("x: %d, y: %d\n", msg->mx, msg->my);
            data->menu = MenustripObject,
                Child, MenuObjectT( GetCatalogStr(Cat, MSG_MENU_EDIT, "Edit") ),
                        Child, MenuitemObject,
                            MUIA_Menuitem_Title, GetCatalogStr(Cat, MSG_MENU_CUT, "Cut"),   MUIA_UserData, MENU_CUT,
                            //MUIA_Menuitem_Shortcut, (ULONG)"x",
                        End,
                        Child, MenuitemObject,
                            MUIA_Menuitem_Title, GetCatalogStr(Cat, MSG_MENU_COPY, "Copy"),  MUIA_UserData, MENU_COPY,
                            //MUIA_Menuitem_Shortcut, (ULONG)"c",
                        End,
                        Child, MenuitemObject,
                            MUIA_Menuitem_Title, GetCatalogStr(Cat, MSG_MENU_PASTE, "Paste"), MUIA_UserData, MENU_PASTE,
                            //MUIA_Menuitem_Shortcut, (ULONG)"v",
                       	End,
                End,
            End;
            data->tn = tn;

            return (ULONG)data->menu;
        }
    }
   
    return FALSE;
}

///
/// xmlviewertree_Dispose()

ULONG xmlviewertree_Dispose(struct IClass *cl, Object *obj, Msg msg)
{
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);

    // remove any existing dynamic menu when disposing
    if(data->menu)
    {
    	MUI_DisposeObject(data->menu);
    	data->menu = NULL;
    }

    // finish clipboard handling
    if (data->iffh)
    {
	if(data->iffh->iff_Stream)
	    CloseClipboard((struct ClipboardHandle *)data->iffh->iff_Stream);

	FreeIFF(data->iffh);
    }

    return(DoSuperMethodA(cl,obj,msg));
}

///
/// cutcopypaste()

LONG cutcopypaste(struct IClass *cl, Object *obj, int item)
{
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);
    struct DataNode *dn;
    struct MinList *tmp_list;
    int rc, len;
    struct xml_data *tmp;

    // albo kopiowanie albo wycinanie
    if ( (item == MENU_CUT) || (item == MENU_COPY) )
    {
	if (OpenIFF(data->iffh, IFFF_WRITE) == 0)
	{
	    if (PushChunk(data->iffh, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN) == 0)
	    {
		if (PushChunk(data->iffh, 0, ID_CHRS, IFFSIZE_UNKNOWN) == 0)
		{
                    if(data->tn->tn_Flags & TNF_LIST)
                    {
                        // for all subtrees except the root
                        if ((rc = DoMethod(obj, MUIM_Listtree_GetNr, data->tn, 0)))
                        {
                    	    len = snprintf(buffer, 1024, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><%s", data->tn->tn_Name);
                            WriteChunkBytes(data->iffh, buffer, len);
                        }
                        else  // move entire XML tree including the root to the buffer
			    WriteChunkBytes(data->iffh, "<?xml", strlen("<?xml"));
                     
                        if ((tmp = (struct xml_data*)(data->tn->tn_User)))
                        {
			    if((tmp_list = tmp->attr_list))
                            {
                            	for (dn = (struct DataNode*)tmp_list->mlh_TailPred;  dn->Node.mln_Pred;  dn = (struct DataNode*)dn->Node.mln_Pred)
                            	{
                   	            if(dn)
                   	            {
                   	            	len = snprintf(buffer, 1024, " %s=\"%s\"", dn->atributes, dn->values);
                   	            	WriteChunkBytes(data->iffh, buffer, len);
                                    }
                   	            else
                   	            	break;
                            	}
                            }
                    	}
                        if (rc)
                            WriteChunkBytes(data->iffh, ">", strlen(">"));
                        else
                            WriteChunkBytes(data->iffh, "?>", strlen("?>"));

                        DoMethod(obj, MUIM_LTree_Save, data->iffh, data->tn, MODE_SAVECLIPBOARD);

                    if(rc) // for every node except the tree root
			{
			    len = snprintf(buffer, 1024, "\n</%s>", data->tn->tn_Name);
			    WriteChunkBytes(data->iffh, buffer, len);
			}

                    }
                    else // for text outside tags copy normally without dumping full XML
                    {
                        char *bez_escapekodu;
                        bez_escapekodu = data->tn->tn_Name;
                        bez_escapekodu += 2;
                        WriteChunkBytes(data->iffh, bez_escapekodu, strlen(bez_escapekodu));
                    }
		    PopChunk(data->iffh);
		}
		else
		    PopChunk(data->iffh);
	    }
	    CloseIFF(data->iffh);
	}

        if (item == MENU_CUT)
        {
	    DoMethod(obj, MUIM_Set, MUIA_Listtree_Quiet, TRUE);
            DoMethod(obj, MUIM_Listtree_Remove, 0, data->tn, 0);
            DoMethod(obj, MUIM_Set, MUIA_Listtree_Quiet, FALSE);
    	}
    }
    else // item == 3 // czyli MENU_PASTE
    {
    	DoMethod(obj, MUIM_LTree_Paste, NULL, data->tn, NULL);
    }
}

///
/// xmlviewertree_MenuChoice()

LONG  xmlviewertree_MenuChoice(struct IClass *cl, Object *obj, struct MUIP_ContextMenuChoice *msg)
{
    int item;
 
    get(msg->item,  MUIA_UserData, &item);
    cutcopypaste(cl, obj, item);

    return TRUE;
}

///
/// xmlviewertree_Paste()

LONG  xmlviewertree_Paste(struct IClass *cl, Object *obj, struct MUIP_LTreeFile* msg)
{
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);
    struct MUIS_Listtree_TreeNode *list =  (struct MUIS_Listtree_TreeNode *)msg->node;
    int done;
    LONG textlen;
    STRPTR textbuf=0;
    char error_buf[100];
 
//    XML_SetXmlDeclHandler(parser, decl_hndl);

    // open the file handle
    if (OpenIFF(data->iffh, IFFF_READ) == 0)
    {
        if (StopChunk(data->iffh, ID_FTXT, ID_CHRS) == 0)
        {
            XML_ParserReset(parser, 0);  // reset the parser
                m_tree.tree = obj;       // listtree object
                m_tree.tn[0]= list;        // sublist that becomes the attachment point
                m_tree.depth = 0;          // start from the beginning
                XML_SetUserData(parser, &m_tree);                      // data for the parser
    	    XML_SetElementHandler(parser, startElement, endElement);
    	    XML_SetCharacterDataHandler(parser,  default_hndl);

	    while (ParseIFF(data->iffh, IFFPARSE_SCAN) == 0)
	    {   
		struct ContextNode *cn;
              
		if ((cn = CurrentChunk(data->iffh)))
		{   
		    LONG size = cn->cn_Size;
                  
		    if (cn->cn_Type == ID_FTXT && size > 0)
		    {   
			if (cn->cn_ID == ID_CHRS && !textbuf)
			{  
			    if((textbuf = AllocVec(size + 1, MEMF_ANY)))
			    {
				if ((textlen = ReadChunkBytes(data->iffh, textbuf, size)))
				    textbuf[textlen] = '\0';

                        	if (XML_Parse(parser, textbuf, textlen, 1) == XML_STATUS_ERROR)
				{
	     			    snprintf(error_buf, 100,  "Error:\n\n%s at line %d",
	    			    XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
                                    MUI_RequestA(_app(obj), _window(obj), 0, "XML Viewer Error", "*OK", error_buf, NULL);
				}

			    	FreeVec(textbuf);
			    }
			}
                    }
                }
            }
        }
    	CloseIFF(data->iffh);
        return TRUE;
    }

    return FALSE;
}

///
/// xmlviewertree_Save()

LONG  xmlviewertree_Save(struct IClass *cl, Object *obj, struct MUIP_LTreeFile* msg)
{

    FILE *fp = (FILE *)msg->file;
    int mode = msg->mode;
    struct MUIS_Listtree_TreeNode *tn, *list =  (struct MUIS_Listtree_TreeNode *)msg->node;
    UWORD pos=0, i, len;
    static int poziom=0;
    static int czy_wart=0;
    struct DataNode *dn;
    struct MinList *tmp_list;
    struct xml_data *tmp;
    char buffer [1024];
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);

    for(pos=0; ; pos++)
    {
	if((tn = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry, list, pos, MUIV_Listtree_GetEntry_Flags_SameLevel)))
	{

            if ((tmp = (struct xml_data*)(tn->tn_User)))
            {
	    	if(tmp->type & XML_VALUES)
	    	{
            	    if ( (poziom == 0) && (mode!=MODE_SAVECLIPBOARD) )
                    {
                        Write(fp, "<?xml", 5);
                   	if((tmp_list = tmp->attr_list))
    		    	{
			    for (dn = (struct DataNode*)tmp_list->mlh_TailPred;  dn->Node.mln_Pred;  dn = (struct DataNode*)dn->Node.mln_Pred)
			    {
			    	if(dn)
                                {
                                    len = snprintf(buffer, 1024, " %s=\"%s\"", dn->atributes, dn->values);
                                    Write(fp, buffer, len);
                                }
                            	else
                            	    break;
    			    }
 		    	}
                        Write(fp, "?>", 2);
                    	poziom++;
                    	DoMethod(obj, MUIM_LTree_Save, fp, tn, mode);
                    	poziom--;
                    }
                    else
                    {
                    	if( mode == MODE_SAVEFORMATTED )
                    	{
                    	    for (i=1; i<poziom; i++)
                        	Write(fp, "\t", 1);
                    	}

   			len = snprintf(buffer, 1024, "\n<%s", tn->tn_Name);
                    	if (mode == MODE_SAVECLIPBOARD)
			    WriteChunkBytes(data->iffh, buffer, len);
                    	else
                            Write(fp, buffer, len);

                    	if((tmp_list = tmp->attr_list))
    		    	{
                  	    for (dn = (struct DataNode*)tmp_list->mlh_TailPred;  dn->Node.mln_Pred;  dn = (struct DataNode*)dn->Node.mln_Pred)
			    {
    			    	if(dn)
    			    	{
                                    len = snprintf(buffer, 1024, " %s=\"%s\"", dn->atributes, dn->values);
                                    if (mode == MODE_SAVECLIPBOARD)
                                    	WriteChunkBytes(data->iffh, buffer, len);
                            	    else
                                        Write(fp, buffer, len);
    			    	}
    			    	else
    	    			    break;
                            }
 		    	}

                    	if (mode == MODE_SAVECLIPBOARD)
                    	    WriteChunkBytes(data->iffh, ">", 1);
                    	else
                            Write(fp, ">", 1);

                    	poziom++;
                    	DoMethod(obj, MUIM_LTree_Save, fp, tn, mode);
                    	poziom--;

                    	if( mode == MODE_SAVEFORMATTED )
                    	{
                    	    for (i=1; i<poziom; i++)
			   	Write(fp, "\t", 1);
                    	}

                    	if (czy_wart)
                    	{
                            len = snprintf(buffer, 1024, "</%s>", tn->tn_Name);
                       	    czy_wart = 0;
                    	}
                    	else
                            len = snprintf(buffer, 1024, "\n</%s>", tn->tn_Name);

                        if (mode == MODE_SAVECLIPBOARD)
			    WriteChunkBytes(data->iffh, buffer, len);
                        else
			    Write(fp, buffer, len);


		    }
            	}
	    	else if (tmp->type & XML_TEXT)
	    	{
		    char *bez_escapekodu;
		    bez_escapekodu = tn->tn_Name;
		    bez_escapekodu += 2;
                    if( mode == MODE_SAVEFORMATTED )
                    {
                    	for (i=0; i<poziom; i++)
                            Write(fp, "\t", 1);
               	    }
        	    if (mode == MODE_SAVECLIPBOARD)
		    {
		    	WriteChunkBytes(data->iffh, bez_escapekodu, strlen(bez_escapekodu));
		    }
		    else
                        Write(fp, bez_escapekodu, strlen(bez_escapekodu));

                    czy_wart = 1;
	    	}
	    }
    	}
        else
	    break;
    }
    // TODO: proper return value
    return TRUE;
}

///
/// xmlviewertree_HEvent()

long xmlviewertree_HEvent (Class *cl, Object *obj, struct MUIP_HandleEvent *msg)
{
    struct MUIS_Listtree_TreeNode *treenode;
    struct xmlviewertree_Data *data = INST_DATA(cl, obj);

    	if(msg->muikey == MUIKEY_COPY)
    	{
            treenode = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry,  MUIV_Listtree_GetEntry_ListNode_Root, MUIV_Listtree_GetEntry_Position_Active, 0);
            data->tn = treenode;
            cutcopypaste(cl, obj, MENU_COPY);
    	}
	else if (msg->muikey == MUIKEY_CUT)
    	{
       	    treenode = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry,  MUIV_Listtree_GetEntry_ListNode_Root, MUIV_Listtree_GetEntry_Position_Active, 0);
            data->tn = treenode;
       	    cutcopypaste(cl, obj, MENU_CUT);
    	}
	else if (msg->muikey == MUIKEY_PASTE)
    	{
       	    treenode = (struct MUIS_Listtree_TreeNode *)DoMethod(obj, MUIM_Listtree_GetEntry,  MUIV_Listtree_GetEntry_ListNode_Root, MUIV_Listtree_GetEntry_Position_Active, 0);
       	    data->tn = treenode;
       	    cutcopypaste(cl, obj, MENU_PASTE);
    	}
        DoSuperMethodA (cl, obj, msg);
    	return FALSE;
}

///
/// xmlviewertree_Setup()

long xmlviewertree_Setup (Class *cl, Object *obj, Msg msg)
{
    struct xmlviewertree_Data *data = INST_DATA(cl,obj);

    if (DoSuperMethodA (cl, obj, msg))
    {
    	data->EHNode.ehn_Priority = 0;
    	data->EHNode.ehn_Flags = 0;
    	data->EHNode.ehn_Object = obj;
    	data->EHNode.ehn_Class = cl;
    	data->EHNode.ehn_Events = IDCMP_RAWKEY;
    	DoMethod (_win(obj), MUIM_Window_AddEventHandler, &data->EHNode);

	return TRUE;
    }
    return FALSE;
}

///
/// xmlviewertree_Cleanup()

long xmlviewertree_Cleanup (Class *cl, Object *obj, Msg msg)
{
    struct xmlviewertree_Data *data = INST_DATA(cl,obj);

    DoMethod (_win(obj), MUIM_Window_RemEventHandler, &data->EHNode);
    return TRUE;
}

///
/// xmlviewertreeDispatcher()

DISPATCHER(xmlviewertree)
	switch (msg->MethodID)
	{
	    case OM_NEW:		 return (xmlviewertree_New        (cl, obj, (struct opSet*)msg));
            case MUIM_ContextMenuBuild:  return (xmlviewertree_MenuBuild  (cl, obj, (struct MUIP_ContextMenuBuild *)msg));
	    case MUIM_LTree_Save:        return (xmlviewertree_Save       (cl, obj, (struct MUIP_LTreeFile*)msg));
            case MUIM_LTree_Paste:       return (xmlviewertree_Paste      (cl, obj, (struct MUIP_LTreeFile*)msg));
            case MUIM_ContextMenuChoice: return (xmlviewertree_MenuChoice (cl, obj, (struct MUIP_ContextMenuChoice*)msg));
            case MUIM_HandleEvent:	 return (xmlviewertree_HEvent     (cl, obj, (struct MUIP_HandleEvent *)msg));
            case MUIM_Setup:        	 return (xmlviewertree_Setup   	  (cl, obj, msg));
            case MUIM_Cleanup:      	 return (xmlviewertree_Cleanup 	  (cl, obj, msg));
            default:  			 return (DoSuperMethodA	          (cl, obj, msg));

        }
DISPATCHER_END

///
