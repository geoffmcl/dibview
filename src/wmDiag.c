// NOTE: Latest BETTER implementation in EWM project!!! Sep 2000
// =========================================================
//
// Copyright (c) 1997  Geoff R. McLane
//
// Module Name:
//		WMDiag.c
//
// Abstract:
//		This is for DIAGNOSTIC windows message information
//		ONLY and should be EXCLUDED from a RELEASE.
//
// Environment:
//		Application mode in Windows 9* ++
//		Application mode in Windows NT 4.0++
//
// History:
//		September, 1997 - Created.
//		September, 1998 - Full interface enabled my DIAGWM flag
//		November,  1999 - Added NAME registration to replace
//			HEX (H=0xcee5) with a USER name, like Edit, Graf, ...
//
// Author:
//		Geoff R. McLane (Email: Geoff_McLane@Compuserve.Com
//
// =========================================================
#include	<windows.h>
#include	<commctrl.h>
#include	"WMDiag.h"
#include	"WMDiag99.h"

//#ifndef	NDEBUG
#if		( defined( SHOWMSG ) || defined( SHOWWM ) || defined( INCLWMCODE ) )
// **************************************************************
// SEE RDi$Cfg.h for
//	SHOWMSG		- Show the HOOK information.
//	SHOWWM		- Ouput ALL messages to DIALOG
#pragma message( "ADVICE: Diagnostic Window messages ENABLED!" )

//#ifdef	BGLVIEW2
//extern	LPSTR GetszTmpBuf2( void );
//#endif	// BGLVIEW2

//typedef	LPSTR (*MSGDEC) (HWND, UINT, WPARAM, LPARAM, LPSTR );
#ifndef WMSTR
typedef struct {
	UINT	val;
	LPSTR	lps;
}WMSTR;
typedef WMSTR FAR * LPWMSTR;
#endif 
#ifndef WMSTR
typedef	LPSTR (*MSGDEC) (HWND, UINT, WPARAM, LPARAM, LPSTR );
typedef struct {
	UINT	val;
	LPSTR	lps;
	DWORD	flg;
	MSGDEC	dec;
}WMSTR2;

LPSTR	Generic( HWND, UINT, WPARAM, LPARAM, LPSTR );
LPSTR	GNotify( HWND, UINT, WPARAM, LPARAM, LPSTR );
#endif // ifndef WMSTR

static char	szGStg1[1024];
static char	szGStg2[1024];

WMSTR WmStr[] = {
 {WM_NULL                         ,"WM_NULL 0x0000 "},
 {WM_CREATE                       ,"WM_CREATE 0x0001 "},
 {WM_DESTROY                      ,"WM_DESTROY 0x0002 "},
 {WM_MOVE                         ,"WM_MOVE 0x0003 "},
 {WM_SIZE                         ,"WM_SIZE 0x0005 "},
 {WM_ACTIVATE                     ,"WM_ACTIVATE 0x0006 "},
 {WM_SETFOCUS                     ,"WM_SETFOCUS 0x0007 "},
 {WM_KILLFOCUS                    ,"WM_KILLFOCUS 0x0008 "},
 {WM_ENABLE                       ,"WM_ENABLE 0x000A "},
 {WM_SETREDRAW                    ,"WM_SETREDRAW 0x000B "},
 {WM_SETTEXT                      ,"WM_SETTEXT 0x000C "},
 {WM_GETTEXT                      ,"WM_GETTEXT 0x000D "},
 {WM_GETTEXTLENGTH                ,"WM_GETTEXTLENGTH 0x000E "},
 {WM_PAINT                        ,"WM_PAINT 0x000F "},
 {WM_CLOSE                        ,"WM_CLOSE 0x0010 "},
 {WM_QUERYENDSESSION              ,"WM_QUERYENDSESSION 0x0011 "},
 {WM_QUIT                         ,"WM_QUIT 0x0012 "},
 {WM_QUERYOPEN                    ,"WM_QUERYOPEN 0x0013 "},
 {WM_ERASEBKGND                   ,"WM_ERASEBKGND 0x0014 "},
 {WM_SYSCOLORCHANGE               ,"WM_SYSCOLORCHANGE 0x0015 "},
 {WM_ENDSESSION                   ,"WM_ENDSESSION 0x0016 "},
 {WM_SHOWWINDOW                   ,"WM_SHOWWINDOW 0x0018 "},
 {WM_WININICHANGE                 ,"WM_WININICHANGE 0x001A "},
 {WM_DEVMODECHANGE                ,"WM_DEVMODECHANGE 0x001B "},
 {WM_ACTIVATEAPP                  ,"WM_ACTIVATEAPP 0x001C "},
 {WM_FONTCHANGE                   ,"WM_FONTCHANGE 0x001D "},
 {WM_TIMECHANGE                   ,"WM_TIMECHANGE 0x001E "},
 {WM_CANCELMODE                   ,"WM_CANCELMODE 0x001F "},
 {WM_SETCURSOR                    ,"WM_SETCURSOR 0x0020 "},
 {WM_MOUSEACTIVATE                ,"WM_MOUSEACTIVATE 0x0021 "},
 {WM_CHILDACTIVATE                ,"WM_CHILDACTIVATE 0x0022 "},
 {WM_QUEUESYNC                    ,"WM_QUEUESYNC 0x0023 "},
 {WM_GETMINMAXINFO                ,"WM_GETMINMAXINFO 0x0024 "},
 {WM_PAINTICON                    ,"WM_PAINTICON 0x0026 "},
 {WM_ICONERASEBKGND               ,"WM_ICONERASEBKGND 0x0027 "},
 {WM_NEXTDLGCTL                   ,"WM_NEXTDLGCTL 0x0028 "},
 {WM_SPOOLERSTATUS                ,"WM_SPOOLERSTATUS 0x002A "},
 {WM_DRAWITEM                     ,"WM_DRAWITEM 0x002B "},
 {WM_MEASUREITEM                  ,"WM_MEASUREITEM 0x002C "},
 {WM_DELETEITEM                   ,"WM_DELETEITEM 0x002D "},
 {WM_VKEYTOITEM                   ,"WM_VKEYTOITEM 0x002E "},
 {WM_CHARTOITEM                   ,"WM_CHARTOITEM 0x002F "},
 {WM_SETFONT                      ,"WM_SETFONT 0x0030 "},
 {WM_GETFONT                      ,"WM_GETFONT 0x0031 "},
 {WM_SETHOTKEY                    ,"WM_SETHOTKEY 0x0032 "},
 {WM_GETHOTKEY                    ,"WM_GETHOTKEY 0x0033 "},
 {WM_QUERYDRAGICON                ,"WM_QUERYDRAGICON 0x0037 "},
 {WM_COMPAREITEM                  ,"WM_COMPAREITEM 0x0039 "},
 {WM_COMPACTING                   ,"WM_COMPACTING 0x0041 "},
 {WM_WINDOWPOSCHANGING            ,"WM_WINDOWPOSCHANGING 0x0046 "},
 {WM_WINDOWPOSCHANGED             ,"WM_WINDOWPOSCHANGED 0x0047 "},
 {WM_POWER                        ,"WM_POWER 0x0048 "},
 {WM_COPYDATA                     ,"WM_COPYDATA 0x004A "},
 {WM_CANCELJOURNAL                ,"WM_CANCELJOURNAL 0x004B "},
 {WM_NOTIFY                       ,"WM_NOTIFY 0x004E "},
 {WM_INPUTLANGCHANGEREQUEST       ,"WM_INPUTLANGCHANGEREQUEST 0x0050 "},
 {WM_INPUTLANGCHANGE              ,"WM_INPUTLANGCHANGE 0x0051 "},
 {WM_TCARD                        ,"WM_TCARD 0x0052 "},
 {WM_HELP                         ,"WM_HELP 0x0053 "},
 {WM_USERCHANGED                  ,"WM_USERCHANGED 0x0054 "},
 {WM_NOTIFYFORMAT                 ,"WM_NOTIFYFORMAT 0x0055 "},
 {WM_CONTEXTMENU                  ,"WM_CONTEXTMENU 0x007B "},
 {WM_STYLECHANGING                ,"WM_STYLECHANGING 0x007C "},
 {WM_STYLECHANGED                 ,"WM_STYLECHANGED 0x007D "},
 {WM_DISPLAYCHANGE                ,"WM_DISPLAYCHANGE 0x007E "},
 {WM_GETICON                      ,"WM_GETICON 0x007F "},
 {WM_SETICON                      ,"WM_SETICON 0x0080 "},
 {WM_NCCREATE                     ,"WM_NCCREATE 0x0081 "},
 {WM_NCDESTROY                    ,"WM_NCDESTROY 0x0082 "},
 {WM_NCCALCSIZE                   ,"WM_NCCALCSIZE 0x0083 "},
 {WM_NCHITTEST                    ,"WM_NCHITTEST 0x0084 "},
 {WM_NCPAINT                      ,"WM_NCPAINT 0x0085 "},
 {WM_NCACTIVATE                   ,"WM_NCACTIVATE 0x0086 "},
 {WM_GETDLGCODE                   ,"WM_GETDLGCODE 0x0087 "},
 {WM_NCMOUSEMOVE                  ,"WM_NCMOUSEMOVE 0x00A0 "},
 {WM_NCLBUTTONDOWN                ,"WM_NCLBUTTONDOWN 0x00A1 "},
 {WM_NCLBUTTONUP                  ,"WM_NCLBUTTONUP 0x00A2 "},
 {WM_NCLBUTTONDBLCLK              ,"WM_NCLBUTTONDBLCLK 0x00A3 "},
 {WM_NCRBUTTONDOWN                ,"WM_NCRBUTTONDOWN 0x00A4 "},
 {WM_NCRBUTTONUP                  ,"WM_NCRBUTTONUP 0x00A5 "},
 {WM_NCRBUTTONDBLCLK              ,"WM_NCRBUTTONDBLCLK 0x00A6 "},
 {WM_NCMBUTTONDOWN                ,"WM_NCMBUTTONDOWN 0x00A7 "},
 {WM_NCMBUTTONUP                  ,"WM_NCMBUTTONUP 0x00A8 "},
 {WM_NCMBUTTONDBLCLK              ,"WM_NCMBUTTONDBLCLK 0x00A9 "},
 {WM_KEYFIRST                     ,"WM_KEYFIRST 0x0100 "},
 {WM_KEYDOWN                      ,"WM_KEYDOWN 0x0100 "},
 {WM_KEYUP                        ,"WM_KEYUP 0x0101 "},
 {WM_CHAR                         ,"WM_CHAR 0x0102 "},
 {WM_DEADCHAR                     ,"WM_DEADCHAR 0x0103 "},
 {WM_SYSKEYDOWN                   ,"WM_SYSKEYDOWN 0x0104 "},
 {WM_SYSKEYUP                     ,"WM_SYSKEYUP 0x0105 "},
 {WM_SYSCHAR                      ,"WM_SYSCHAR 0x0106 "},
 {WM_SYSDEADCHAR                  ,"WM_SYSDEADCHAR 0x0107 "},
 {WM_KEYLAST                      ,"WM_KEYLAST 0x0108 "},
 {WM_IME_STARTCOMPOSITION         ,"WM_IME_STARTCOMPOSITION 0x010D "},
 {WM_IME_ENDCOMPOSITION           ,"WM_IME_ENDCOMPOSITION 0x010E "},
 {WM_IME_COMPOSITION              ,"WM_IME_COMPOSITION 0x010F "},
 {WM_IME_KEYLAST                  ,"WM_IME_KEYLAST 0x010F "},
 {WM_INITDIALOG                   ,"WM_INITDIALOG 0x0110 "},
 {WM_COMMAND                      ,"WM_COMMAND 0x0111 "},
 {WM_SYSCOMMAND                   ,"WM_SYSCOMMAND 0x0112 "},
 {WM_TIMER                        ,"WM_TIMER 0x0113 "},
 {WM_HSCROLL                      ,"WM_HSCROLL 0x0114 "},
 {WM_VSCROLL                      ,"WM_VSCROLL 0x0115 "},
 {WM_INITMENU                     ,"WM_INITMENU 0x0116 "},
 {WM_INITMENUPOPUP                ,"WM_INITMENUPOPUP 0x0117 "},
 {WM_MENUSELECT                   ,"WM_MENUSELECT 0x011F "},
 {WM_MENUCHAR                     ,"WM_MENUCHAR 0x0120 "},
 {WM_ENTERIDLE                    ,"WM_ENTERIDLE 0x0121 "},
// #ifndef _WIN32_WCE
// #if(WINVER >= 0x0500)
 { 0x0122                         ,"WM_MENURBUTTONUP 0x0122 "},
 { 0x0123                         ,"WM_MENUDRAG 0x0123 " },
 { 0x0124                         ,"WM_MENUGETOBJECT 0x0124 " },
 { 0x0125                         ,"WM_UNINITMENUPOPUP 0x0125 " },
 { 0x0126                         ,"WM_MENUCOMMAND 0x0126 " },
// #if(_WIN32_WINNT >= 0x0500)
 { 0x0127                         ,"WM_CHANGEUISTATE 0x0127 " },
 { 0x0128                         ,"WM_UPDATEUISTATE 0x0128 " },
 { 0x0129                         ,"WM_QUERYUISTATE 0x0129 " },
// #endif /* _WIN32_WINNT >= 0x0500 */
// #endif /* WINVER >= 0x0500 */
// #endif /* #ifndef _WIN32_WCE */
 {WM_CTLCOLORMSGBOX               ,"WM_CTLCOLORMSGBOX 0x0132 "},
 {WM_CTLCOLOREDIT                 ,"WM_CTLCOLOREDIT 0x0133 "},
 {WM_CTLCOLORLISTBOX              ,"WM_CTLCOLORLISTBOX 0x0134 "},
 {WM_CTLCOLORBTN                  ,"WM_CTLCOLORBTN 0x0135 "},
 {WM_CTLCOLORDLG                  ,"WM_CTLCOLORDLG 0x0136 "},
 {WM_CTLCOLORSCROLLBAR            ,"WM_CTLCOLORSCROLLBAR 0x0137 "},
 {WM_CTLCOLORSTATIC               ,"WM_CTLCOLORSTATIC 0x0138 "},
// {WM_MOUSEFIRST                   ,"WM_MOUSEFIRST 0x0200 "},
 {WM_MOUSEMOVE                    ,"WM_MOUSEMOVE 0x0200 "},
 {WM_LBUTTONDOWN                  ,"WM_LBUTTONDOWN 0x0201 "},
 {WM_LBUTTONUP                    ,"WM_LBUTTONUP 0x0202 "},
 {WM_LBUTTONDBLCLK                ,"WM_LBUTTONDBLCLK 0x0203 "},
 {WM_RBUTTONDOWN                  ,"WM_RBUTTONDOWN 0x0204 "},
 {WM_RBUTTONUP                    ,"WM_RBUTTONUP 0x0205 "},
 {WM_RBUTTONDBLCLK                ,"WM_RBUTTONDBLCLK 0x0206 "},
 {WM_MBUTTONDOWN                  ,"WM_MBUTTONDOWN 0x0207 "},
 {WM_MBUTTONUP                    ,"WM_MBUTTONUP 0x0208 "},
 {WM_MBUTTONDBLCLK                ,"WM_MBUTTONDBLCLK 0x0209 "},
// {WM_MOUSEWHEEL                   ,"WM_MOUSEWHEEL 0x020A "},
 { 0x020a                         ,"WM_MOUSEWHEEL 0x020A "},
// #if (_WIN32_WINNT >= 0x0500)
 { 0x020b                         ,"WM_XBUTTONDOWN 0x020B "},
 { 0x020c                         ,"WM_XBUTTONUP 0x020C "},
 { 0x020d                         ,"WM_XBUTTONDBLCLK 0x020D "},
// #endif
// #if (_WIN32_WINNT >= 0x0500)
// #define WM_MOUSELAST                    0x020D
// {WM_MOUSELAST                    ,"WM_MOUSELAST 0x0209 "},
// {WM_MOUSELAST                    ,"WM_MOUSELAST 0x020A "},
 {WM_PARENTNOTIFY                 ,"WM_PARENTNOTIFY 0x0210 "},
 {WM_ENTERMENULOOP                ,"WM_ENTERMENULOOP 0x0211 "},
 {WM_EXITMENULOOP                 ,"WM_EXITMENULOOP 0x0212 "},
 {WM_NEXTMENU                     ,"WM_NEXTMENU 0x0213 "},
 {WM_SIZING                       ,"WM_SIZING 0x0214 "},
 {WM_CAPTURECHANGED               ,"WM_CAPTURECHANGED 0x0215 "},
 {WM_MOVING                       ,"WM_MOVING 0x0216 "},
 {WM_POWERBROADCAST               ,"WM_POWERBROADCAST 0x0218 "},
 {WM_DEVICECHANGE                 ,"WM_DEVICECHANGE 0x0219 "},
 {WM_IME_SETCONTEXT               ,"WM_IME_SETCONTEXT 0x0281 "},
 {WM_IME_NOTIFY                   ,"WM_IME_NOTIFY 0x0282 "},
 {WM_IME_CONTROL                  ,"WM_IME_CONTROL 0x0283 "},
 {WM_IME_COMPOSITIONFULL          ,"WM_IME_COMPOSITIONFULL 0x0284 "},
 {WM_IME_SELECT                   ,"WM_IME_SELECT 0x0285 "},
 {WM_IME_CHAR                     ,"WM_IME_CHAR 0x0286 "},
 {WM_IME_KEYDOWN                  ,"WM_IME_KEYDOWN 0x0290 "},
 {WM_IME_KEYUP                    ,"WM_IME_KEYUP 0x0291 "},
 {WM_MDICREATE                    ,"WM_MDICREATE 0x0220 "},
 {WM_MDIDESTROY                   ,"WM_MDIDESTROY 0x0221 "},
 {WM_MDIACTIVATE                  ,"WM_MDIACTIVATE 0x0222 "},
 {WM_MDIRESTORE                   ,"WM_MDIRESTORE 0x0223 "},
 {WM_MDINEXT                      ,"WM_MDINEXT 0x0224 "},
 {WM_MDIMAXIMIZE                  ,"WM_MDIMAXIMIZE 0x0225 "},
 {WM_MDITILE                      ,"WM_MDITILE 0x0226 "},
 {WM_MDICASCADE                   ,"WM_MDICASCADE 0x0227 "},
 {WM_MDIICONARRANGE               ,"WM_MDIICONARRANGE 0x0228 "},
 {WM_MDIGETACTIVE                 ,"WM_MDIGETACTIVE 0x0229 "},
 {WM_MDISETMENU                   ,"WM_MDISETMENU 0x0230 "},
 {WM_ENTERSIZEMOVE                ,"WM_ENTERSIZEMOVE 0x0231 "},
 {WM_EXITSIZEMOVE                 ,"WM_EXITSIZEMOVE 0x0232 "},
 {WM_DROPFILES                    ,"WM_DROPFILES 0x0233 "},
 {WM_MDIREFRESHMENU               ,"WM_MDIREFRESHMENU 0x0234 "},
// {WM_MOUSEHOVER                   ,"WM_MOUSEHOVER 0x02A1 "},
 { 0x02a1                         ,"WM_MOUSEHOVER 0x02A1 "},
// {WM_MOUSELEAVE                   ,"WM_MOUSELEAVE 0x02A3 "},
 { 0x02a3                         ,"WM_MOUSELEAVE 0x02A3 "},
 {WM_CUT                          ,"WM_CUT 0x0300 "},
 {WM_COPY                         ,"WM_COPY 0x0301 "},
 {WM_PASTE                        ,"WM_PASTE 0x0302 "},
 {WM_CLEAR                        ,"WM_CLEAR 0x0303 "},
 {WM_UNDO                         ,"WM_UNDO 0x0304 "},
 {WM_RENDERFORMAT                 ,"WM_RENDERFORMAT 0x0305 "},
 {WM_RENDERALLFORMATS             ,"WM_RENDERALLFORMATS 0x0306 "},
 {WM_DESTROYCLIPBOARD             ,"WM_DESTROYCLIPBOARD 0x0307 "},
 {WM_DRAWCLIPBOARD                ,"WM_DRAWCLIPBOARD 0x0308 "},
 {WM_PAINTCLIPBOARD               ,"WM_PAINTCLIPBOARD 0x0309 "},
 {WM_VSCROLLCLIPBOARD             ,"WM_VSCROLLCLIPBOARD 0x030A "},
 {WM_SIZECLIPBOARD                ,"WM_SIZECLIPBOARD 0x030B "},
 {WM_ASKCBFORMATNAME              ,"WM_ASKCBFORMATNAME 0x030C "},
 {WM_CHANGECBCHAIN                ,"WM_CHANGECBCHAIN 0x030D "},
 {WM_HSCROLLCLIPBOARD             ,"WM_HSCROLLCLIPBOARD 0x030E "},
 {WM_QUERYNEWPALETTE              ,"WM_QUERYNEWPALETTE 0x030F "},
 {WM_PALETTEISCHANGING            ,"WM_PALETTEISCHANGING 0x0310 "},
 {WM_PALETTECHANGED               ,"WM_PALETTECHANGED 0x0311 "},
 {WM_HOTKEY                       ,"WM_HOTKEY 0x0312 "},
 {WM_PRINT                        ,"WM_PRINT 0x0317 "},
 {WM_PRINTCLIENT                  ,"WM_PRINTCLIENT 0x0318 "},
 {WM_HANDHELDFIRST                ,"WM_HANDHELDFIRST 0x0358 "},
 {WM_HANDHELDLAST                 ,"WM_HANDHELDLAST 0x035F "},
 {WM_AFXFIRST                     ,"WM_AFXFIRST 0x0360 "},
 {WM_AFXLAST                      ,"WM_AFXLAST 0x037F "},
 {WM_PENWINFIRST                  ,"WM_PENWINFIRST 0x0380 "},
 {WM_PENWINLAST                   ,"WM_PENWINLAST 0x038F "},
 {WM_APP                          ,"WM_APP 0x8000 "},
 {WM_USER                         ,"WM_USER 0x0400 "},
 {(UINT)-1                        ,"WM_UNKNOWN "}
};


// Specifies the notification code.
// This member can be a control-specific notification code,
// or it can be one of the following common notification values:
// Value Meaning
WMSTR NmStr[] = {
	{ NM_CLICK, "NM_CLICK" }, // The user has clicked the left mouse button within the control.
	{ NM_DBLCLK, "NM_DBLCLK" }, // The user has double-clicked the left mouse button within the control.
	{ NM_KILLFOCUS, "NM_KILLFOCUS" }, // The control has lost the input focus.
	{ NM_OUTOFMEMORY, "NM_OUTOFMEMORY" }, // The control could not complete an operation because there was not enough memory available. 
	{ NM_RCLICK, "NM_RCLICK" }, // The user has clicked the right mouse button within the control.
	{ NM_RDBLCLK, "NM_RDBLCLK" }, // The user has double-clicked the right mouse button within the control.
	{ NM_RETURN, "NM_RETURN" }, // The control has the input focus, and the user has pressed the ENTER key.
	{ NM_SETFOCUS, "NM_SETFOCUS" }, // The control has received the input focus.
	{ NM_CUSTOMDRAW, "NM_CUSTOMDRAW" },
	{ NM_HOVER, "NM_HOVER" }
};

#define		nNmCount	(sizeof( NmStr ) / sizeof( WMSTR ))

// C:\MSDEV\INCLUDE\COMMCTRL.H
// #define LVN_FIRST               (0U-100U)       // listview 
// #define LVN_LAST                (0U-199U) 
WMSTR LvnStr[] = {
	{ LVN_ITEMCHANGING, "LVN_ITEMCHANGING" },
	{ LVN_ITEMCHANGED, "LVN_ITEMCHANGED" },
	{ LVN_INSERTITEM, "LVN_INSERTITEM" },
	{ LVN_DELETEITEM, "LVN_DELETEITEM" },
	{ LVN_DELETEALLITEMS, "LVN_DELETEALLITEMS" },
	{ LVN_COLUMNCLICK, "LVN_COLUMNCLICK" },
	{ LVN_BEGINDRAG, "LVN_BEGINDRAG" },
	{ LVN_BEGINRDRAG, "LVN_BEGINRDRAG" },
	{ LVN_ODCACHEHINT, "LVN_ODCACHEHINT" },
	{ LVN_ITEMACTIVATE, "LVN_ITEMACTIVATE" },
	{ LVN_ODFINDITEM, "LVN_ODFINDITEM" },
	{ LVN_BEGINLABELEDIT, "LVN_BEGINLABELEDIT" },
	{ LVN_ENDLABELEDIT, "LVN_ENDLABELEDIT" },
	{ LVN_GETDISPINFO, "LVN_GETDISPINFO" },
	{ LVN_SETDISPINFO, "LVN_SETDISPINFO" },
	{ LVN_KEYDOWN, "LVN_KEYDOWN" },	//  (LVN_FIRST-55) 
	{ LVN_MARQUEEBEGIN, "LVN_MARQUEEBEGIN" } //(LVN_FIRST-56) 
};

#define		nLvnCount	(sizeof( LvnStr ) / sizeof( WMSTR ))

WMSTR2 LvnStr2[] = {
	{ LVN_ITEMCHANGING, "LVN_ITEMCHANGING", 0, Generic },
	{ LVN_ITEMCHANGED, "LVN_ITEMCHANGED", 0, Generic  },
	{ LVN_INSERTITEM, "LVN_INSERTITEM", 0, GNotify  },
	{ LVN_DELETEITEM, "LVN_DELETEITEM", 0, GNotify  },
	{ LVN_DELETEALLITEMS, "LVN_DELETEALLITEMS", 0, GNotify  },
	{ LVN_COLUMNCLICK, "LVN_COLUMNCLICK", 0, Generic  },
	{ LVN_BEGINDRAG, "LVN_BEGINDRAG", 0, Generic  },
	{ LVN_BEGINRDRAG, "LVN_BEGINRDRAG", 0, Generic  },
	{ LVN_ODCACHEHINT, "LVN_ODCACHEHINT", 0, Generic  },
	{ LVN_ITEMACTIVATE, "LVN_ITEMACTIVATE", 0, Generic  },
	{ LVN_ODFINDITEM, "LVN_ODFINDITEM", 0, Generic  },
	{ LVN_BEGINLABELEDIT, "LVN_BEGINLABELEDIT", 0, Generic  },
	{ LVN_ENDLABELEDIT, "LVN_ENDLABELEDIT", 0, Generic  },
	{ LVN_GETDISPINFO, "LVN_GETDISPINFO", 0, Generic  },
	{ LVN_SETDISPINFO, "LVN_SETDISPINFO", 0, Generic  },
	{ LVN_KEYDOWN, "LVN_KEYDOWN", 0, Generic  },	//  (LVN_FIRST-55) 
	{ LVN_MARQUEEBEGIN, "LVN_MARQUEEBEGIN", 0, Generic  } //(LVN_FIRST-56) 
};

#define		nLvnCount2	(sizeof( LvnStr2 ) / sizeof( WMSTR2 ))

WMSTR2 NmStr2[] = {
	{ NM_CLICK, "NM_CLICK", 0, Generic }, // The user has clicked the left mouse button within the control.
	{ NM_DBLCLK, "NM_DBLCLK", 0, Generic }, // The user has double-clicked the left mouse button within the control.
	{ NM_KILLFOCUS, "NM_KILLFOCUS", 0, Generic }, // The control has lost the input focus.
	{ NM_OUTOFMEMORY, "NM_OUTOFMEMORY", 0, Generic }, // The control could not complete an operation because there was not enough memory available. 
	{ NM_RCLICK, "NM_RCLICK", 0, Generic }, // The user has clicked the right mouse button within the control.
	{ NM_RDBLCLK, "NM_RDBLCLK", 0, Generic }, // The user has double-clicked the right mouse button within the control.
	{ NM_RETURN, "NM_RETURN", 0, Generic }, // The control has the input focus, and the user has pressed the ENTER key.
	{ NM_SETFOCUS, "NM_SETFOCUS", 0, Generic }, // The control has received the input focus.
	{ NM_CUSTOMDRAW, "NM_CUSTOMDRAW", 0, Generic },
	{ NM_HOVER, "NM_HOVER", 0, Generic }
};

#define		nNmCount2	(sizeof( NmStr2 ) / sizeof( WMSTR2 ))

WMSTR2 WmStr2[] = {
 {WM_NULL ,"WM_NULL 0x0000", 0, Generic },
 {WM_CREATE ,"WM_CREATE 0x0001", 0, Generic },
 {WM_DESTROY ,"WM_DESTROY 0x0002", 0, Generic },
 {WM_MOVE ,"WM_MOVE 0x0003", 0, Generic },
 {WM_SIZE ,"WM_SIZE 0x0005", 0, Generic },
 {WM_ACTIVATE ,"WM_ACTIVATE 0x0006", 0, Generic },
 {WM_SETFOCUS ,"WM_SETFOCUS 0x0007", 0, Generic },
 {WM_KILLFOCUS ,"WM_KILLFOCUS 0x0008", 0, Generic },
 {WM_ENABLE ,"WM_ENABLE 0x000A", 0, Generic },
 {WM_SETREDRAW ,"WM_SETREDRAW 0x000B", 0, Generic },
 {WM_SETTEXT ,"WM_SETTEXT 0x000C", 0, Generic },
 {WM_GETTEXT ,"WM_GETTEXT 0x000D", 0, Generic },
 {WM_GETTEXTLENGTH ,"WM_GETTEXTLENGTH 0x000E", 0, Generic },
 {WM_PAINT ,"WM_PAINT 0x000F", 0, Generic },
 {WM_CLOSE ,"WM_CLOSE 0x0010", 0, Generic },
 {WM_QUERYENDSESSION ,"WM_QUERYENDSESSION 0x0011", 0, Generic },
 {WM_QUIT ,"WM_QUIT 0x0012", 0, Generic },
 {WM_QUERYOPEN ,"WM_QUERYOPEN 0x0013", 0, Generic },
 {WM_ERASEBKGND ,"WM_ERASEBKGND 0x0014", 0, Generic },
 {WM_SYSCOLORCHANGE ,"WM_SYSCOLORCHANGE 0x0015", 0, Generic },
 {WM_ENDSESSION ,"WM_ENDSESSION 0x0016", 0, Generic },
 {WM_SHOWWINDOW ,"WM_SHOWWINDOW 0x0018", 0, Generic },
 {WM_WININICHANGE ,"WM_WININICHANGE 0x001A", 0, Generic },
 {WM_DEVMODECHANGE ,"WM_DEVMODECHANGE 0x001B", 0, Generic },
 {WM_ACTIVATEAPP ,"WM_ACTIVATEAPP 0x001C", 0, Generic },
 {WM_FONTCHANGE ,"WM_FONTCHANGE 0x001D", 0, Generic },
 {WM_TIMECHANGE ,"WM_TIMECHANGE 0x001E", 0, Generic },
 {WM_CANCELMODE ,"WM_CANCELMODE 0x001F", 0, Generic },
 {WM_SETCURSOR ,"WM_SETCURSOR 0x0020", 0, Generic },
 {WM_MOUSEACTIVATE ,"WM_MOUSEACTIVATE 0x0021", 0, Generic },
 {WM_CHILDACTIVATE ,"WM_CHILDACTIVATE 0x0022", 0, Generic },
 {WM_QUEUESYNC ,"WM_QUEUESYNC 0x0023", 0, Generic },
 {WM_GETMINMAXINFO ,"WM_GETMINMAXINFO 0x0024", 0, Generic },
 {WM_PAINTICON ,"WM_PAINTICON 0x0026", 0, Generic },
 {WM_ICONERASEBKGND ,"WM_ICONERASEBKGND 0x0027", 0, Generic },
 {WM_NEXTDLGCTL ,"WM_NEXTDLGCTL 0x0028", 0, Generic },
 {WM_SPOOLERSTATUS ,"WM_SPOOLERSTATUS 0x002A", 0, Generic },
 {WM_DRAWITEM ,"WM_DRAWITEM 0x002B", 0, Generic },
 {WM_MEASUREITEM ,"WM_MEASUREITEM 0x002C", 0, Generic },
 {WM_DELETEITEM ,"WM_DELETEITEM 0x002D", 0, Generic },
 {WM_VKEYTOITEM ,"WM_VKEYTOITEM 0x002E", 0, Generic },
 {WM_CHARTOITEM ,"WM_CHARTOITEM 0x002F", 0, Generic },
 {WM_SETFONT ,"WM_SETFONT 0x0030", 0, Generic },
 {WM_GETFONT ,"WM_GETFONT 0x0031", 0, Generic },
 {WM_SETHOTKEY ,"WM_SETHOTKEY 0x0032", 0, Generic },
 {WM_GETHOTKEY ,"WM_GETHOTKEY 0x0033", 0, Generic },
 {WM_QUERYDRAGICON ,"WM_QUERYDRAGICON 0x0037", 0, Generic },
 {WM_COMPAREITEM ,"WM_COMPAREITEM 0x0039", 0, Generic },
 {WM_COMPACTING ,"WM_COMPACTING 0x0041", 0, Generic },
 {WM_WINDOWPOSCHANGING ,"WM_WINDOWPOSCHANGING 0x0046", 0, Generic },
 {WM_WINDOWPOSCHANGED ,"WM_WINDOWPOSCHANGED 0x0047", 0, Generic },
 {WM_POWER ,"WM_POWER 0x0048", 0, Generic },
 {WM_COPYDATA ,"WM_COPYDATA 0x004A", 0, Generic },
 {WM_CANCELJOURNAL ,"WM_CANCELJOURNAL 0x004B", 0, Generic },
 {WM_NOTIFY ,"WM_NOTIFY 0x004E", 0, Generic },
 {WM_INPUTLANGCHANGEREQUEST ,"WM_INPUTLANGCHANGEREQUEST 0x0050", 0, Generic },
 {WM_INPUTLANGCHANGE ,"WM_INPUTLANGCHANGE 0x0051", 0, Generic },
 {WM_TCARD ,"WM_TCARD 0x0052", 0, Generic },
 {WM_HELP ,"WM_HELP 0x0053", 0, Generic },
 {WM_USERCHANGED ,"WM_USERCHANGED 0x0054", 0, Generic },
 {WM_NOTIFYFORMAT ,"WM_NOTIFYFORMAT 0x0055", 0, Generic },
 {WM_CONTEXTMENU ,"WM_CONTEXTMENU 0x007B", 0, Generic },
 {WM_STYLECHANGING ,"WM_STYLECHANGING 0x007C", 0, Generic },
 {WM_STYLECHANGED ,"WM_STYLECHANGED 0x007D", 0, Generic },
 {WM_DISPLAYCHANGE ,"WM_DISPLAYCHANGE 0x007E", 0, Generic },
 {WM_GETICON ,"WM_GETICON 0x007F", 0, Generic },
 {WM_SETICON ,"WM_SETICON 0x0080", 0, Generic },
 {WM_NCCREATE ,"WM_NCCREATE 0x0081", 0, Generic },
 {WM_NCDESTROY ,"WM_NCDESTROY 0x0082", 0, Generic },
 {WM_NCCALCSIZE ,"WM_NCCALCSIZE 0x0083", 0, Generic },
 {WM_NCHITTEST ,"WM_NCHITTEST 0x0084", 0, Generic },
 {WM_NCPAINT ,"WM_NCPAINT 0x0085", 0, Generic },
 {WM_NCACTIVATE ,"WM_NCACTIVATE 0x0086", 0, Generic },
 {WM_GETDLGCODE ,"WM_GETDLGCODE 0x0087", 0, Generic },
 {WM_NCMOUSEMOVE ,"WM_NCMOUSEMOVE 0x00A0", 0, Generic },
 {WM_NCLBUTTONDOWN ,"WM_NCLBUTTONDOWN 0x00A1", 0, Generic },
 {WM_NCLBUTTONUP ,"WM_NCLBUTTONUP 0x00A2", 0, Generic },
 {WM_NCLBUTTONDBLCLK ,"WM_NCLBUTTONDBLCLK 0x00A3", 0, Generic },
 {WM_NCRBUTTONDOWN ,"WM_NCRBUTTONDOWN 0x00A4", 0, Generic },
 {WM_NCRBUTTONUP ,"WM_NCRBUTTONUP 0x00A5", 0, Generic },
 {WM_NCRBUTTONDBLCLK ,"WM_NCRBUTTONDBLCLK 0x00A6", 0, Generic },
 {WM_NCMBUTTONDOWN ,"WM_NCMBUTTONDOWN 0x00A7", 0, Generic },
 {WM_NCMBUTTONUP ,"WM_NCMBUTTONUP 0x00A8", 0, Generic },
 {WM_NCMBUTTONDBLCLK ,"WM_NCMBUTTONDBLCLK 0x00A9", 0, Generic },
 {WM_KEYFIRST ,"WM_KEYFIRST 0x0100", 0, Generic },
 {WM_KEYDOWN ,"WM_KEYDOWN 0x0100", 0, Generic },
 {WM_KEYUP ,"WM_KEYUP 0x0101", 0, Generic },
 {WM_CHAR ,"WM_CHAR 0x0102", 0, Generic },
 {WM_DEADCHAR ,"WM_DEADCHAR 0x0103", 0, Generic },
 {WM_SYSKEYDOWN ,"WM_SYSKEYDOWN 0x0104", 0, Generic },
 {WM_SYSKEYUP ,"WM_SYSKEYUP 0x0105", 0, Generic },
 {WM_SYSCHAR ,"WM_SYSCHAR 0x0106", 0, Generic },
 {WM_SYSDEADCHAR ,"WM_SYSDEADCHAR 0x0107", 0, Generic },
 {WM_KEYLAST ,"WM_KEYLAST 0x0108", 0, Generic },
 {WM_IME_STARTCOMPOSITION ,"WM_IME_STARTCOMPOSITION 0x010D", 0, Generic },
 {WM_IME_ENDCOMPOSITION ,"WM_IME_ENDCOMPOSITION 0x010E", 0, Generic },
 {WM_IME_COMPOSITION ,"WM_IME_COMPOSITION 0x010F", 0, Generic },
 {WM_IME_KEYLAST ,"WM_IME_KEYLAST 0x010F", 0, Generic },
 {WM_INITDIALOG ,"WM_INITDIALOG 0x0110", 0, Generic },
 {WM_COMMAND ,"WM_COMMAND 0x0111", 0, Generic },
 {WM_SYSCOMMAND ,"WM_SYSCOMMAND 0x0112", 0, Generic },
 {WM_TIMER ,"WM_TIMER 0x0113", 0, Generic },
 {WM_HSCROLL ,"WM_HSCROLL 0x0114", 0, Generic },
 {WM_VSCROLL ,"WM_VSCROLL 0x0115", 0, Generic },
 {WM_INITMENU ,"WM_INITMENU 0x0116", 0, Generic },
 {WM_INITMENUPOPUP ,"WM_INITMENUPOPUP 0x0117", 0, Generic },
 {WM_MENUSELECT ,"WM_MENUSELECT 0x011F", 0, Generic },
 {WM_MENUCHAR ,"WM_MENUCHAR 0x0120", 0, Generic },
 {WM_ENTERIDLE ,"WM_ENTERIDLE 0x0121", 0, Generic },
 {WM_CTLCOLORMSGBOX ,"WM_CTLCOLORMSGBOX 0x0132", 0, Generic },
 {WM_CTLCOLOREDIT ,"WM_CTLCOLOREDIT 0x0133", 0, Generic },
 {WM_CTLCOLORLISTBOX ,"WM_CTLCOLORLISTBOX 0x0134", 0, Generic },
 {WM_CTLCOLORBTN ,"WM_CTLCOLORBTN 0x0135", 0, Generic },
 {WM_CTLCOLORDLG ,"WM_CTLCOLORDLG 0x0136", 0, Generic },
 {WM_CTLCOLORSCROLLBAR ,"WM_CTLCOLORSCROLLBAR 0x0137", 0, Generic },
 {WM_CTLCOLORSTATIC ,"WM_CTLCOLORSTATIC 0x0138", 0, Generic },
// {WM_MOUSEFIRST ,"WM_MOUSEFIRST 0x0200", 0, Generic },
 {WM_MOUSEMOVE ,"WM_MOUSEMOVE 0x0200", 0, Generic },
 {WM_LBUTTONDOWN ,"WM_LBUTTONDOWN 0x0201", 0, Generic },
 {WM_LBUTTONUP ,"WM_LBUTTONUP 0x0202", 0, Generic },
 {WM_LBUTTONDBLCLK ,"WM_LBUTTONDBLCLK 0x0203", 0, Generic },
 {WM_RBUTTONDOWN ,"WM_RBUTTONDOWN 0x0204", 0, Generic },
 {WM_RBUTTONUP ,"WM_RBUTTONUP 0x0205", 0, Generic },
 {WM_RBUTTONDBLCLK ,"WM_RBUTTONDBLCLK 0x0206", 0, Generic },
 {WM_MBUTTONDOWN ,"WM_MBUTTONDOWN 0x0207", 0, Generic },
 {WM_MBUTTONUP ,"WM_MBUTTONUP 0x0208", 0, Generic },
 {WM_MBUTTONDBLCLK ,"WM_MBUTTONDBLCLK 0x0209", 0, Generic },
// {WM_MOUSEWHEEL ,"WM_MOUSEWHEEL 0x020A", 0, Generic },
 { 0x020a ,"WM_MOUSEWHEEL 0x020A", 0, Generic },
// {WM_MOUSELAST ,"WM_MOUSELAST 0x0209", 0, Generic },
// {WM_MOUSELAST ,"WM_MOUSELAST 0x020A", 0, Generic },
 {WM_PARENTNOTIFY ,"WM_PARENTNOTIFY 0x0210", 0, Generic },
 {WM_ENTERMENULOOP ,"WM_ENTERMENULOOP 0x0211", 0, Generic },
 {WM_EXITMENULOOP ,"WM_EXITMENULOOP 0x0212", 0, Generic },
 {WM_NEXTMENU ,"WM_NEXTMENU 0x0213", 0, Generic },
 {WM_SIZING ,"WM_SIZING 0x0214", 0, Generic },
 {WM_CAPTURECHANGED ,"WM_CAPTURECHANGED 0x0215", 0, Generic },
 {WM_MOVING ,"WM_MOVING 0x0216", 0, Generic },
 {WM_POWERBROADCAST ,"WM_POWERBROADCAST 0x0218", 0, Generic },
 {WM_DEVICECHANGE ,"WM_DEVICECHANGE 0x0219", 0, Generic },
 {WM_IME_SETCONTEXT ,"WM_IME_SETCONTEXT 0x0281", 0, Generic },
 {WM_IME_NOTIFY ,"WM_IME_NOTIFY 0x0282", 0, Generic },
 {WM_IME_CONTROL ,"WM_IME_CONTROL 0x0283", 0, Generic },
 {WM_IME_COMPOSITIONFULL ,"WM_IME_COMPOSITIONFULL 0x0284", 0, Generic },
 {WM_IME_SELECT ,"WM_IME_SELECT 0x0285", 0, Generic },
 {WM_IME_CHAR ,"WM_IME_CHAR 0x0286", 0, Generic },
 {WM_IME_KEYDOWN ,"WM_IME_KEYDOWN 0x0290", 0, Generic },
 {WM_IME_KEYUP ,"WM_IME_KEYUP 0x0291", 0, Generic },
 {WM_MDICREATE ,"WM_MDICREATE 0x0220", 0, Generic },
 {WM_MDIDESTROY ,"WM_MDIDESTROY 0x0221", 0, Generic },
 {WM_MDIACTIVATE ,"WM_MDIACTIVATE 0x0222", 0, Generic },
 {WM_MDIRESTORE ,"WM_MDIRESTORE 0x0223", 0, Generic },
 {WM_MDINEXT ,"WM_MDINEXT 0x0224", 0, Generic },
 {WM_MDIMAXIMIZE ,"WM_MDIMAXIMIZE 0x0225", 0, Generic },
 {WM_MDITILE ,"WM_MDITILE 0x0226", 0, Generic },
 {WM_MDICASCADE ,"WM_MDICASCADE 0x0227", 0, Generic },
 {WM_MDIICONARRANGE ,"WM_MDIICONARRANGE 0x0228", 0, Generic },
 {WM_MDIGETACTIVE ,"WM_MDIGETACTIVE 0x0229", 0, Generic },
 {WM_MDISETMENU ,"WM_MDISETMENU 0x0230", 0, Generic },
 {WM_ENTERSIZEMOVE ,"WM_ENTERSIZEMOVE 0x0231", 0, Generic },
 {WM_EXITSIZEMOVE ,"WM_EXITSIZEMOVE 0x0232", 0, Generic },
 {WM_DROPFILES ,"WM_DROPFILES 0x0233", 0, Generic },
 {WM_MDIREFRESHMENU ,"WM_MDIREFRESHMENU 0x0234", 0, Generic },
// {WM_MOUSEHOVER ,"WM_MOUSEHOVER 0x02A1", 0, Generic },
 { 0x02a1 ,"WM_MOUSEHOVER 0x02A1", 0, Generic },
// {WM_MOUSELEAVE ,"WM_MOUSELEAVE 0x02A3", 0, Generic },
 { 0x02a3 ,"WM_MOUSELEAVE 0x02A3", 0, Generic },
 {WM_CUT ,"WM_CUT 0x0300", 0, Generic },
 {WM_COPY ,"WM_COPY 0x0301", 0, Generic },
 {WM_PASTE ,"WM_PASTE 0x0302", 0, Generic },
 {WM_CLEAR ,"WM_CLEAR 0x0303", 0, Generic },
 {WM_UNDO ,"WM_UNDO 0x0304", 0, Generic },
 {WM_RENDERFORMAT ,"WM_RENDERFORMAT 0x0305", 0, Generic },
 {WM_RENDERALLFORMATS ,"WM_RENDERALLFORMATS 0x0306", 0, Generic },
 {WM_DESTROYCLIPBOARD ,"WM_DESTROYCLIPBOARD 0x0307", 0, Generic },
 {WM_DRAWCLIPBOARD ,"WM_DRAWCLIPBOARD 0x0308", 0, Generic },
 {WM_PAINTCLIPBOARD ,"WM_PAINTCLIPBOARD 0x0309", 0, Generic },
 {WM_VSCROLLCLIPBOARD ,"WM_VSCROLLCLIPBOARD 0x030A", 0, Generic },
 {WM_SIZECLIPBOARD ,"WM_SIZECLIPBOARD 0x030B", 0, Generic },
 {WM_ASKCBFORMATNAME ,"WM_ASKCBFORMATNAME 0x030C", 0, Generic },
 {WM_CHANGECBCHAIN ,"WM_CHANGECBCHAIN 0x030D", 0, Generic },
 {WM_HSCROLLCLIPBOARD ,"WM_HSCROLLCLIPBOARD 0x030E", 0, Generic },
 {WM_QUERYNEWPALETTE ,"WM_QUERYNEWPALETTE 0x030F", 0, Generic },
 {WM_PALETTEISCHANGING ,"WM_PALETTEISCHANGING 0x0310", 0, Generic },
 {WM_PALETTECHANGED ,"WM_PALETTECHANGED 0x0311", 0, Generic },
 {WM_HOTKEY ,"WM_HOTKEY 0x0312", 0, Generic },
 {WM_PRINT ,"WM_PRINT 0x0317", 0, Generic },
 {WM_PRINTCLIENT ,"WM_PRINTCLIENT 0x0318", 0, Generic },
 {WM_HANDHELDFIRST ,"WM_HANDHELDFIRST 0x0358", 0, Generic },
 {WM_HANDHELDLAST ,"WM_HANDHELDLAST 0x035F", 0, Generic },
 {WM_AFXFIRST ,"WM_AFXFIRST 0x0360", 0, Generic },
 {WM_AFXLAST ,"WM_AFXLAST 0x037F", 0, Generic },
 {WM_PENWINFIRST ,"WM_PENWINFIRST 0x0380", 0, Generic },
 {WM_PENWINLAST ,"WM_PENWINLAST 0x038F", 0, Generic },
 {WM_APP ,"WM_APP 0x8000", 0, Generic },
 {WM_USER ,"WM_USER 0x0400", 0, Generic },
};

#define		nWmCount2	(sizeof( WmStr2 ) / sizeof( WMSTR2 ))

WMSTR2	HdnStr2[] = {
	{ HDN_ITEMCHANGING, "HDN_ITEMCHANGING", 0, Generic },
	{ HDN_ITEMCHANGED,  "HDN_ITEMCHANGED" , 0, Generic },
	{ HDN_ITEMCLICK,    "HDN_ITEMCLICK",    0, Generic },
	{ HDN_ITEMDBLCLICK, "HDN_ITEMDBLCLICK", 0, Generic },
	{ HDN_DIVIDERDBLCLICK,"HDN_DIVIDERDBLCLICK",0,Generic},
	{ HDN_BEGINTRACK,   "HDN_BEGINTRACK",   0, Generic },
	{ HDN_ENDTRACK,     "HDN_ENDTRACK",     0, Generic },
	{ HDN_TRACK,        "HDN_TRACK",        0, Generic },
	{ HDN_GETDISPINFO,  "HDN_GETDISPINFO",  0, Generic },
	{ HDN_BEGINDRAG,    "HDN_BEGINDRAG",    0, Generic },
	{ HDN_ENDDRAG,      "HDN_ENDDRAG",      0, Generic }
};

#define	nHdnCount2	(sizeof( HdnStr2 ) / sizeof( WMSTR2 ))

LPSTR	GetNMStg( UINT uMsg )
{
	LPSTR	lps;
	int		i;

	lps = 0;
	for( i = 0; i < nNmCount; i++ )
	{
		if( NmStr[i].val == uMsg )
		{
			lps = NmStr[i].lps;
			break;
		}
	}
	return lps;
}

LPSTR	GetNMStg2( HWND hWnd, UINT uMsg,
				  WPARAM wParam, LPARAM lParam )
{
	LPSTR	lps;
	int		i;

	lps = 0;
	for( i = 0; i < nWmCount2; i++ )
	{
		if( NmStr2[i].val == uMsg )
		{
			lps = NmStr2[i].lps;
			if( NmStr2[i].dec )
				lps = (*NmStr2[i].dec)(hWnd, uMsg, wParam, lParam, lps );
			break;
		}
	}
	if( lps == 0 )
	{
		lps = Generic(hWnd, uMsg, wParam, lParam, lps );
	}
	return lps;
}

LPSTR	GetLVNStg( UINT uMsg )
{
	LPSTR	lps;
	int		i;

	lps = 0;
	for( i = 0; i < nLvnCount; i++ )
	{
		if( LvnStr[i].val == uMsg )
		{
			lps = LvnStr[i].lps;
			break;
		}
	}
	return lps;
}

LPSTR	GetNotify( UINT uMsg )
{
	LPSTR	lps;
	if( (lps = GetNMStg( uMsg )) == 0 )
		lps = GetLVNStg( uMsg );
	return lps;
}

LPSTR	GetNotify2( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	int		i;
	LPSTR	lps;
	lps = 0;
	for( i = 0; i < nLvnCount2; i++ )
	{
		if( LvnStr2[i].val == uMsg )
		{
			lps = LvnStr2[i].lps;
			if( LvnStr2[i].dec )
				lps = (*LvnStr2[i].dec)( hWnd, uMsg, wParam, lParam, lps );
			break;
		}
	}
	if( lps == 0 )
	{
		for( i = 0; i < nNmCount2; i++ )
		{
			if( NmStr2[i].val == uMsg )
			{
				lps = NmStr2[i].lps;
				if( NmStr2[i].dec )
					lps = (*NmStr2[i].dec)( hWnd, uMsg, wParam, lParam, lps );
				break;
			}
		}
	}
	if( lps == 0 )
	{
		for( i = 0; i < nHdnCount2; i++ )
		{
			if( HdnStr2[i].val == uMsg )
			{
				lps = HdnStr2[i].lps;
				if( HdnStr2[i].dec )
					lps = (*HdnStr2[i].dec)( hWnd, uMsg, wParam, lParam, lps );
				break;
			}
		}
	}
	if( lps == 0 )
	{
		lps = Generic( hWnd, uMsg, wParam, lParam, lps );
	}
	return lps;
}

LPTSTR	RetWMStg( UINT uMsg )
{
   LPTSTR    lps = 0;
   UINT     ui;
   LPWMSTR  pwm = &WmStr[0];
   while( ( ui = pwm->val ) != (UINT)-1 )
   {
		if( ui == uMsg )
		{
			lps = pwm->lps;
			break;
		}
      pwm++;   // bump to next message
   }
   return lps;
}

LPSTR	GetWMStg( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	LPSTR	lps;
	int		i;
	UINT	ui;
	static char	szGStg2[1024];

	lps = 0;
	for( i = 0; ; i++ )
	{
		ui = WmStr[i].val;
		if( ui == uMsg )
		{
			lps = WmStr[i].lps;
			break;
		}
		if( ui == (UINT)-1 )
		{
//#ifdef	BGLVIEW2
//			lps = GetszTmpBuf2();
//#else	// !BGLVIEW2
			lps = &szGStg2[0];
//			lps = &szGStg1[0];
//#endif	// BGLVIEW2 y/n
			if( uMsg > WM_USER )
			{
				UINT	ud;
				ud = uMsg - WM_USER;
				wsprintf( lps,
					"WM_USER + %d (0x%x) wP=0x%x lP=0x%x",
					ud, uMsg,
					wParam,
					lParam );
			}
			else
			{
				wsprintf( lps, "%s 0x%x(%d) wP=0x%x lP=0x%x",
					WmStr[i].lps,
					uMsg, uMsg,
					wParam,
					lParam );
			}
			break;
		}
	}
	return lps;
}

LPSTR	Generic( HWND hWnd, UINT uMsg,
				WPARAM wParam, LPARAM lParam,
				LPSTR lpm )
{
	LPSTR	lps;
	lps = &szGStg1[0];
	if( lpm )
	{
		wsprintf( lps,
			"H=0x%x %s WP=0x%x LP=0x%x",
			hWnd,
			lpm,
			wParam,
			lParam );
	}
	else
	{
		wsprintf( lps,
			"H=0x%x ***UNLISTED 0x%x (%d)*** WP=0x%x LP=0x%x",
			hWnd,
			uMsg, uMsg,
			wParam,
			lParam );
	}
	return lps;
}

LPSTR	GNotify( HWND hWnd, UINT uMsg,
				WPARAM wParam, LPARAM lParam,
				LPSTR lpm )
{
	LPSTR	lps;
	lps = &szGStg1[0];
	if( lpm )
	{
		wsprintf( lps,
			"%s",
			lpm );
	}
	else
	{
		wsprintf( lps,
			"H=0x%x ***UNLISTED 0x%x (%d)*** WP=0x%x LP=0x%x",
			hWnd,
			uMsg, uMsg,
			wParam,
			lParam );
	}
	return lps;
}

WMSTR	MActLst[] = {
	{ MA_ACTIVATE, "MA_ACTIVATE" }, //Activates the window, and does not discard the mouse message.
	{ MA_ACTIVATEANDEAT, "MA_ACTIVATEANDEAT" }, //Activates the window, and discards the mouse message.
	{ MA_NOACTIVATE, "MA_NOACTIVATE" }, //Does not activate the window, and does not discard the mouse message.
	{ MA_NOACTIVATEANDEAT, "MA_NOACTIVATEANDEAT" }, //Does not activate the window, but discards the mouse message.
	{ (UINT)-1, "MA_UNKNOWN" }
};

// HIWORD
LPSTR	GetMAMsg( UINT uMsg )
{
	LPSTR	lps;
	LPWMSTR	lpwm;

	lpwm = &MActLst[0];
	while( lpwm->val != (UINT)-1 )
	{
		if( lpwm->val == uMsg )
			break;
		lpwm++;
	}
	lps = lpwm->lps;
	return lps;
}

// LOWORD
WMSTR	MActLst2[] = {
	{ HTBORDER,      "In border" },
	{ HTBOTTOM,      "Lower horiz. border" },
	{ HTBOTTOMLEFT,  "Lower-left corner" },
	{ HTBOTTOMRIGHT, "Lower-right corner" },
	{ HTCAPTION,     "In title bar" },
	{ HTCLIENT,      "In client area" },
	{ HTCLOSE,       "In close button" },
	{ HTERROR,       "Screen background or divide" },
	{ HTGROWBOX,     "In size box" },
	{ HTHELP,        "In Help button" },
	{ HTHSCROLL,     "In horiz. scroll" },
	{ HTLEFT,        "In left border" },
	{ HTMENU,        "In menu" },
	{ HTMAXBUTTON,   "In Max. button" },
	{ HTMINBUTTON,   "In Min. button" },
	{ HTNOWHERE,     "Screen background or divide" },
	{ HTREDUCE,      "In Min. button" },
	{ HTRIGHT,       "In right border" },
	{ HTSIZE,        "In size box" },
	{ HTSYSMENU,     "In System menu" },
	{ HTTOP,         "Upper horiz. border" },
	{ HTTOPLEFT,     "Upper-left corner" },
	{ HTTOPRIGHT,    "Upper right corner" },
	{ HTTRANSPARENT, "In covered window" },
	{ HTVSCROLL,     "In vertical scroll" },
	{ HTZOOM,        "In Max. button" },
	{ (UINT)-1,      "HIT Unknown" }
};

LPSTR	GetHTMsg( UINT uMsg )
{
	LPSTR	lps;
	LPWMSTR	lpwm;

	lpwm = &MActLst2[0];
	while( lpwm->val != (UINT)-1 )
	{
		if( lpwm->val == uMsg )
			break;
		lpwm++;
	}
	lps = lpwm->lps;
	return lps;
}

//			( lpd = GetDiagBuf() ) )
#define		bExcluded(a)\
	( ( a == WM_TIMER ) ||\
	( a == WM_ENTERIDLE ) ||\
	( a == WM_GETTEXT ) ||\
	( a == WM_SETTEXT ) ||\
	( a == WM_SETCURSOR ) ||\
	( a == WM_NCHITTEST ) ||\
	( a == WM_MOUSEMOVE ) ||\
	( a == WM_NCMOUSEMOVE ) ||\
	( a == WM_NCACTIVATE ) ||\
	( a == WM_WINDOWPOSCHANGING ) ||\
	( a == WM_WINDOWPOSCHANGED ) ||\
	( a == WM_CANCELMODE ) ||\
	( a == WM_ENABLE     ) )

BOOL	bNotExcluded( UINT a )
{
	BOOL	flg = TRUE;
	if( bExcluded( a ) )
		flg = FALSE;
	return flg;
}

// ===============================================================
// Interface
//
// ===============================================================
#define		MXREGS			10
#define		MXCREG			256

HANDLE	hNxtHand = (HANDLE)0x8001;
DWORD	dwRegCnt = 0;

typedef void  (*OUTMSG) (LPSTR);

typedef struct {
	HANDLE	h_hWnd;
	LPSTR	h_lpNm;
}CREG;
typedef struct {
	HANDLE	hm_Hand;
	LPSTR	hm_lpHdr;
	UINT	hm_uiMainLast;
	HWND	hm_hMainLast;
	WPARAM	hm_wpMainLast;
	LPARAM	hm_lpMainLast;
	UINT	hm_uiMainCnt;
	OUTMSG	hm_pOutMsg;
	int		hm_iFlag;
	int		hm_iCCnt;
	CREG	hm_sCRegs[MXCREG];
}REGLST;
typedef	REGLST FAR * LPREGLST;

#define	NULPRL(a)			memset(a,0,sizeof(REGLST))
#define	MOVEPRL(a,b)		memcpy(a,b,sizeof(REGLST))

REGLST		sRegLst[MXREGS] = {0};
LPREGLST	lpActReg;
TCHAR		szWmBuf[1024];

#define	uiMainLast		lpActReg->hm_uiMainLast
#define	hMainLast		lpActReg->hm_hMainLast
#define	wpMainLast		lpActReg->hm_wpMainLast
#define	lpMainLast		lpActReg->hm_lpMainLast
#define	uiMainCnt		lpActReg->hm_uiMainCnt

HANDLE	RegWMDiags( LPSTR lphdr, OUTMSG pOut )
{
	HANDLE		rHand = (HANDLE)0;
	DWORD		dwi;
	LPREGLST	lprl;

	if( ( lphdr ) &&
		( *lphdr ) &&
		( dwRegCnt < MXREGS ) &&
		( pOut ) )
	{
		lprl = &sRegLst[dwRegCnt];
		NULPRL(lprl);
		rHand = lprl->hm_Hand = hNxtHand;
		lprl->hm_lpHdr      = lphdr;
		//lprl->hm_uiMainLast = 0;
		//lprl->hm_hMainLast  = 0;
		//lprl->hm_wpMainLast = 0;
		//lprl->hm_lpMainLast = 0;
		//lprl->hm_uiMainCnt  = 0;
		lprl->hm_pOutMsg    = pOut;
		dwi = PtrToUint(hNxtHand);
		dwi++;
		hNxtHand = UIntToPtr(dwi);
		dwRegCnt++;
	}
	return rHand;
}

HANDLE	RegWMDiagsEx( LPREGSTR lprs )
{
	HANDLE		rHand = 0;
	LPSTR		lphdr;
	OUTMSG		pOut;
	LPREGLST	lprl;
	if( ( lprs ) &&
		( lphdr = lprs->rs_lpName ) &&
		( pOut  = lprs->rs_vpOut  ) )
	{
		DWORD	dwi;
		if( dwRegCnt )
		{
			for( dwi = 0; dwi < dwRegCnt; dwi++ )
			{
				lprl = &sRegLst[dwi];
				//rHand = lprl->hm_Hand;
				if( lprl->hm_lpHdr == lphdr )
				{
					rHand = lprl->hm_Hand;
					break;
				}
			}
		}
		if( rHand == 0 )
		{
			rHand = RegWMDiags( lphdr, pOut );
		}
		if( ( rHand == 0    ) ||
			( dwRegCnt == 0 ) )
		{
			rHand = 0;
			return rHand;
		}
		for( dwi = 0; dwi < dwRegCnt; dwi++ )
		{
			lprl = &sRegLst[dwi];
			if( lprl->hm_Hand == rHand )
				break;
//		lprl->hm_uiMainLast = 0;
//		lprl->hm_hMainLast  = 0;
//		lprl->hm_wpMainLast = 0;
//		lprl->hm_lpMainLast = 0;
//		lprl->hm_uiMainCnt  = 0;
//		lprl->hm_pOutMsg    = pOut;
		}
		if( dwi < dwRegCnt )
		{


		}
		else
		{

		}
	}
	return rHand;
}

BOOL	DeRegWMDiags( HANDLE hand )
{
	BOOL		bRet = FALSE;
	DWORD		dwi;
	LPREGLST	lprl, lprl2;

	if( dwRegCnt )
	{
		lprl = &sRegLst[0];
		for( dwi = 0; dwi < dwRegCnt; dwi++ )
		{
			if( lprl->hm_Hand == hand )
			{
				// kill this entry
				//lprl->hm_Hand       = 0;
				//lprl->hm_lpHdr      = 0;
				//lprl->hm_uiMainLast = 0;
				//lprl->hm_hMainLast  = 0;
				//lprl->hm_wpMainLast = 0;
				//lprl->hm_lpMainLast = 0;
				//lprl->hm_uiMainCnt  = 0;
				//lprl->hm_pOutMsg    = 0;
				NULPRL(lprl);
				dwi++;
				// if there are more,
				// move them all down one
				for( ; dwi < dwRegCnt; dwi++ )
				{
					lprl2 = lprl;
					lprl++;
					// would this be faster by memcpy???
					//lprl2->hm_Hand       = lprl->hm_Hand;
					//lprl2->hm_lpHdr      = lprl->hm_lpHdr;
					//lprl2->hm_uiMainLast = lprl->hm_uiMainLast;
					//lprl2->hm_hMainLast  = lprl->hm_hMainLast;
					//lprl2->hm_wpMainLast = lprl->hm_wpMainLast;
					//lprl2->hm_lpMainLast = lprl->hm_lpMainLast;
					//lprl2->hm_uiMainCnt  = lprl->hm_uiMainCnt;
					//lprl2->hm_pOutMsg    = lprl->hm_pOutMsg;
					// yes sir eee
					MOVEPRL(lprl2,lprl);
					NULPRL(lprl);
				}
				dwRegCnt--;
				bRet = TRUE;
				break;
			}
			lprl++;
		}
	}
	return bRet;
}


LPSTR	GetWMHeader( HANDLE hand )
{
	LPSTR		lps = 0;
	DWORD		dwi;
	LPREGLST	lprl;

	if( dwRegCnt )
	{
		lprl = &sRegLst[0];
		for( dwi = 0; dwi < dwRegCnt; dwi++ )
		{
			if( lprl->hm_Hand == hand )
			{
				lps = lprl->hm_lpHdr;	// get the registered NAME
				lpActReg = lprl;
				break;
			}
			lprl++;
		}
	}

	return lps;
}

// ================================================================
// void	OutWM( HANDLE hand, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
//
// INPUT:	HANDLE hand - From RegWMDiags
//			HWND hWnd   - handle of window getting message
//			UINT uMsg   - The message
//			WPARAM wParam - First parameter
//			LPARAM lParam - Second parameter
//
// PURPOSE: To construct a diagnostic message, and output it to
//			a file.
//
// OUTPUT: Message constructed and passed to OUPUT service got
//			from the "register" call.
//
// ================================================================
void	OutWM( HANDLE hand, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	LPSTR		lphd;
	LPREGLST	lprl;
	LPSTR		lpn = 0;
	int			i, iCnt;
	LPSTR		lpwm;

	if( ( lphd = GetWMHeader( hand ) ) &&
		( lprl = lpActReg ) &&
		( lpActReg->hm_pOutMsg ) )
	{
		LPSTR	lpd;
		if( bNotExcluded( uMsg ) )
		{
			if( ( uiMainLast == uMsg ) &&
				( hMainLast == hWnd  ) )
			{
				uiMainCnt++;
			}
			else
			{
//				lpd = GetDiagBuf();
				lpd = &szWmBuf[0];
				if( uiMainCnt )
				{
					lpwm = GetWMStg( hMainLast,
									uiMainLast,
									wpMainLast,
									lpMainLast );
					iCnt = lprl->hm_iCCnt;
					for( i = 0; i < iCnt; i++ )
					{
						if( lprl->hm_sCRegs[i].h_hWnd == hMainLast )
						{
							lpn = lprl->hm_sCRegs[i].h_lpNm;
							break;
						}
					}
				}

				if( uiMainCnt > 1 )
				{
					if( lpn )
					{
						wsprintf( lpd, "%s %s (%s) for %u\r\n",
							lphd,
							lpwm,
							lpn,
							uiMainCnt );
					}
					else
					{
						wsprintf( lpd, "%s %s (H=0x%x) for %u\r\n",
							lphd,
							lpwm,
							hMainLast,
							uiMainCnt );
					}

					//DIAG1( lpd );
					//( *lpActReg->hm_pOutMsg ) ( lpd );
				}
				else if( uiMainCnt )
				{
					if( lpn )
					{
						wsprintf( lpd, "%s %s (%s) +\r\n",
							lphd,
							lpwm,
							lpn );
					}
					else
					{
						wsprintf( lpd, "%s %s (H=0x%x) +\r\n",
							lphd,
							lpwm,
							hMainLast );
					}
					//DIAG1( lpd );
					//( *lpActReg->hm_pOutMsg ) ( lpd );
				}
				if( uiMainCnt )
				{
					//DIAG1( lpd );
					( *lpActReg->hm_pOutMsg ) ( lpd );

				}

				lpn        = 0;
				// grab the new item
				hMainLast  = hWnd;
				uiMainLast = uMsg;
				wpMainLast = wParam;
				lpMainLast = lParam;
				uiMainCnt = 0;
				lpwm = GetWMStg( hWnd,
								uMsg,
								wParam,
								lParam );

				iCnt = lprl->hm_iCCnt;
				for( i = 0; i < iCnt; i++ )
				{
					if( lprl->hm_sCRegs[i].h_hWnd == hMainLast )
					{
						lpn = lprl->hm_sCRegs[i].h_lpNm;
						break;
					}
				}
				if( lpn )
				{
					wsprintf( lpd, "%s %s (%s)\r\n",
						lphd,
						lpwm,
						lpn );
				}
				else
				{
					wsprintf( lpd, "%s %s (H=0x%x)\r\n",
						lphd,
						lpwm,
						hWnd );
				}

				//DIAG1( lpd );
				( *lpActReg->hm_pOutMsg ) ( lpd );

			}
		}

		lpActReg = 0;
	}
}

/*	=======================================================	
	void	RegWMWindow( HANDLE hand, HWND hwnd, LPSTR lpn )

	PURPOSE: Register a NAME to output, instead of
		just HEX (H=0x4ff5)

	ADDED:	Nov, 1999

	=======================================================	*/
void	RegWMWindow( HANDLE hand, HWND hwnd, LPSTR lpn )
{
	LPSTR		lphd;
	LPREGLST	lprl;
	int			i, iCnt;

	lpActReg = 0;
	if( ( lphd = GetWMHeader( hand ) ) &&
		( lprl = lpActReg            ) )
	{
		if( iCnt = lprl->hm_iCCnt )
		{
			for( i = 0; i < iCnt; i++ )
			{
				if( lprl->hm_sCRegs[i].h_hWnd == hwnd )
				{
//	int		hm_iCCnt;
//	CREG	hm_sCRegs[MXCREG];
//typedef struct {
//	HANDLE	h_hWnd;
//	LPSTR	h_lpNm;
//}CREG;
					lprl->hm_sCRegs[i].h_lpNm = lpn;
					return;
					break;
				}
			}
		}
		if( lprl->hm_iCCnt < MXCREG )
		{
			i = lprl->hm_iCCnt;
			lprl->hm_sCRegs[i].h_hWnd = hwnd;
			lprl->hm_sCRegs[i].h_lpNm = lpn;
			i++;
			lprl->hm_iCCnt = i;
		}
	}
	lpActReg = 0;
}

// **************************************************
// HEAVY Window diagnostic stuff
// =============================
typedef void (*MGHND) (UINT, LPTSTR, WPARAM, LPARAM);

typedef struct {
   UINT     w_uiMsg;
   LPTSTR   w_lpStg;
   MGHND    w_pMgHnd;
}WMSTR4, * PWMSTR4;

// Edit Control Notification Messages
// The user makes editing requests by using the keyboard and mouse.
// The system sends each request to the edit control's parent window
// in the form of a WM_COMMAND message. The message includes the
// edit control identifier in the low-order word of the wParam parameter,
// the handle of the edit control in the lParam parameter,
// and an edit control notification message corresponding to
// the user's action in the high-order word of the wParam parameter. 
// The following table lists each edit control notification message
///*
// * Edit Control Notification Codes
// */
// #define EN_SETFOCUS         0x0100
// #define EN_KILLFOCUS        0x0200
// #define EN_CHANGE           0x0300
// #define EN_UPDATE           0x0400
// #define EN_ERRSPACE         0x0500
// #define EN_MAXTEXT          0x0501
// #define EN_HSCROLL          0x0601
// #define EN_VSCROLL          0x0602
WMSTR4 sEdNote[] = {
   { EN_CHANGE,    "EN_CHANGE", 0 }, // The user has modified text in an edit control.
      // The system updates the display before sending this message (unlike EN_UPDATE).
   { EN_ERRSPACE,  "EN_ERRSPACE", 0 },  // The edit control cannot allocate enough
      // memory to meet a specific request.
   { EN_HSCROLL,   "EN_HSCROLL", 0 }, // The user has clicked the edit control's
      // horizontal scroll bar. The system sends this message before updating the screen.
   { EN_KILLFOCUS, "EN_KILLFOCUS", 0 }, // The user has selected another control.
   { EN_MAXTEXT,   "EN_MAXTEXT", 0 }, // While inserting text,
      // the user has exceeded the specified number of characters for the edit control.
      // Insertion has been truncated. This message is also sent either
      // when an edit control does not have the ES_AUTOHSCROLL style and
      // the number of characters to be inserted exceeds the width of the
      // edit control or when an edit control does not have the ES_AUTOVSCROLL style
      // and the total number of lines to be inserted exceeds the height of the edit control.
   { EN_SETFOCUS,  "EN_SETFOCUS", 0 },  // The user has selected this edit control.
   { EN_UPDATE,    "EN_UPDATE", 0 },   // The user has altered the text in the edit control
      // and the system is about to display the new text. The system sends this message
      // after formatting the text, but before displaying it, so that
      // the application can resize the edit control window.
   { EN_VSCROLL,   "EN_VSCROLL", 0 }, // The user has clicked the edit control's
      // vertical scroll bar or has scrolled the mouse wheel over the edit control.
      // The system sends this message before updating the screen.
   { 0,           0,           0 }
};
TCHAR szNL[32];
LPTSTR   DBGGetEdNote( UINT uiNote )
{
   LPTSTR   lpret, lpstg;
   PWMSTR4  pwm = &sEdNote[0];
   while( lpstg = pwm->w_lpStg )
   {
      if( pwm->w_uiMsg == uiNote )
      {
         lpret = lpstg;
         break;
      }
      pwm++;
   }
   if( !lpstg )
   {
      lpret = &szNL[0];
      wsprintf(lpret, "UNKNOWN(%#x)", uiNote);
   }
   return lpret;
}

// **************************************************************
#else	// !( SHOWMSG | SHOWWM )
// **************************************************************

// ****************************************************************
#pragma message( "ADVICE: Diagnostic Window messages DISABLED!" )
// ****************************************************************

// **************************************************************
#endif	// SHOWMSG | SHOWWM

//#endif	// !NDEBUG
// eof - WMDiag.c
