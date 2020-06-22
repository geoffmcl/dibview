
/* ==========================================================
 *
 *  DvFile.c
 *
 * Functions:
 *		BOOL     GetFileName       (LPSTR, WORD);
 *
 * ========================================================== */
#include	"dv.h"		// Single INCLUSIVE include ...
#undef	IMGFIXED		// Problem with PAINTING the image
#undef	DBGSACMD	// Out some DEBUG stuff
#define  ADDREVIEW2  // another try

// FIX980517 - Now this should ALREADY be included!!!
//#if defined( WIN32 )
//IFF a constant reminder is needed, uncomment the following pragma...
//#pragma message( "DLL: WIN32 Port ..." )
//#define EXPORT32 __declspec(dllexport)
//#if (WINVER >= 0x0400)
//#pragma message( "WINVER greater than or equal to 0x0400 ..." )
//#else
//#pragma message( "WINVER less than 0x0400 ..." )
//#endif
//#else
//#pragma message( "DLL: 16-Bit Port ..." )
//#   define EXPORT32
//#endif	// WIN32 y/n

//#define	TRYADDD

// NOTE: The CUSTOM Dialog AND OFNHook has NOT yet been doen for WIN32
// if UseExplorer
#define	FNOpenFlag1		OFN_EXPLORER | OFN_HIDEREADONLY
#define FNSaveFlag1		OFN_EXPLORER | OFN_HIDEREADONLY |\
	OFN_OVERWRITEPROMPT
// else use old Win 3.1 style
#define	FNOpenFlag2		OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY
#define FNSaveFlag2		OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |\
	OFN_OVERWRITEPROMPT

// external services
extern	int		GetDibInfo2( LPINFOSTRUCT );
extern	int		InStr( LPSTR, LPSTR );
extern	void	AddToFileList( LPSTR );
//extern	void	DVGetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );

extern	BOOL    GetRootDir( LPSTR lpf );

extern  HLOCAL  GMLocalAlloc( UINT, size_t );
extern  HLOCAL  GMLocalFree( HLOCAL );
extern  BOOL    GMLocalOK( HLOCAL );    // Check memory exist

extern	BOOL	CenterWindow(HWND hwndChild, HWND hwndParent);
extern	BOOL	FindNewName( LPSTR );
extern	BOOL	WriteBMP( LPSTR lpOutReq, HANDLE hOutDIB, BOOL fErrDlg );
extern	HANDLE	CopyDIB( HANDLE hDIB );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );

// Local/Global variables
void	AddBMPExt( LPSTR, DWORD );
void	GetCurWDir( LPSTR );

// external data
extern	LPSTR	pRDefLst[MXELST];
extern	LPSTR	pRDefExt;	// = &szBmp[0];	say, BMP extent ... 
extern	LPSTR	pszFilter;	//  = &szFStrJ[0]; - File Filter String
extern	WORD	InIndex;
extern	BOOL	fChgInInd;
extern	char    szDFileName[_MAX_PATH];         // Filename of DIB
//extern	BOOL	fChgOpen;
//extern	BOOL	fChgSave;
// = "%s (%ld x %ld x %d BPP)"; MDI Window Title
extern	LPTSTR	pRDibTit;

#if   (defined(ADDREVIEW) || defined(ADDREVIEW2))
#ifdef   WIN32
UINT_PTR CALLBACK EXPOPENHOOKPROC(
  HWND hdlg,      // handle to child dialog box
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
);
#else // WIN32
BOOL MLIBCALL EXPOPENHOOKPROC( HWND, UINT, WPARAM, LPARAM );
#endif	// WIN32 y/n
#endif   // ADDREVIEW

FARPROC	   fpDlgFunc;	// far proc to dlg function
LPSTR		   pADir;	   /* Currently ACTIVE Directory ... */
char    szTitle[64];         // Dialog Box title
char    szTemplate[64];      // Dialog Box template
//char    szFileTitle[260];    // Title
WORD	gwOutIndex = 0;
DWORD	gdwbiStyle;		// Type of Dib - RGB, RLE4, RLE8, PM
WORD	wbiBits;		// bits per pixel
BOOL	fChgOutInd = FALSE;
BOOL	fUseExplorer = TRUE;
BOOL	fChgExplorer = FALSE;

INFOSTRUCT ExpInfo = { 0 };

BOOL gbAspect = TRUE;
BOOL gbKeepAspect = TRUE;
BOOL gbChgKeepAsp = FALSE;

char *pExtErr;
char szComErr[] = "Common Error";
char szUnkErr[] = "Unknown Error";

/* ************************************************************************

  Function:  GetFileName( LPSTR lpFn, WORD Type )

   Purpose:  Prompts user for a filename through the use of a Windows 3.1
             FileOpen common dialog box.

   NOTE: Dialog template "FILEOPEN" NOT_USED

   Returns:  TRUE if a filename is selected.
             FALSE if no filename is selected.

  Comments:  Filename is put into the string passed to the routine.
             If a filename is not selected, NULL is returned.

   History:   Date      Author      Reason

             6/1/91    Created
             6/27/91   Changed OPENFILENAME structure to
                       support customized common dialog box
				31 Mch 96	Separate Open/Read directory from Write/Save
                        directory to ALLOW Read from one place, and
                        Write to another for several files. Geoff.

   Handles: 1. OpenDIBFile from Do_IDM_OPEN from IDM_OPEN from menu
            2. etc

************************************************************************ */
//The OPENFILENAME structure contains information that the GetOpenFileName and
//GetSaveFileName functions use to initialize an Open or Save As
//common dialog box. After the user closes the dialog box, the
//system returns information about the user's selection in this
//structure. 
//typedef struct tagOFN { // ofn  
//    DWORD         lStructSize; 
//    HWND          hwndOwner; 
//    HINSTANCE     hInstance; 
//    LPCTSTR       lpstrFilter; 
//    LPTSTR        lpstrCustomFilter; 
//    DWORD         nMaxCustFilter; 
//    DWORD         nFilterIndex; 
//    LPTSTR        lpstrFile; 
//    DWORD         nMaxFile; 
//    LPTSTR        lpstrFileTitle; 
//    DWORD         nMaxFileTitle; 
//    LPCTSTR       lpstrInitialDir; 
//    LPCTSTR       lpstrTitle; 
//    DWORD         Flags; 
//    WORD          nFileOffset; 
//    WORD          nFileExtension; 
//    LPCTSTR       lpstrDefExt; 
//    DWORD         lCustData; 
//    LPOFNHOOKPROC lpfnHook; 
//    LPCTSTR       lpTemplateName; 
//} OPENFILENAME; 
//Members
/* =============================
typedef struct tagOFN { 
  DWORD         lStructSize; 
  HWND          hwndOwner; 
  HINSTANCE     hInstance; 
  LPCTSTR       lpstrFilter; 
  LPTSTR        lpstrCustomFilter; 
  DWORD         nMaxCustFilter; 
  DWORD         nFilterIndex; 
  LPTSTR        lpstrFile; 
  DWORD         nMaxFile; 
  LPTSTR        lpstrFileTitle; 
  DWORD         nMaxFileTitle; 
  LPCTSTR       lpstrInitialDir; 
  LPCTSTR       lpstrTitle; 
  DWORD         Flags; 
  WORD          nFileOffset; 
  WORD          nFileExtension; 
  LPCTSTR       lpstrDefExt; 
  LPARAM        lCustData; 
  LPOFNHOOKPROC lpfnHook; 
  LPCTSTR       lpTemplateName; 
#if (_WIN32_WINNT >= 0x0500)
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} OPENFILENAME, *LPOPENFILENAME;
   ============================= */
//lStructSize
//Specifies the length, in bytes, of the structure. 

//hwndOwner
//Identifies the window that owns the dialog box. This member can
//be any valid window handle, or it can be NULL if the dialog box
//has no owner. 

//hInstance
//If the OFN_ENABLETEMPLATEHANDLE flag is set in the Flags member,
//hInstance is the handle of a memory object containing a dialog
//box template. If the OFN_ENABLETEMPLATE flag is set, hInstance
//identifies a module that contains a dialog box template named by
//the lpTemplateName member. If neither flag is set, this member
//is ignored.
//If the OFN_EXPLORER flag is set, the system uses the specified
//template to create a dialog box that is a child of the default
//Explorer-style dialog box. If the OFN_EXPLORER flag is not set,
//the system uses the template to create an old-style dialog box
//that replaces the default dialog box.

//lpstrFilter
//Pointer to a buffer containing pairs of null-terminated filter
//strings. The last string in the buffer must be terminated by two
//NULL characters. 
//The first string in each pair is a display string that describes
//the filter (for example, "Text Files"), and the second string
//specifies the filter pattern (for example, "*.TXT"). To specify
//multiple filter patterns for a single display string, use a
//semicolon to separate the patterns (for example,
//"*.TXT;*.DOC;*.BAK"). A pattern string can be a combination of
//valid filename characters and the asterisk (*) wildcard
//character. Do not include spaces in the pattern string.
//The operating system does not change the order of the filters.
//It displays them in the File Types combo box in the order
//specified in lpstrFilter.
//If lpstrFilter is NULL, the dialog box does not display any
//filters. 

//lpstrCustomFilter
//Pointer to a static buffer that contains a pair of
//null-terminated filter strings for preserving the filter pattern
//chosen by the user. The first string is your display string that
//describes the custom filter, and the second string is the filter
//pattern selected by the user. The first time your application
//creates the dialog box, you specify the first string, which can
//be any nonempty string. When the user selects a file, the dialog
//box copies the current filter pattern to the second string. The
//preserved filter pattern can be one of the patterns specified in
//the lpstrFilter buffer, or it can be a filter pattern typed by
//the user. The system uses the strings to initialize the
//user-defined file filter the next time the dialog box is
//created. If the nFilterIndex member is zero, the dialog box uses
//the custom filter. 
//If this member is NULL, the dialog box does not preserve
//user-defined filter patterns.
//If this member is not NULL, the value of the nMaxCustFilter
//member must specify the size, in bytes (ANSI version) or
//characters (Unicode version), of the lpstrCustomFilter buffer. 

//nMaxCustFilter
//Specifies the size, in bytes or characters, of the buffer
//identified by lpstrCustomFilter. This buffer should be at least
//40 characters long. This member is ignored if lpstrCustomFilter
//is NULL or points to a NULL string. 

//nFilterIndex
//Specifies the index of the currently selected filter in the File
//Types control. The buffer pointed to by lpstrFilter contains
//pairs of strings that define the filters. The first pair of
//strings has an index value of 1, the second pair 2, and so on.
//An index of zero indicates the custom filter specified by
//lpstrCustomFilter. You can specify an index on input to indicate
//the initial filter description and filter pattern for the dialog
//box. When the user selects a file, nFilterIndex returns the
//index of the currently displayed filter. 
//If nFilterIndex is zero and lpstrCustomFilter is NULL, the
//system uses the first filter in the lpstrFilter buffer. If all
//three members are zero or NULL, the system does not use any
//filters and does not show any files in the file list control of
//the dialog box. 

//lpstrFile
//Pointer to a buffer that contains a filename used to initialize
//the File Name edit control. The first character of this buffer
//must be NULL if initialization is not necessary. When the
//GetOpenFileName or GetSaveFileName function returns
//successfully, this buffer contains the drive designator, path,
//filename, and extension of the selected file. 
//If the OFN_ALLOWMULTISELECT flag is set and the user selects
//multiple files, the buffer contains the current directory
//followed by the filenames of the selected files. For
//Explorer-style dialog boxes, the directory and filename strings
//are NULL separated, with an extra NULL character after the last
//filename. For old-style dialog boxes, the strings are space
//separated and the function uses short filenames for filenames
//with spaces. You can use the FindFirstFile function to convert
//between long and short filenames.
//If the buffer is too small, the function returns FALSE and the
//CommDlgExtendedError function returns FNERR_BUFFERTOOSMALL. In
//this case, the first two bytes of the lpstrFile buffer contain
//the required size, in bytes or characters.

//nMaxFile
//Specifies the size, in bytes (ANSI version) or characters
//(Unicode version), of the buffer pointed to by lpstrFile. The
//GetOpenFileName and GetSaveFileName functions return FALSE if
//the buffer is too small to contain the file information. The
//buffer should be at least 256 characters long. 

//lpstrFileTitle
//Pointer to a buffer that receives the filename and extension
//(without path information) of the selected file. This member can
//be NULL. 

//nMaxFileTitle
//Specifies the size, in bytes (ANSI version) or characters
//(Unicode version), of the buffer pointed to by lpstrFileTitle.
//This member is ignored if lpstrFileTitle is NULL. 

//lpstrInitialDir
//Pointer to a string that specifies the initial file directory.
//If this member is NULL, the system uses the current directory as
//the initial directory. 

//lpstrTitle
//Pointer to a string to be placed in the title bar of the dialog
//box. If this member is NULL, the system uses the default title
//(that is, "Save As" or "Open"). 

//Flags
//A set of bit flags you can use to initialize the dialog box.
//When the dialog box returns, it sets these flags to indicate the
//user's input. This member can be a combination of the following
//flags: 
//Flag	Meaning
//OFN_ALLOWMULTISELECT	
//	Specifies that the File Name list box allows multiple
//selections. If you also set the OFN_EXPLORER flag, the dialog
//box uses the Explorer-style user interface; otherwise, it uses
//the old-style user interface. If the user selects more than one
//file, the lpstrFile buffer returns the path to the current
//directory followed by the filenames of the selected files. The
//nFileOffset member is the offset to the first filename, and the
//nFileExtension member is not used. For Explorer-style dialog
//boxes, the directory and filename strings are NULL separated,
//with an extra NULL character after the last filename. This
//format enables the Explorer-style dialogs to return long
//filenames that include spaces. For old-style dialog boxes, the
//directory and filename strings are separated by spaces and the
//function uses short filenames for filenames with spaces. You can
//use the FindFirstFile function to convert between long and short
//filenames. If you specify a custom template for an old-style
//dialog box, the definition of the File Name list box must
//contain the LBS_EXTENDEDSEL value. 
//OFN_CREATEPROMPT	
//	If the user specifies a file that does not exist, this flag
//causes the dialog box to prompt the user for permission to
//create the file. If the user chooses to create the file, the
//dialog box closes and the function returns the specified name;
//otherwise, the dialog box remains open. 
//OFN_ENABLEHOOK	
//	Enables the hook function specified in the lpfnHook member. 
//OFN_ENABLETEMPLATE	
//	Indicates that the lpTemplateName member points to the name of
//a dialog template resource in the module identified by the
//hInstance member.If the OFN_EXPLORER flag is set, the system
//uses the specified template to create a dialog box that is a
//child of the default Explorer-style dialog box. If the
//OFN_EXPLORER flag is not set, the system uses the template to
//create an old-style dialog box that replaces the default dialog
//box.
//OFN_ENABLETEMPLATEHANDLE	
//	Indicates that the hInstance member identifies a data block
//that contains a preloaded dialog box template. The system
//ignores the lpTemplateName if this flag is specified.If the
//OFN_EXPLORER flag is set, the system uses the specified template
//to create a dialog box that is a child of the default
//Explorer-style dialog box. If the OFN_EXPLORER flag is not set,
//the system uses the template to create an old-style dialog box
//that replaces the default dialog box.
//OFN_EXPLORER	
//	Indicates that any customizations made to the Open or Save As
//dialog box use the new Explorer-style customization methods. For
//more information, see the "Explorer-Style Hook Procedures" and
//"Explorer-Style Custom Templates" sections of the Common Dialog
//Box Library overview.By default, the Open and Save As dialog
//boxes use the Explorer-style user interface regardless of
//whether this flag is set. This flag is necessary only if you
//provide a hook procedure or custom template, or set the
//OFN_ALLOWMULTISELECT flag. If you want the old-style user
//interface, omit the OFN_EXPLORER flag and provide a replacement
//old-style template or hook procedure. If you want the old style
//but do not need a custom template or hook procedure, simply
//provide a hook procedure that always returns FALSE.
//OFN_EXTENSIONDIFFERENT	
//	Specifies that the user typed a filename extension that differs
//from the extension specified by lpstrDefExt. The function does
//not use this flag if lpstrDefExt is NULL.
//OFN_FILEMUSTEXIST	
//	Specifies that the user can type only names of existing files
//in the File Name entry field. If this flag is specified and the
//user enters an invalid name, the dialog box procedure displays a
//warning in a message box. If this flag is specified, the
//OFN_PATHMUSTEXIST flag is also used.
//OFN_HIDEREADONLY	
//	Hides the Read Only check box.
//OFN_LONGNAMES	
//	For old-style dialog boxes, this flag causes the dialog box to
//use long filenames. If this flag is not specified, or if the
//OFN_ALLOWMULTISELECT flag is also set, old-style dialog boxes
//use short filenames (8.3 format) for filenames with spaces.
//Explorer-style dialog boxes ignore this flag and always display
//long filenames.
//OFN_NOCHANGEDIR	
//	Restores the current directory to its original value if the
//user changed the directory while searching for files.
//OFN_NODEREFERENCELINKS	
//	Directs the dialog box to return the path and filename of the
//selected shortcut (.LNK) file. If this value is not given, the
//dialog box returns the path and filename of the file referenced
//by the shortcut
//OFN_NOLONGNAMES	
//	For old-style dialog boxes, this flag causes the dialog box to
//use short filenames (8.3 format). Explorer-style dialog boxes
//ignore this flag and always display long filenames.
//OFN_NONETWORKBUTTON	
//	Hides and disables the Network button.
//OFN_NOREADONLYRETURN	
//	Specifies that the returned file does not have the Read Only
//check box checked and is not in a write-protected directory.
//OFN_NOTESTFILECREATE	
//	Specifies that the file is not created before the dialog box is
//closed. This flag should be specified if the application saves
//the file on a create-nonmodify network sharepoint. When an
//application specifies this flag, the library does not check for
//write protection, a full disk, an open drive door, or network
//protection. Applications using this flag must perform file
//operations carefully, because a file cannot be reopened once it
//is closed.
//OFN_NOVALIDATE	
//	Specifies that the common dialog boxes allow invalid characters
//in the returned filename. Typically, the calling application
//uses a hook procedure that checks the filename by using the
//FILEOKSTRING message. If the text box in the edit control is
//empty or contains nothing but spaces, the lists of files and
//directories are updated. If the text box in the edit control
//contains anything else, nFileOffset and nFileExtension are set
//to values generated by parsing the text. No default extension is
//added to the text, nor is text copied to the buffer specified by

//lpstrFileTitle.
//	If the value specified by nFileOffset is less than zero, the
//filename is invalid. Otherwise, the filename is valid, and
//nFileExtension and nFileOffset can be used as if the
//OFN_NOVALIDATE flag had not been specified.
//OFN_OVERWRITEPROMPT	
//	Causes the Save As dialog box to generate a message box if the
//selected file already exists. The user must confirm whether to
//overwrite the file.
//OFN_PATHMUSTEXIST	
//	Specifies that the user can type only valid paths and
//filenames. If this flag is used and the user types an invalid
//path and filename in the File Name entry field, the dialog box
//function displays a warning in a message box.
//OFN_READONLY	
//	Causes the Read Only check box to be checked initially when the
//dialog box is created. This flag indicates the state of the Read
//Only check box when the dialog box is closed.
//OFN_SHAREAWARE	
//	Specifies that if a call to the OpenFile function fails because
//of a network sharing violation, the error is ignored and the
//dialog box returns the selected filename. If this flag is not
//set, the dialog box notifies your hook procedure when a network
//sharing violation occurs for the filename specified by the user.
//If you set the OFN_EXPLORER flag, the dialog box sends the
//CDN_SHAREVIOLATION message to the hook procedure. If you do not
//set OFN_EXPLORER, the dialog box sends the SHAREVISTRING
//registered message to the hook procedure. 
//OFN_SHOWHELP	
//	Causes the dialog box to display the Help button. The hwndOwner
//member must specify the window to receive the HELPMSGSTRING
//registered messages that the dialog box sends when the user
//clicks the Help button.An Explorer-style dialog box sends a
//CDN_HELP notification message to your hook procedure when the
//user clicks the Help button. 

//nFileOffset
//Specifies a zero-based offset from the beginning of the path to
//the filename in the string pointed to by lpstrFile. For example,
//if lpstrFile points to the following string,
//"c:\dir1\dir2\file.ext", this member contains the value 13 to
//indicate the offset of the "file.ext" string. 
//nFileExtension
//Specifies a zero-based offset from the beginning of the path to
//the filename extension in the string pointed to by lpstrFile.
//For example, if lpstrFile points to the following string,
//"c:\dir1\dir2\file.ext", this member contains the value 18. If
//the user did not type an extension and lpstrDefExt is NULL, this
//member specifies an offset to the terminating null character. If
//the user typed "." as the last character in the filename, this
//member specifies zero. 

//lpstrDefExt
//Points to a buffer that contains the default extension.
//GetOpenFileName and GetSaveFileName append this extension to the
//filename if the user fails to type an extension. This string can
//be any length, but only the first three characters are appended.
//The string should not contain a period (.). If this member is
//NULL and the user fails to type an extension, no extension is
//appended. 

//lCustData
//Specifies application-defined data that the system passes to the
//hook procedure identified by the lpfnHook member. When the
//system sends the WM_INITDIALOG message to the hook procedure,
//the message's lParam parameter is a pointer to the OPENFILENAME
//structure specified when the dialog box was created. The hook
//procedure can use this pointer to get the lCustData value.

//lpfnHook
//Pointer to a hook procedure. This member is ignored unless the
//Flags member includes the OFN_ENABLEHOOK flag.
//If the OFN_EXPLORER flag is not set in the Flags member,
//lpfnHook is a pointer to an OFNHookProcOldStyle hook procedure
//that receives messages intended for the dialog box. The hook
//procedure returns FALSE to pass a message to the default dialog
//box procedure or TRUE to discard the message. 

//If OFN_EXPLORER is set, lpfnHook is a pointer to an OFNHookProc
//hook procedure. The hook procedure receives notification
//messages sent from the dialog box. The hook procedure also
//receives messages for any additional controls that you defined
//by specifying a child dialog template. The hook procedure does
//not receive messages intended for the standard controls of the
//default dialog box. 

//lpTemplateName
//Pointer to a null-terminated string that names a dialog template
//resource in the module identified by the hInstance member. For
//numbered dialog box resources, this can be a value returned by
//the MAKEINTRESOURCE macro. This member is ignored unless the
//OFN_ENABLETEMPLATE flag is set in the Flags member.
//If the OFN_EXPLORER flag is set, the system uses the specified
//template to create a dialog box that is a child of the default
//Explorer-style dialog box. If the OFN_EXPLORER flag is not set,
//the system uses the template to create an old-style dialog box
//that replaces the default dialog box.
//See Also
////GetOpenFileName, GetSaveFileName 

//You can customize an Explorer-style Open or Save As dialog box 
// by providing a hook procedure, a custom template, or both.
//If you provide a hook procedure for an
//Explorer-style dialog box, the system creates a dialog box that
//is a child of the default dialog box. The hook procedure acts as
//the dialog procedure for the child dialog box. This child dialog
//box is based on the custom template, or on a default template if
//none is provided. For more information, see Explorer-Style
//Custom Templates.To enable a hook procedure for an
//Explorer-style Open or Save As dialog box, use the OPENFILENAME
//structure when you create the dialog box. Set the OFN_ENABLEHOOK
//and OFN_EXPLORER flags in the Flags member and specify the
//address of an OFNHookProc hook procedure in the lpfnHook member.
//If you provide a hook procedure and omit the OFN_EXPLORER flag,
//you must use an OFNHookProcOldStyle hook procedure and you will
//get the old-style user-interface. For more information, see
//Customizing Old-Style Dialog Boxes. 

//To define additional controls for an Explorer-style Open or Save 
//As dialog box, use the 
//OPENFILENAME structure to specify a template for a child dialog
//box that contains the additional controls. If your child dialog
//template is a resource in an application or dynamic-link
//library, set the OFN_ENABLETEMPLATE flag in the Flags member and
//use the hInstance and lpTemplateName members of the structure to
//identify the module and resource name. If the template is
//already in memory, set the OFN_ENABLETEMPLATEHANDLE flag and use
//the hInstance member to identify the memory object that contains
//the template. When providing a child dialog template for an
//Explorer-style dialog box, you must also set the OFN_EXPLORER
//flag; otherwise, the system assumes you are providing a
//replacement template for an old-style dialog box. Typically, if
//you provide additional controls, you must also provide an
//Explorer-style hook procedure to process messages for the new
//controls.

//You can create your child dialog box template as you do any
//other template, except that you must specify the WS_CHILD and
//WS_CLIPSIBLINGS styles and should specify the DS_3DLOOK and
//DS_CONTROL styles. The system requires the WS_CHILD style
//because your template defines a child dialog of the default Open
//or Save As dialog box. The WS_CLIPSIBLINGS style ensures that
//the child dialog box does not paint over any of the controls in
//the default dialog box. The DS_3DLOOK style makes sure that the
//appearance of the controls in the child dialog box is consistent
//with the controls in the default dialog box. The DS_CONTROL
//style makes sure that the user can use the TAB and other
//navigation keys to move between all controls, default or custom,
//in the customized dialog box.

//To make room for the new controls, the system expands the
//default dialog box by the width and height of the custom dialog
//box. By default, all controls from the custom dialog box are
//positioned below the controls in the default dialog box.
//However, you can override this default positioning by including
//a static text control in your custom dialog box template and
//assigning it the control identifier value of stc32. (This value
//is defined in the DLG.H header file.) In this case, the system
//uses the control as the point of reference for determining where
//to position the new controls. All new controls above and to the
//left of the stc32 control are positioned the same amount above
//and to the left of the controls in the default dialog box. New
//controls below and to the right of the stc32 control are
//positioned below and to the right of the default controls. In
//general, each new control is positioned so that it has the same
//position relative to the default controls as it had to the stc32
//control. To make room for these new controls, the system adds
//space to the left, right, bottom, and top of the default dialog
//box as needed.

//The system requires the hook procedure to process all messages
//intended for the custom dialog box and therefore sends the same
//window messages to the hook procedure as to any other dialog box
//procedure. For example, the hook procedure receives WM_COMMAND
//messages when the user clicks on button controls in the custom
//dialog box. The hook procedure is responsible for initializing
//these controls and retrieving values from the controls when the
//dialog box is closed. Note that when the hook procedure receives
//the WM_INITDIALOG message, the system has not yet moved the
//controls to their final positions.

//The default dialog box procedure handles messages for all the
//controls in the default dialog box, but the hook procedure
//receives the WM_NOTIFY notification messages for user actions on
//these controls as described in Explorer-Style Hook
//Procedures.

BOOL GetFileName( LPSTR lpFn, WORD wIDString )
{
	BOOL		      rflag;
	DWORD          flags;
	HANDLE         hDibInfo;            // Handle to extra words
	LPDIBINFO      ptr;                 // pointer to extra words
   LPOPENFILENAME pof;

   pof = &gOpnFN;    // pointer to GLOBAL open file name buffer
   ZeroMemory( pof, sizeof(OPENFILENAME) );
   sprtf( "Entering GetFileName, for %s, with %s ..."MEOR,
      ((wIDString == IDS_SAVEDLG) ? "SAVEDLG" :
      (wIDString == IDS_OPENDLG) ? "OPENDLG" : "UNKNOWN ID"),
      lpFn );
	rflag = FALSE;
	fpDlgFunc = 0;
	ExpInfo.is_szName[0] = 0;
	ExpInfo.is_hdlDib    = 0;	// NO Dib handle
	ExpInfo.is_hPal      = 0;	// NO Palette
	ExpInfo.is_hBitmap   = 0;	// NO Bitmap
	gszFile[0]           = '\0';
	pADir                = &gszWDirName[0];

	if( wIDString == IDS_OPENDLG )
	{
		pADir = &gszRDirName[0];
		LoadString( ghDvInst, wIDString, szTitle, sizeof (szTitle));
		LoadString( ghDvInst, IDS_FILEOPEN, szTemplate, sizeof (szTemplate));
#ifdef	WIN32
		if( fUseExplorer )
		{
			flags = FNOpenFlag1;
		}
		else
		{
			fpDlgFunc = (FARPROC) MakeProcInstance( FILEOPENHOOKPROC, ghDvInst );
			flags = FNOpenFlag2;
		}
#else // !WIN32
		fpDlgFunc = (FARPROC) MakeProcInstance( FILEOPENHOOKPROC, ghDvInst );
		flags = FNOpenFlag2;
#endif	// WIN32 y/n
	}
	else if( wIDString == IDS_SAVEDLG )
	{
      ptr = 0;
      if( hDibInfo = (HANDLE) GetWindowExtra( GetCurrentMDIWnd(), WW_DIB_HINFO ) )
         ptr = (LPDIBINFO) DVGlobalLock( hDibInfo );

		LoadString( ghDvInst, wIDString, szTitle, sizeof (szTitle) );
		LoadString( ghDvInst, IDS_FILESAVE, szTemplate, sizeof (szTemplate) );
		fpDlgFunc = (FARPROC) MakeProcInstance( FILESAVEHOOKPROC,
			ghDvInst );
#ifdef	WIN32
		if( fUseExplorer )
			flags = FNSaveFlag1;
		else
			flags = FNSaveFlag2;
#else	// !WIN32
		flags = FNSaveFlag2;
#endif	// WIN32 y/n
      if(ptr)
      {
         lstrcpy( lpFn, ptr->di_szDibFile );
   		lstrcpy( (LPSTR) gszFile, (LPSTR)ptr->di_szDibFile );
      }
      else
         *lpFn = 0;

//		_splitpath (gszFile, szDrive, szDir, szFname, szExt);
		DVGetFPath( gszFile, gszDrive, gszDir, gszFname, gszExt );
		if( pADir[0] == 0 )	/* If we appear to have NO SAVE/Write directory */
		{
			lstrcat( (LPSTR) pADir, (LPSTR) gszDrive );
			lstrcat( (LPSTR) pADir, (LPSTR) gszDir );
		}

		gszFile[0]=0;
		lstrcpy( (LPSTR)gszFile, (LPSTR) gszFname );
		if( gszExt[0] )	/* If it ALREADY has an extent ... */
		{
         /* Maybe we should consider ADJUSTING it per the index ... */
			if( (gwOutIndex < MXCLST) && pRDefLst[gwOutIndex] )
				lstrcat( (LPSTR)gszFile, (LPSTR) pRDefLst[gwOutIndex] );
			else	/* can only use the current extent ... */
				lstrcat ((LPSTR)gszFile, (LPSTR) gszExt);
		}
		else	/* It presently DOES NOT HAVE an EXTENT ... */
		{
         /* OK, we will use gwOutIndex if it is valid ... */
			if( (gwOutIndex < MXCLST) && pRDefLst[gwOutIndex] )
				lstrcat( (LPSTR)gszFile, (LPSTR) pRDefLst[gwOutIndex] );
			else
				lstrcat( (LPSTR)gszFile, (LPSTR) pRDefExt );	/* BMP extent */
		}

      if( hDibInfo && ptr )
         DVGlobalUnlock( hDibInfo );

	}

	pof->lStructSize       = sizeof (OPENFILENAME);
	pof->hwndOwner         = GetFocus ();
	pof->hInstance         = ghDvInst;
	pof->lpstrFilter       = pszFilter; // pointer to nul terminated PAIRS
	//pof->lpstrCustomFilter = NULL;
	//pof->nMaxCustFilter    = 0L;
	if( wIDString == IDS_SAVEDLG )
	{
		pof->nFilterIndex      = (DWORD) (gwOutIndex + 1);
	}
	else
	{
		pof->nFilterIndex      = (DWORD) (InIndex + 1);
	}
	pof->lpstrFile         = gszFile;
	pof->nMaxFile          = 256;    // sizeof (szFile);
	pof->lpstrFileTitle    = gszFileTitle;
	pof->nMaxFileTitle     = 256;    // sizeof (szFileTitle);
	pof->lpstrInitialDir   = pADir;	/* Should be SAVE(W) or OPEN(R) Directory */
	pof->lpstrTitle        = szTitle;
	pof->Flags             = flags;
   sprtf( "Set initial directory to [%s] ..."MEOR, pADir );
	//pof->nFileOffset       = 0;
	//pof->nFileExtension    = 0;
	//pof->lpstrDefExt       = NULL;
#ifdef	WIN32

	if( fUseExplorer )
	{
		//pof->lpfnHook = NULL;
		//pof->lpTemplateName = NULL;
#if   (defined(ADDREVIEW) || defined(ADDREVIEW2))
		if( wIDString == IDS_OPENDLG )	/* Open - Load File ... */
		{
#ifdef   WIN32
				pof->lpfnHook	      = (LPOFNHOOKPROC) EXPOPENHOOKPROC;
				pof->lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPEN);  // was IDD_DIALOG2
				pof->Flags         |= OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
            sprtf( "Setting HOOK nd IDD_FILEOPEN pofn = %#x."MEOR,
               pof );
#else // !WIN32
			if( fpDlgFunc = (FARPROC) MakeProcInstance( EXPOPENHOOKPROC, ghDvInst ) )
			{
				pof->lpfnHook	      = (LPOFNHOOKPROC) fpDlgFunc;
#ifdef	IMGFIXED
				pof->lpTemplateName = MAKEINTRESOURCE( IDD_FILEOPEN );
#else	// !IMGFIXED
				pof->lpTemplateName = MAKEINTRESOURCE( IDD_DIALOG3 );
#endif	// IMGFIXED y/n
				pof->Flags         |= OFN_ENABLEHOOK | OFN_ENABLETEMPLATE;
			}
#endif   // WIN32 y/n

		}
#endif	// ADDREVIEW or ADDREVIEW2
	}
	else
	{
#ifdef	WIN32
		pof->lpfnHook	= (LPOFNHOOKPROC) fpDlgFunc;
#else	// !WIN32
		pof->lpfnHook		= (FARHOOK)fpDlgFunc;
#endif	// WIN32 y/n
		pof->lpTemplateName    = szTemplate;
	}

#else	// !WIN32
	pof->lpfnHook		     = (FARHOOK)fpDlgFunc;
	pof->lpTemplateName    = szTemplate;
#endif	// WIN32 y/n

	// Call the GetOpenFilename function
	if( wIDString == IDS_OPENDLG )	/* Open - Load File ... */
	{
		if( GetOpenFileName (pof) )
		{
			lstrcpy( lpFn, pof->lpstrFile);
			if( InIndex != (WORD) (pof->nFilterIndex - 1) )
			{
				fChgInInd = TRUE;
				InIndex = (WORD) (pof->nFilterIndex - 1);
			}
			rflag = TRUE;
		}
		else
		{
#ifdef	WIN32
			flags = CommDlgExtendedError();
			switch( flags )
			{
			case CDERR_FINDRESFAILURE:
			case CDERR_NOHINSTANCE:
			case CDERR_INITIALIZATION:
			case CDERR_NOHOOK:
			case CDERR_LOCKRESFAILURE:
			case CDERR_NOTEMPLATE:
			case CDERR_LOADRESFAILURE:
			case CDERR_STRUCTSIZE:
			case CDERR_LOADSTRFAILURE:
			case FNERR_BUFFERTOOSMALL:
			case CDERR_MEMALLOCFAILURE:
			case FNERR_INVALIDFILENAME:
			case CDERR_MEMLOCKFAILURE:
			case FNERR_SUBCLASSFAILURE:
				pExtErr = &szComErr[0];	// = "Common Error";
				break;
			default:
				pExtErr = &szUnkErr[0];	// = "Unknown Error";
				break;
			}
#else	// !WIN32
			pExtErr = &szUnkErr[0];	// = "Unknown Error";
#endif	// WIN32 y/n
 			rflag = FALSE;
		}
	}
	else	/* if( wIDString == IDS_SAVEDLG ) */
	{
		if( GetSaveFileName (pof) )
		{
			lstrcpy( lpFn, pof->lpstrFile );
			AddBMPExt( lpFn, pof->nFilterIndex );	/* Ensure an EXTENT added*/
			rflag = TRUE;		/* AND set gwOutIndex value for later ... */
		}
	}
	if( fpDlgFunc )
		FreeProcInstance( fpDlgFunc );
	fpDlgFunc = 0;

	// NOTE: This COULD be used INSTEAD of RE-LOADING the file!!!
	// ==========================================================
	if( ExpInfo.is_hdlDib )	// IF Dib handle
		DVGlobalFree( ExpInfo.is_hdlDib );
	ExpInfo.is_hdlDib = 0;

	if( ExpInfo.is_hBitmap )
		DeleteObject( ExpInfo.is_hBitmap );
	ExpInfo.is_hBitmap = 0;

	if( ExpInfo.is_hPal )
		DeleteObject( ExpInfo.is_hPal );
	ExpInfo.is_hPal = 0;

	return( rflag );

}


BOOL Do_IDD_INFO( HWND hDlg )
{
	BOOL	rFlg = FALSE;
	LPINFOSTRUCT lpi;
#ifndef	WIN32
	FARPROC    lpInfo;
#endif

	// User has selected the Dib Information button.  Query the
	// text in the edit control, check if it is a valid file, get
	// the file statistics, and then display the dialog box.
	if( lpi = (LPINFOSTRUCT)GMLocalAlloc( LPTR, sizeof(INFOSTRUCT) ) )
	{
		NULPINFOSTRUCT(lpi);
		GetDlgItemText( hDlg, edt1,
			(LPSTR) szDFileName, sizeof (szDFileName) );
		if( CheckIfFileExists( szDFileName ) )
		{
			if( GetDibInfo( szDFileName, lpi, FALSE ) )
			{
#ifdef	WIN32
#ifdef WIN64
				DialogBoxParam( ghDvInst, "INFO", hDlg,
					(DLGPROC)INFODLGPROC,	// lpInfo,
					(LPARAM)(LPINFOSTRUCT) lpi );
#else // !WIN64
				DialogBoxParam( ghDvInst, "INFO", hDlg,
					INFODLGPROC,	// lpInfo,
					(DWORD) (LPINFOSTRUCT) lpi );
#endif // WIN64 y/n
#else	/* !WIN32 */
				lpInfo = MakeProcInstance( INFODLGPROC, ghDvInst );
				DialogBoxParam( ghDvInst, "INFO", hDlg, lpInfo,
					(DWORD) (LPINFOSTRUCT) lpi );
				FreeProcInstance (lpInfo);
#endif	/* WIN32 y/n */
				rFlg = TRUE;
			}
			else
			{
				DIBError( ERR_READ );
			}
		}
		else
		{
			DIBError( ERR_OPEN );
			SetFocus( GetDlgItem( hDlg, edt1 ) );
		}
		GMLocalFree(lpi);
	}
	return rFlg;
}

BOOL Do_IDD_PREVIEW( HWND hDlg )
{
	BOOL		bRet = FALSE;
	LPINFOSTRUCT lpi;
#ifndef	WIN32
	FARPROC    lpInfo;
#endif

	// User has selected PREVIEW Button
	//chkpv();
	if( lpi = (LPINFOSTRUCT)GMLocalAlloc( LPTR, sizeof(INFOSTRUCT) ) )
	{
		NULPINFOSTRUCT(lpi);
		GetDlgItemText( hDlg, edt1, (LPSTR) szDFileName, sizeof (szDFileName) );
		lpi->is_hdlDib = 0;	// No DIB Handle ...
		if( CheckIfFileExists( szDFileName ) )
		{
			if( ((lpi->is_hdlDib = GetDIB((LPSTR) szDFileName, gd_DP )) == 0) ||
				(!GetDibInfo( szDFileName, lpi, FALSE )) )
			{
				if( lpi->is_hdlDib )
					DVGlobalFree( lpi->is_hdlDib );
				DIBError( ERR_READ );
			}
			else
			{
#ifdef	WIN32
#ifdef WIN64
				DialogBoxParam( ghDvInst, "INFOVIEW", hDlg,
					(DLGPROC)VIEWDLGPROC,	// lpInfo,
					(LPARAM)(LPINFOSTRUCT) lpi );
#else //!#ifdef WIN64
				DialogBoxParam( ghDvInst, "INFOVIEW", hDlg,
					VIEWDLGPROC,	// lpInfo,
					(DWORD) (LPINFOSTRUCT) lpi );
#endif //#ifdef WIN64
#else	/* !WIN32 */
				lpInfo = MakeProcInstance(VIEWDLGPROC, ghDvInst);
				DialogBoxParam( ghDvInst, "INFOVIEW", hDlg, lpInfo,
					(DWORD) (LPINFOSTRUCT) lpi );
				FreeProcInstance (lpInfo);
#endif	/* W32 y/n */
				if( lpi->is_hdlDib )
					DVGlobalFree( lpi->is_hdlDib );
				bRet = TRUE;
			}
		}
		else
		{
			DIBError( ERR_OPEN );
			SetFocus( GetDlgItem( hDlg, edt1 ) );
		}
	}
	return bRet;
}

/* ************************************************************************

  Function:  FILEOPENHOOKPROC( HWND, UINT, WPARAM, LPARAM )

   Purpose:  Hook procedure for FileOpen common dialog box.

   Returns:  TRUE if message was processed.
             FALSE if the system needs to process the message.

  Comments:

   History:   Date     Reason

             6/27/91   Created

************************************************************************ */
//EXPORT32
//BOOL MLIBCALL FILEOPENHOOKPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
UINT_PTR CALLBACK FILEOPENHOOKPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   DWORD cmd = LOWORD(wParam);
   switch (msg)
   {
   case WM_COMMAND:
        switch(cmd)
        {
        case IDOK:
			  GetCurWDir( pADir );
           break;

        case IDD_INFO:
			  Do_IDD_INFO( hDlg );
           break;

        case IDD_PREVIEW:
			  Do_IDD_PREVIEW( hDlg );
			  break;

        }
    break;
  }
  return FALSE;
}

BOOL	Do_FSH_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL		bRet;
	HANDLE		hDibInfo;
	LPDIBINFO	ptr;
	int			nIDDlgItem, nIDBits;

	bRet = TRUE;
#ifdef	DIAGSAVEHOOK
	WriteBgnDiag();	/* Output the BEGIN message ... */
#endif

	//  Get the memory handle stored in the extra words.
	//  From this, insert the filename, and the file format
	if( ( hDibInfo = GetWindowExtra( GetCurrentMDIWnd(), WW_DIB_HINFO) ) &&
		 ( ptr = (LPDIBINFO) DVGlobalLock (hDibInfo) ) )
	{
		// Only IF we have a HANDLE and a POINTER
		nIDDlgItem = IDD_RGB;	// Set a DEFAULT
		switch( ptr->wDIBType )
		{

		case BI_RGB:
			nIDDlgItem = IDD_RGB;
			break;

		case BI_RLE4:
			nIDDlgItem = IDD_RLE4;
			break;

		case BI_RLE8:
			nIDDlgItem = IDD_RLE8;
			break;

		case BI_PM:
			nIDDlgItem = IDD_PM;
			break;
		}	// Switch wDIBType
		SendDlgItemMessage( hDlg, nIDDlgItem,   BM_SETCHECK, 1, 0L );

		nIDBits = 0;
		switch( ptr->di_dwDIBBits )
		{

		case 1:
			nIDBits = IDD_1;
			break;

		case 4:
			nIDBits = IDD_4;
			break;

		case 8:
			nIDBits = IDD_8;
			break;

		case 24:
			nIDBits = IDD_24;
			break;

		}	// Switch di_dwDIBBits

		if( nIDBits )
			SendDlgItemMessage( hDlg, nIDBits,   BM_SETCHECK, 1, 0L );

		DVGlobalUnlock( hDibInfo );

	}	// If HANDLE and POINTER OK

	return bRet;
}

BOOL	Do_FSH_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL		bRet;
	HWND		hGroup;
	RECT		rect, DlgRect;
   DWORD    cmd = LOWORD(wParam);
	bRet = FALSE;

//	switch( wParam )
	switch( cmd )
	{

	case IDOK:
#ifdef	DIAGSAVEHOOK
		WriteEndDiag();	// Output the BEGIN message
#endif	// DIAGSAVEHOOK

		GetCurWDir( pADir );	// Update the ACTIVE SAVE Directory

		// Get output STYLE
		if( SendDlgItemMessage( hDlg, IDD_RGB, BM_GETCHECK, 0, 0L) )
			gdwbiStyle = BI_RGB;
		else if( SendDlgItemMessage( hDlg, IDD_RLE4, BM_GETCHECK, 0, 0L) )
			gdwbiStyle = BI_RLE4;
		else if( SendDlgItemMessage( hDlg, IDD_RLE8, BM_GETCHECK, 0, 0L) )
			gdwbiStyle = BI_RLE8;
		else
			gdwbiStyle = BI_PM;

		// Get output BITS
		if( SendDlgItemMessage( hDlg, IDD_1, BM_GETCHECK, 0, 0L ) )
			wbiBits = 1;
		else if( SendDlgItemMessage( hDlg, IDD_4, BM_GETCHECK, 0, 0L ) )
			wbiBits = 4;
		else if( SendDlgItemMessage( hDlg, IDD_8, BM_GETCHECK, 0, 0L ) )
			wbiBits = 8;
		else
			wbiBits = 24;

		bRet = TRUE;
		break;

	case IDD_FILETYPE:
		hGroup = GetDlgItem( hDlg, IDD_FILETYPEGROUP );
		GetWindowRect( hGroup, &rect );
		GetWindowRect( hDlg, &DlgRect );
		SetWindowPos( hDlg,
			0,
			DlgRect.left,
			DlgRect.top,
			(DlgRect.right-DlgRect.left),
			(rect.bottom+(rect.left-DlgRect.left)-DlgRect.top),
			SWP_NOMOVE | SWP_NOZORDER );
		EnableWindow( GetDlgItem( hDlg, IDD_FILETYPE), 0 );
		SetFocus( hGroup );
		bRet = TRUE;
		break;

	case IDD_RLE4:
		if( SendDlgItemMessage( hDlg, IDD_1, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_1, BM_SETCHECK, 0,0L );
		if( SendDlgItemMessage( hDlg, IDD_8, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_8, BM_SETCHECK, 0, 0L );
		if( SendDlgItemMessage( hDlg, IDD_24, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_24, BM_SETCHECK, 0, 0L );
		EnableWindow( GetDlgItem(hDlg, IDD_4), 1 );
		SendDlgItemMessage( hDlg, IDD_4, BM_SETCHECK, 1, 0L );
		EnableWindow( GetDlgItem( hDlg, IDD_1 ), 0 );
		EnableWindow( GetDlgItem( hDlg, IDD_8 ), 0 );
		EnableWindow( GetDlgItem( hDlg, IDD_24), 0 );
		bRet = TRUE;
		break;

	case IDD_RLE8:

		if( SendDlgItemMessage( hDlg, IDD_1, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_1, BM_SETCHECK, 0, 0L );
		if( SendDlgItemMessage( hDlg, IDD_4, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_4, BM_SETCHECK, 0, 0L );
		if( SendDlgItemMessage( hDlg, IDD_24, BM_GETCHECK, 0, 0L ) )
			SendDlgItemMessage( hDlg, IDD_24, BM_SETCHECK, 0, 0L );
		EnableWindow( GetDlgItem( hDlg, IDD_8 ), 1 );
		SendDlgItemMessage( hDlg, IDD_8, BM_SETCHECK, 1, 0L );
		EnableWindow( GetDlgItem( hDlg, IDD_1 ), 0 );
		EnableWindow( GetDlgItem( hDlg, IDD_4 ), 0 );
		EnableWindow( GetDlgItem( hDlg, IDD_24), 0 );
		bRet = TRUE;
		break;

	case IDD_RGB:
	case IDD_PM:
		EnableWindow( GetDlgItem( hDlg, IDD_1 ), 1 );
		EnableWindow( GetDlgItem( hDlg, IDD_4 ), 2 );
		EnableWindow( GetDlgItem( hDlg, IDD_8 ), 3 );
		EnableWindow( GetDlgItem( hDlg, IDD_24), 1 );
		bRet = TRUE;
		break;

	}
	return bRet;
}

/* ***************************************************************

  Function:  FILESAVEHOOKPROC (HWND, UINT, WPARAM, LPARAM)

   Purpose:  Hook procedure for FileSave common dialog box.

   NOTE: Appears "FILESAVE" dialog template NOT_USED

   Returns:  TRUE if message was processed.
             FALSE if the system needs to process the message.

  Comments:

   History:   Date    Reason

             7/8/91   Created

   *************************************************************** */
//EXPORT32
//BOOL MLIBCALL FILESAVEHOOKPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)

UINT_PTR CALLBACK FILESAVEHOOKPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL		bRet;

	bRet = FALSE;
#ifdef	DIAGSAVEHOOK
	DiagSaveHook( hDlg, msg, wParam, lParam );
#endif	// DIAGSAVEHOOK

	switch( msg )
	{

	case WM_INITDIALOG:
		bRet = Do_FSH_Init( hDlg, wParam, lParam );
        break;

    case WM_COMMAND:
		bRet = Do_FSH_Command( hDlg, wParam, lParam );
        break;

    default:
		break;

	}	// Switch ( msg )

	return( bRet );
}

/* ==================================================================
 * AddBMPExt( FIleName, Index )
 *
 * ================================================================== */
void AddBMPExt( LPSTR fn, DWORD index )
{
	WORD i;
	LPSTR	lps;
	if( index )
	{
		if( gwOutIndex != (WORD) (index - 1) )
		{
			gwOutIndex = (WORD) (index - 1);
			fChgOutInd = TRUE;
		}
	}
	else
	{
		gwOutIndex = 0;
	}
	if( gwOutIndex < MXELST )
	{
		lps = (LPSTR) pRDefLst[gwOutIndex];	/* Select PER INDEX ... */
		if( (lps == 0) || (lps[0] == 0) )
		{
			gwOutIndex = 0;
			lps = (LPSTR) pRDefLst[gwOutIndex];	/* Select BMP ... */
		}
	}
	else
	{
		fChgOutInd = TRUE;
		gwOutIndex = 0;
		lps = (LPSTR) pRDefLst[gwOutIndex];	/* Select BMP ... */
	}
	if( i = lstrlen( fn ) )
	{
		if( InStr( fn, "." ) == 0 )	/* If NO EXTENT ... */
		{
			lstrcat( fn, lps );	/* Add an EXTENT ... BMP is default ... */
		}
	}
}


void	GetCurWDir( LPSTR pDir )
{
	char	szD[MXDIR];
	LPSTR	lps;
	LPINT	lpb;

	if( pDir == &gszRDirName[0] )
		lpb = &gfChgOpen ;
	else if( pDir == &gszWDirName[0] )
		lpb = &gfChgSave ;
	else
		lpb = 0;
	if( lpb )
	{
		lps = &szD[0];
		//_getcwd( &szD[0], MXDIR );	/* Update the ACTIVE Directory ... */
		// if( dwL = GetModuleFileName( NULL, lpf, MAX_PATH ) )
		GetRootDir( &szD[0] );
		if( szD[0] &&
			( lstrcmpi( pDir, lps ) != 0 ) )
		{
			lstrcpy( pDir, lps );
			*lpb = TRUE;
		}
	}
}

// **************************************************************
//
//  Function:  Dv_IDM_SAVE( HWND ) (Formerly SaveDIBFile (void))
//
//   Purpose:  Prompts user for a filename, opens it, and writes DIB
//             out in format specified.
//             Handles IDM_SAVE from the MENU
//
//   Returns:  FALSE if no file is opened, or an error occurs writing the file.
//             TRUE  if successful
//
//  Comments:
//
//   History:   Date      Reason
//
//             7/8/91    Created
//             10/1/91   BugFix -- wasn't freeing allocated memory,
//                        Cleaned up code, and added comments.
//             10/2/91   Added error checking/reporting
//	Feb 96	Added save as JPEG (compressed/lossy) JPG file - Geoff.
//
// **************************************************************

// Formerly BOOL SaveDIBFile (void)
BOOL Dv_IDM_SAVE( HWND hWnd )
{
	BOOL		bNew;
	HANDLE		hDib, hDibInfo;
	LPDIBINFO	lpDIBInfo;
	DWORD		dwDIBHeight, dwDIBWidth;
	DWORD		wBPP;
	char		szTitleBuf[MAX_PATH+16];
	BOOL		flg;
	HWND		hMDIWnd;
	LPSTR		lpt, lpd;

	// A little setup
	flg  = FALSE;
	bNew = FALSE;
	lpt = &szTitleBuf[0];
	lpd = &szDFileName[0];
	*lpt = 0;
	// In some case a NEW child can be Created ... so get this NOW
	//  Get some information about the DIB.  Some of the info
	//  is obtained from within the header (be it a BITMAPINFOHEADER
	//  or a BITMAPCOREHEADER).
	// Get the FileName to save under.

	if( ( hMDIWnd = GetCurrentMDIWnd() ) &&
		( GetFileName( lpd, IDS_SAVEDLG ) ) )
	{
		// Get all the info on the current DIB Window.
		if( ( hDibInfo = GetWindowExtra( hMDIWnd, WW_DIB_HINFO) ) == 0 )
		{
			DIBError( ERR_NO_INFO );
			return( FALSE );
		}
		if( (lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDibInfo )) == 0 )
		{
			DIBError( ERR_NO_LOCK );
			return( FALSE );
		}

		//SetCursor(LoadCursor(NULL, IDC_WAIT));
		Hourglass( TRUE );

		// Convert the DDB (Device Dependant Bitmap)
		// to the format desired.
		if( gdwbiStyle == BI_PM )
		{
			hDib = PMDibFromBitmap( lpDIBInfo->hBitmap,
				gdwbiStyle,
				wbiBits,
				lpDIBInfo->hPal );
			if( hDib )
			{
				bNew = TRUE;	// Mark for CLEANUP
			}
			else
			{
				DVGlobalUnlock( hDibInfo );
				Hourglass( FALSE );
				DIBError( ERR_NO_PMDIB );
				return( FALSE );
			}
		}
		else
		{
			// Here we convert the SCREEN BITMAP to a
			// DEVICE INDEPENDANT BITMAP (DIB)
			// BUT we also already have a DIB lpDIBInfo->hDIB,
			// which is the ORIGINAL (READ) DIB used to CREATE this BITMAP
			// for screen rendering.
			if( hDib = lpDIBInfo->hDIB )
			{
				// Use the READ IN DIB
				bNew = FALSE;	// Mark NO DIB CLEANUP
			}
			else
			{
				// Use the SCREEN RENDERED BITMAP
				hDib = WinDibFromBitmap( lpDIBInfo->hBitmap,
					gdwbiStyle,
					wbiBits,
					lpDIBInfo->hPal);
				if( hDib )
				{
					bNew = TRUE;	// Mark for CLEANUP
				}
				else
				{
					DVGlobalUnlock( hDibInfo );
					Hourglass( FALSE );
					DIBError( ERR_NO_WINDIB );
					return( FALSE );
				}
			}
		}

		// Write out the DIB in the specified format.
		if( WriteDIB( lpd, hDib ) )
		{
			// If successfully written, change the WINDOW TITLE
//			_splitpath( lpd, szDrive, szDir, szFname, szExt );
			DVGetFPath( lpd, gszDrive, gszDir, gszFname, gszExt );
			wBPP        = lpDIBInfo->di_dwDIBBits;
			dwDIBWidth  = lpDIBInfo->di_dwDIBWidth;
			dwDIBHeight = lpDIBInfo->di_dwDIBHeight;
			wsprintf( lpt,
            pRDibTit,
				lpd, dwDIBWidth, dwDIBHeight, wBPP);
			SetWindowText( hMDIWnd, lpt );

#ifdef	WIN32
			lstrcpy( lpt, (LPSTR)gszDrive );
			lstrcat( lpt, (LPSTR)gszDir );
			if( lstrcmpi( lpt, &gszWDirName[0] ) )
			{
				lstrcpy( &gszWDirName[0], lpt );
				gfChgSave = TRUE;
			}
#endif	// WIN32

#ifdef	CHGADDTO
			AddToFileList( lpd );
#endif	// CHGADDTO
			flg = TRUE;
		}

		// Clean up and return.
		if( bNew )
		{
			// Throw away this one
			DVGlobalFree( hDib );
		}

		DVGlobalUnlock( hDibInfo );
//      SetCursor(LoadCursor(NULL, IDC_ARROW));

		Hourglass( FALSE );

	}
	return( flg );
}

// **************************************************************
// Dv_IDM_SAVEAS()
// Uses typedef struct tagINFOSTRUCT {		/* is */
//	HWND		is_hMDIWnd;
//	HANDLE	is_hDibInfo;
//	HANDLE	is_hdlDib;		// Handle of DIB read in
//	HPALETTE	is_hPal;		// Bitmap's palette. (if exists)
//	HBITMAP	is_hBitmap;		// Handle to the DDB. (if exists)
//	DWORD		is_cbWidth;		// Width
//	DWORD		is_cbHeight;	// Height
//	DWORD		is_cbColors;	// Color count
//	DWORD		is_dwType;	// One of BI_RGB, BI_RLE4, BI_RLE8, BI_PM
//	DWORD		is_dwBits;	// One of 1, 4, 8 or 24.
//	BOOL		is_bOpenWnd;	// Open a CHILD Window
//	UINT		is_uType, is_uRType, is_uBits, is_uRBits;
//	DWORD		is_Res1, is_Res2, is_Res3;	// Just some RESERVED
//	TCHAR		is_szCompress[8];	// Compression type
//	TCHAR		is_szType[16];	// Type
//	TCHAR		is_szName[MAX_PATH+8];	// File Name
//	TCHAR		is_szTitle[MAX_PATH+8];	// File Title
//}INFOSTRUCT, * PI;

//#define		ATOM_SAVEAS		0x115

void	Do_SA_Defaults( HWND hDlg, LPINFOSTRUCT lpi )
{
	CheckDlgButton( hDlg,	// handle to dialog box
		IDC_CHECK1,	// button-control identifier and check state
		(lpi->is_bOpenWnd ? BST_CHECKED : BST_UNCHECKED) );


	if( lpi->is_hBitmap )
	{
		// CheckRadioButton
		CheckRadioButton( hDlg,	// handle to dialog box
			IDD_RGB,	// identifier of first radio button in group
			IDD_PM,		// identifier of last radio button in group
			lpi->is_uType );	// identifier of radio button to select
		CheckRadioButton( hDlg,	// handle to dialog box
			IDD_1,	// identifier of first radio button in group
			IDD_24,		// identifier of last radio button in group
			lpi->is_uBits );	// identifier of radio button to select
	}
	else
	{
		// We can NOT party with the non-existant BITMAP
		HWND	hWnd;
		UINT	ui;
		for( ui = IDD_RGB; ui <= IDD_PM; ui++ )
		{
			if( ui == lpi->is_uType )
			{
				CheckDlgButton( hDlg,	// handle to dialog box
					ui,		// button-control identifier
					BST_CHECKED );
			}
			else if( hWnd = GetDlgItem( hDlg, ui ) )
			{
				EnableWindow( hWnd, FALSE );
				ShowWindow( hWnd, SW_HIDE );
			}
		}
		for( ui = IDD_1; ui <= IDD_24; ui++ )
		{
			if( ui == lpi->is_uBits )
			{
				CheckDlgButton( hDlg,	// handle to dialog box
					ui,		// button-control identifier
					BST_CHECKED );
			}
			else if( hWnd = GetDlgItem( hDlg, ui ) )
			{
				EnableWindow( hWnd, FALSE );
				ShowWindow( hWnd, SW_HIDE );
			}
		}
	}

	// Finally SET A NEW NAME
	SetDlgItemText( hDlg,
		IDC_EDIT1,
		&lpi->is_szName[0] );

	// And SET FOCUS TO IT
	SetFocus( GetDlgItem( hDlg, IDC_EDIT1 ) );

}

BOOL	Do_SA_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL			bRet = TRUE;
	LPINFOSTRUCT	lpi;
	HANDLE			hDibInfo, hDib;
	LPDIBINFO		lpDibInfo;
	LPSTR			lpdib;
	UINT			iType, iBits;

	iType = 0;
	CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ) );
	if( ( lpi = (LPINFOSTRUCT)lParam ) &&
		( hDibInfo = lpi->is_hDibInfo ) &&
		( lpDibInfo = (LPDIBINFO)DVGlobalLock( hDibInfo ) ) )
	{
		SET_PROP( hDlg, ATOM_SAVEAS, (HANDLE)lpi );

		SetDlgItemText( hDlg,
			IDC_EDIT5,
			&lpDibInfo->di_szDibFile[0] ); // DIB's filename
		lstrcpy( &lpi->is_szName[0], &lpDibInfo->di_szDibFile[0] );	// File Name
		FindNewName( &lpi->is_szName[0] );

		// Establish DEFAULTS
		iType = IDD_RGB;
		iBits = IDD_8;
		if( ( hDib = lpDibInfo->hDIB       ) &&
			 ( lpdib = DVGlobalLock( hDib ) ) ) // LOCK DIB HANDLE
		{
			LPBITMAPINFOHEADER lpbmi;
			lpbmi = (LPBITMAPINFOHEADER)lpdib;
			if( lpbmi->biSize == sizeof (BITMAPINFOHEADER) )
			{
				switch( lpbmi->biCompression )
				{
				case BI_RGB:
					iType = IDD_RGB;
					break;
				case BI_RLE4:
					iType = IDD_RLE4;
					break;
				case BI_RLE8:
					iType = IDD_RLE8;
					break;
				default:
					iType = IDD_RGB;
					break;
				}
			}
			else
			{
				iType = IDD_PM;
			}
			lpi->is_cbHeight = DIBHeight(lpdib);
			lpi->is_cbWidth = DIBWidth(lpdib);		// Width
			lpi->is_cbColors = DIBNumColors(lpdib);
			switch( DIBBitCount(lpdib) )
			{
			case 1:
				iBits = IDD_1;
				break;
			case 4:
				iBits = IDD_4;
				break;
			case 8:
				iBits = IDD_8;
				break;
			case 24:
				iBits = IDD_24;
				break;
			default:
				iBits = IDD_8;
				break;
			}

			// SetDlgItemInt
			SetDlgItemInt( hDlg,	// handle of dialog box
				IDC_EDIT2,	// identifier of control
				lpi->is_cbWidth,	// value to set
				FALSE );	// signed or unsigned indicator
			SetDlgItemInt( hDlg,	// handle of dialog box
				IDC_EDIT3,	// identifier of control
				lpi->is_cbHeight,	// value to set
				FALSE );	// signed or unsigned indicator
			SetDlgItemInt( hDlg,	// handle of dialog box
				IDC_EDIT4,	// identifier of control
				lpi->is_cbColors,	// value to set
				FALSE );	// signed or unsigned indicator

			DVGlobalUnlock(hDib);   // UNLOCK DIB HANDLE

		}

		// Keep current values
		lpi->is_uType = iType;
		lpi->is_uBits = iBits;
//	HPALETTE	is_hPal;		// Bitmap's palette. (if exists)
//	HBITMAP		is_hBitmap;		// Handle to the DDB. (if exists)
		lpi->is_hPal = lpDibInfo->hPal;	// if one
		lpi->is_hBitmap = lpDibInfo->hBitmap;

		// and the REQUEST is the SAME
		lpi->is_uRType = iType;
		lpi->is_uRBits = iBits;

		Do_SA_Defaults( hDlg, lpi );

		bRet = FALSE;

		DVGlobalUnlock( hDibInfo );
	}
	else
	{
		EndDialog( hDlg, FALSE );
		bRet = (BOOL)-1;
	}
	return bRet;
}

BOOL	IsCheckedButton( HWND hDlg, UINT id )
{
	BOOL	flg = FALSE;
	UINT	ui;
 
	ui = IsDlgButtonChecked( hDlg,     // handle to dialog box
		id );	// button identifier
	if( ui == BST_CHECKED )
		flg = TRUE;
	return flg;
}

BOOL	Do_SA_Browse( HWND hDlg )
{
	BOOL	bRet = FALSE;
	LPINFOSTRUCT	lpi;
	LPSTR			lpf;
	UINT			ui;

	if( ( lpi = (LPINFOSTRUCT)GET_PROP( hDlg, ATOM_SAVEAS ) ) &&
		( lpf = GMLocalAlloc( LPTR, MAX_PATH ) ) )
	{
		// GetDlgItemText
		if( ui = GetDlgItemText( hDlg,	// handle of dialog box
			IDC_EDIT1,	// identifier of control
			lpf,		// address of buffer for text
			MAX_PATH ) )	// maximum size of string
		{
			// Got the TEXT from the EDIT BOX
		}
		else
		{
			lstrcpy( lpf, &lpi->is_szName[0] );
		}
		if( GetFileName( lpf, IDS_SAVEDLG ) )
		{
			lstrcpy( &lpi->is_szName[0], lpf );
			// Finally SET A NEW NAME
			SetDlgItemText( hDlg,
				IDC_EDIT1,
				&lpi->is_szName[0] );
			// And SET FOCUS TO IT
			SetFocus( GetDlgItem( hDlg, IDC_EDIT1 ) );
			bRet = TRUE;
		}

		GMLocalFree(lpf);
	}
	return bRet;
}

BOOL	Do_SA_OK( HWND hDlg, LPINFOSTRUCT lpi )
{
	BOOL	flg;

	flg = FALSE;
	if( IsCheckedButton( hDlg, IDD_RGB ) )
		lpi->is_uRType = IDD_RGB;
	else if( IsCheckedButton( hDlg, IDD_RLE4 ) )
		lpi->is_uRType = IDD_RLE4;
	else if( IsCheckedButton( hDlg, IDD_RLE8 ) )
		lpi->is_uRType = IDD_RLE8;
	else if( IsCheckedButton( hDlg, IDD_PM ) )
		lpi->is_uRType = IDD_PM;
	else
		lpi->is_uRType = IDD_RGB;

	if( IsCheckedButton( hDlg, IDD_1 ) )
		lpi->is_uRBits = IDD_1;
	else if( IsCheckedButton( hDlg, IDD_4 ) )
		lpi->is_uRBits = IDD_4;
	else if( IsCheckedButton( hDlg, IDD_8 ) )
		lpi->is_uRBits = IDD_8;
	else if( IsCheckedButton( hDlg, IDD_24 ) )
		lpi->is_uRBits = IDD_24;
	else
		lpi->is_uRBits = IDD_24;

	if( IsCheckedButton( hDlg, IDC_CHECK1 ) )
		lpi->is_bOpenWnd = TRUE;
	else
		lpi->is_bOpenWnd = FALSE;

	if( GetDlgItemText( hDlg, IDC_EDIT1, &lpi->is_szName[0], MAX_PATH ) )
	{
		switch( lpi->is_uRType )
		{
		case IDD_RGB:
			lpi->is_dwType = BI_RGB;
			break;
		case IDD_RLE4:
			lpi->is_dwType = BI_RLE4;
			break;
		case IDD_RLE8:
			lpi->is_dwType = BI_RLE8;
			break;
		case IDD_PM:
			lpi->is_dwType = BI_PM;
			break;
		default:
			lpi->is_dwType = BI_RGB;
			break;
		}

		if( lpi->is_uRType == IDD_RLE4 )
			lpi->is_dwBits = 4;
		else if( lpi->is_uRType == IDD_RLE8 )
			lpi->is_dwBits = 8;
		else
		{
			switch( lpi->is_uRBits )
			{
			case IDD_1:
				lpi->is_dwBits = 1;
				break;
			case IDD_4:
				lpi->is_dwBits = 4;
				break;
			case IDD_8:
				lpi->is_dwBits = 8;
				break;
			case IDD_24:
				lpi->is_dwBits = 24;
				break;
			default:
				if( lpi->is_dwType == BI_RLE4 )
					lpi->is_dwBits = 4;
				else	//  if( lpi->is_dwType == BI_RLE8 )
					lpi->is_dwBits = 8;
				break;
			}

		}
		flg = TRUE;
	}
	return flg;
}

BOOL	Do_SA_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	bRet = FALSE;
	LPINFOSTRUCT	lpi;
   DWORD cmd = LOWORD(wParam);

	lpi = (LPINFOSTRUCT)GET_PROP( hDlg, ATOM_SAVEAS );
//	switch( wParam )
	switch( cmd )
	{
	case IDOK:
		if( lpi )
			EndDialog( hDlg, Do_SA_OK( hDlg, lpi ) );
		else
			EndDialog( hDlg, FALSE );
		break;

	case IDCANCEL:
		EndDialog( hDlg, FALSE );
		break;

	case IDC_BUTTON1:	// BROWSE
		Do_SA_Browse( hDlg );
		break;

	case IDC_BUTTON2:	// RESTORE
		if( lpi )
			Do_SA_Defaults( hDlg, lpi );
		break;

	case IDD_RGB:
		if( lpi )
		{
			lpi->is_uRType = IDD_RGB;
			// and with this any BIT choice is OK
		}
		break;
	case IDD_RLE4:		// User has CHOSEN
		if( lpi )
		{
			lpi->is_uRType = IDD_RLE4;
			// and BIT choice must agree
			CheckRadioButton( hDlg,	// handle to dialog box
				IDD_1,	// identifier of first radio button in group
				IDD_24,		// identifier of last radio button in group
				IDD_4 );	// identifier of radio button to select
		}
		break;
	case IDD_RLE8:		// User has CHOSEN
		if( lpi )
		{
			lpi->is_uRType = IDD_RLE8;
			// and BIT choice must agree
			CheckRadioButton( hDlg,	// handle to dialog box
				IDD_1,	// identifier of first radio button in group
				IDD_24,		// identifier of last radio button in group
				IDD_8 );	// identifier of radio button to select
		}
		break;
	case IDD_PM:
		if( lpi )
		{
			lpi->is_uRType = IDD_PM;
			// and with this any BIT choice is OK
		}
		break;
	case IDD_1:
		if( lpi )
		{
			lpi->is_uRBits = IDD_1;
			// and with this BIT choice
			if( ( lpi->is_uRType == IDD_RLE4 ) ||
				( lpi->is_uRType == IDD_RLE8 ) )
			{
				lpi->is_uRType = IDD_RGB;
				CheckRadioButton( hDlg,	// handle to dialog box
					IDD_RGB,	// identifier of first radio button in group
					IDD_PM,		// identifier of last radio button in group
					IDD_RGB );	// identifier of radio button to select
			}
		}
		break;
	case IDD_4:
		if( lpi )
		{
			lpi->is_uRBits = IDD_4;
			// and with this BIT choice
			if( lpi->is_uRType == IDD_RLE8 )
			{
				lpi->is_uRType = IDD_RGB;
				CheckRadioButton( hDlg,	// handle to dialog box
					IDD_RGB,	// identifier of first radio button in group
					IDD_PM,		// identifier of last radio button in group
					IDD_RGB );	// identifier of radio button to select
			}
		}
		break;
	case IDD_8:
		if( lpi )
		{
			lpi->is_uRBits = IDD_8;
			// and with this BIT choice
			if( lpi->is_uRType == IDD_RLE4 )
			{
				lpi->is_uRType = IDD_RGB;
				CheckRadioButton( hDlg,	// handle to dialog box
					IDD_RGB,	// identifier of first radio button in group
					IDD_PM,		// identifier of last radio button in group
					IDD_RGB );	// identifier of radio button to select
			}
		}
		break;
	case IDD_24:
		if( lpi )
		{
			lpi->is_uRBits = IDD_24;
			// and with this BIT choice
			if( ( lpi->is_uRType == IDD_RLE4 ) ||
				( lpi->is_uRType == IDD_RLE8 ) )
			{
				lpi->is_uRType = IDD_RGB;
				CheckRadioButton( hDlg,	// handle to dialog box
					IDD_RGB,	// identifier of first radio button in group
					IDD_PM,		// identifier of last radio button in group
					IDD_RGB );	// identifier of radio button to select
			}
		}
		break;

#ifdef	DBGSACMD	// Out some DEBUG stuff
	default:
		{
			LPSTR	lpd, lps;
			lpd = GetTmp1();
			lps = 0;
			switch( wParam )
			{
			case IDD_RGB:
				lps = "IDD_RGB";
				break;
			case IDD_RLE4:
				lps = "IDD_RLE4";
				break;
			case IDD_RLE8:
				lps = "IDD_RLE8";
				break;
			case IDD_PM:
				lps = "IDD_PM";
				break;
			case IDD_1:
				lps = "IDD_1";
				break;
			case IDD_4:
				lps = "IDD_4";
				break;
			case IDD_8:
				lps = "IDD_8";
				break;
			case IDD_24:
				lps = "IDD_24";
				break;
			}
			if( lps && *lps )
			{
				wsprintf( lpd,
					"DBGCMD: wP=%x (%s) lP=%x"MEOR,
					wParam,
					lps,
					lParam );
			}
			else
			{
				wsprintf( lpd,
					"DBGCMD: wP=%x lP=%x"MEOR,
					wParam,
					lParam );
			}
//			// Out debug stuff - DiagOut - TEMPDIAG.TXT output
			// ***********************************************
			DO(lpd);
		}
		break;
#endif	// DBGSACMD
	}
	return bRet;
}

//EXPORT32
//BOOL MLIBCALL IDM_SAVEASPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
UINT_PTR CALLBACK IDM_SAVEASPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	BOOL	bRet = FALSE;

	switch( msg )
	{
	case WM_INITDIALOG:
		bRet = Do_SA_Init( hDlg, wParam, lParam );
        break;

    case WM_COMMAND:
		bRet = Do_SA_Command( hDlg, wParam, lParam );
		break;

	case WM_DESTROY:
		REMOVE_PROP( hDlg, ATOM_SAVEAS );
		break;
	}
	return bRet;
}

BOOL	BuildRequestedDib( HWND hWnd, LPINFOSTRUCT lpis,
						  LPDIBINFO lpDIBInfo )
{
	BOOL		flg = FALSE;
	HANDLE		hDIB;
	LPSTR		lpf;

#ifdef	DBGSACMD	// Out some DEBUG stuff
	LPSTR		lpd, lpm1, lpm2, lpdib;
	HANDLE		hdib;
#endif	// DBGSACMD

	lpf = &lpis->is_szName[0];
#ifdef	DBGSACMD	// Out some DEBUG stuff
	lpd = GetTmp2();
	lpm1 = &lpd[256];
	lpm2 = &lpm1[32];
	if( ( hdib = lpDIBInfo->hDIB     ) &&
		 ( lpdib = DVGlobalLock(hdib) ) )   // LOCK DIB HANDLE
	{
		switch( lpis->is_dwType )
		{
		case BI_RGB:
			lstrcpy( lpm1, "BI_RGB" );
			break;
		case BI_RLE4:
			lstrcpy( lpm1, "BI_RLE4" );
			break;
		case BI_RLE8:
			lstrcpy( lpm1, "BI_RLE8" );
			break;
		case BI_PM:
			lstrcpy( lpm1, "BI_PM" );
			break;
		default:
			lstrcpy( lpm1, "UNKNOWN" );
			break;
		}
		wsprintf( lpd,
			"Creating new DIB using %s, %d bits"MEOR,
			lpm1,
			lpis->is_dwBits );
		DVGlobalUnlock(hdib);   // UNLOCK DIB HANDLE
		DO(lpd);
	}
	else
	{
		DO( "ERROR: No hDIB or LOCK FAILED!" );
	}
#endif	// DBGSACMD
	if( lpDIBInfo->hBitmap )
	{
		if( lpis->is_dwType == BI_PM )
		{
			hDIB = PMDibFromBitmap( lpDIBInfo->hBitmap,
				lpis->is_dwType,
				(WORD)lpis->is_dwBits,
				lpDIBInfo->hPal );
		}
		else
		{
			hDIB = WinDibFromBitmap( lpDIBInfo->hBitmap,
				lpis->is_dwType,
				(WORD)lpis->is_dwBits,
				lpDIBInfo->hPal );
		}
	}
	else
	{
		hDIB = CopyDIB( lpDIBInfo->hDIB );
	}
	if( hDIB )
	{
		if( WriteBMP( lpf, hDIB, TRUE ) )
		{
			if( lpis->is_bOpenWnd )
			{
         	PRDIB	prd = (PRDIB)MALLOC( sizeof(RDIB) );
            if(!prd)
               chkme( "C:ERROR: No memory!!!"MEOR );
            NULPRDIB(prd);

				prd->rd_hWnd = lpis->is_hMDIWnd;
            prd->rd_pTitle = gszRPTit;  // set buffers
            prd->rd_pPath  = gszRPNam;
            strcpy( gszRPTit, lpf );
            DVGetFullName2( gszRPNam, gszRPTit );
				//rd.rd_pFName = lpf;
				prd->rd_hDIB = hDIB;
				prd->rd_Caller = df_DUPE;	// It is LIKE Duplicate
				if( OpenDIBWindow2( prd ) )
				{
#ifdef	CHGADDTO
               ADD2LIST( prd );
               //PDI   pdi;
               //if( pdi = GETDI( rd.rd_hMDI ) )
               //{
               //   pdi->di_psMWL = AddToFileList4( &rd );
               //   RELDI(pdi);
               //}
               //else
               //{
               //   chkdi( __FILE__, __LINE__ );
               //}
					//AddToFileList( lpf );
#endif	// CHGADDTO
				}

            MFREE(prd);
			}
		}
		else
		{
			DIBError( ERR_WRITEDIB );	//"Error writing DIB file! "
			DVGlobalFree( hDIB );
		}
	}
	else
	{
		// Errors
		if( lpDIBInfo->hBitmap )
		{
			if( lpis->is_dwType == BI_PM )
			{
				DIBError( ERR_NO_PMDIB );	//"ERROR: Function PMDibFromBitmap FAILED to return a handle! "
			}
			else
			{
				DIBError( ERR_NO_WINDIB );	//"ERROR: Function WinDibFromBitmap FAILED to return a handle! "
			}
		}
		else
		{
			DIBError( ERR_NO_COPY );	//"ERROR: Copying of DIB handle FAILED!\r\nPerhaps insufficient memory! "
		}
	}

	return flg;
}

// ==============================================================
// BOOL Dv_IDM_SAVEAS( HWND hWnd )
//
// Special SAVE AS BMP Option, which PRESENTS Options for
// the TYPE of BMP to be SAVED
//
// ==============================================================
BOOL Dv_IDM_SAVEAS( HWND hWnd )
{
	BOOL			bRet = FALSE;
	HWND			hMDIWnd;
	HANDLE			hDibInfo;
	LPDIBINFO		lpDibInfo;
	LPINFOSTRUCT	lpis;
	int				iRet;
#ifndef	WIN32
	FARPROC			lpInfo;
#endif
	if( ( hMDIWnd = GetCurrentMDIWnd() ) &&
		( hDibInfo = GetWindowExtra( hMDIWnd, WW_DIB_HINFO) ) )
	{
		lpis = (LPINFOSTRUCT)GMLocalAlloc( LPTR, sizeof(INFOSTRUCT) );
		if( lpis )
		{
			// Get all the info on the current DIB Window.
			lpis->is_hDibInfo = hDibInfo;
			lpis->is_hMDIWnd = hMDIWnd;
			lpis->is_bOpenWnd = gfOpenWindow;	// &gfChgOW
#ifdef	WIN32
			iRet = DialogBoxParam( ghDvInst,
				MAKEINTRESOURCE( IDD_IDM_SAVEAS ),
				hMDIWnd,
				IDM_SAVEASPROC,	// lpInfo,
				(LPARAM) lpis );
#else	/* !WIN32 */
			lpInfo = MakeProcInstance( IDM_SAVEASPROC, ghDvInst );
			iRet = DialogBoxParam( ghDvInst,
				MAKEINTRESOURCE( IDD_IDM_SAVEAS ),
				hMDIWnd,
				lpInfo,
				(LPARAM) lpis );
			FreeProcInstance (lpInfo);
#endif	/* WIN32 y/n */
			if( ( iRet == TRUE                                  ) &&
				 ( lpDibInfo = (LPDIBINFO)DVGlobalLock(hDibInfo) ) )
			{
				BuildRequestedDib( hWnd, lpis, lpDibInfo );
				DVGlobalUnlock(hDibInfo);
				if( lpis->is_bOpenWnd )
				{
					if( !gfOpenWindow )
					{
						gfOpenWindow = TRUE;
						gfChgOW = TRUE;
					}
				}
				else
				{
					if( gfOpenWindow )
					{
						gfOpenWindow = FALSE;
						gfChgOW = TRUE;
					}
				}
			}
			GMLocalFree(lpis);
		}
		else
		{

		}
	}
	else
	{
		DIBError( ERR_NO_INFO );
	}
	return bRet;
}
// **************************************************************

#if   (defined(ADDREVIEW2) && defined(WIN32))
//extern	char	gszDiag[];	// abt 1024 General diag buffer
#ifndef  NDEBUG
extern	LPSTR	GetWMStg( HWND hWnd, UINT uMsg,
				 WPARAM wParam, LPARAM lParam );
#endif   // !NDEBUG
BOOL  bRetB = FALSE;
static   HWND  hwndFrame = 0; // of IDD_BITMAP
static   HWND  hwndParent = 0;
static   RECT  rcFrame;
static   TCHAR szNoFile[] = "No file chosen";   // text to draw
static   TCHAR szPFail[]  = "No image available"; // text to draw
static   TCHAR szPFail2[]  = "Image too big to load preview!"; // text to draw

// OPENFILENAME
VOID  ShwPOFN( LPOPENFILENAME pof )
{
   LPTSTR   lpf, lpt, lpd;
   LPTSTR   lpout = &gszDiag[0];
   lpf = pof->lpstrFile;
   lpt = pof->lpstrFileTitle; // = szFileTitle;
   lpd = (LPTSTR)pof->lpstrInitialDir; //  = pADir;	/* Should be SAVE(W) or OPEN(R) Directory */
	//of.lpstrTitle        = szTitle;
   *lpout = 0;
   if(lpf)
   {
      if( *lpf )
      {
         wsprintf(EndBuf(lpout),
            "lpf (%#x) is [%s] ",
            lpf,
            lpf );
      }
      else
      {
         wsprintf(EndBuf(lpout),
            "lpf (%#x) is <blank> ",
            lpf );
      }
   }
   else
   {
      strcat(lpout,"lpf is NULL ");
   }
   if(lpt)
   {
      if( *lpt )
      {
         wsprintf(EndBuf(lpout),
            "lpt (%#x) is [%s] ",
            lpt,
            lpt );
      }
      else
      {
         wsprintf(EndBuf(lpout),
            "lpt (%#x) is <blank> ",
            lpt );
      }
   }
   else
   {
      strcat(lpout,"lpt is NULL ");
   }
   if(lpd)
   {
      if(*lpd)
      {
         wsprintf(EndBuf(lpout),
            "lpd (%#x) is [%s] ",
            lpd,
            lpd );
      }
      else
      {
         wsprintf(EndBuf(lpout),
            "lpd (%#x) is <blank> ",
            lpd );
      }
   }
   else
   {
      strcat(lpout,"lpd is NULL ");
   }
   strcat(lpout,MEOR);
   sprtf(lpout);
}

VOID  SetDefText( HDC hdc )
{

   FillRect(hdc, &rcFrame, GetStockObject(BLACK_BRUSH) );
   SetTextColor(hdc, RGB(255,255,255));
   SetBkColor(hdc, RGB(0,0,0));
   DrawText( hdc, // handle to DC
      szNoFile,   // "No file chosen", // text to draw
      strlen(szNoFile), // text length
      &rcFrame,   // formatting dimensions
      DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
}

VOID  SetNoImageText( HDC hdc )
{
   FillRect(hdc, &rcFrame, GetStockObject(BLACK_BRUSH) );
   SetTextColor(hdc, RGB(255,255,255));
   SetBkColor(hdc, RGB(0,0,0));
   DrawText( hdc, // handle to DC
      szPFail,   // "No image available", // text to draw
      strlen(szPFail), // text length
      &rcFrame,   // formatting dimensions
      DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
}

VOID  SetImageBigText( HDC hdc )
{
   FillRect(hdc, &rcFrame, GetStockObject(WHITE_BRUSH) );
   SetBkColor  (hdc, RGB(255,255,255));
   SetTextColor(hdc, RGB(200,032,032));
   DrawText( hdc, // handle to DC
      szPFail2,   // = []  = "Image too big to load preview!"; // text to draw
      strlen(szPFail2), // text length
      &rcFrame,   // formatting dimensions
      DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
}

LPTSTR   GetFiltPtr( LPTSTR lpf, DWORD ui )
{
   LPTSTR   lps = lpf;
   DWORD    dwi, dwj;
   if( ui > 1 )
   {
Nxt_Stg:
      ui--; // reduce
      if( dwi = strlen(lps) )
      {
         if( dwj = strlen( &lps[dwi+1] ) )
         {
            if( lps[dwi + dwj + 2] )
               lps += (dwi + dwj + 2);
         }
      }
      if(ui > 1)
         goto Nxt_Stg;
   }
   return lps;
}

//BOOL  gbKeepAspect = TRUE;
//BOOL  gbChgKeepAsp = FALSE;

VOID  SetDestRect( PRECT prc, DWORD Width, DWORD Height )
{
   double   aspec = ( (double)Width / (double)Height );
   RECT     rc = *prc;
   INT      ht,wd, d;

   wd = (INT)((double)rc.bottom * aspec );
   ht = (INT)((double)rc.right  / aspec );
   if( wd <= rc.right )
      ht = rc.bottom;
   else
      wd = rc.right;

   // now have the window within a window width and height
   if( wd < rc.right )  // center for the width
   {
      d = (rc.right - wd) / 2;
      if(d)
      {
         rc.left  += d;       // set LEFT of paint
         rc.right -= (d * 2); // reduce WIDTH of paint
      }
   }
   if( ht < rc.bottom )
   {
      d = (rc.bottom - ht) / 2;
      if(d)
      {
         rc.top    += d;         // set TOP of paint
         rc.bottom -= (d * 2);   // reduce HEIGHT of paint
      }
   }

   *prc = rc;
}

VOID  ToggleBool( HWND hDlg, UINT ui, PBOOL pb, PBOOL pc )
{
   UINT  ut = IsDlgButtonChecked( hDlg,     // handle to dialog box
      ui );  // button identifier
   if( ut == BST_CHECKED )
   {
      if( !*pb )
      {
         *pb = TRUE;
         *pc = TRUE;
      }
   }
   else
   {
      if( *pb )
      {
         *pb = FALSE;
         *pc = TRUE;
      }
   }
}

//#define  CDI(a,b) CheckDlgButton( hDlg, a, ( b ? BST_CHECKED : BST_UNCHECKED ) )
//      ISCDI( IDC_KEEPASPECT, &gbKeepAspect, &gbChgKeepAsp );
//#define  ISCDI(a,b,c)   ToggleBool( hDlg, a, b, c )

//typedef struct tagBITMAPFILEHEADER { 
//  WORD    bfType; 
//  DWORD   bfSize; 
//  WORD    bfReserved1; 
//  WORD    bfReserved2; 
//  DWORD   bfOffBits; 
//} BITMAPFILEHEADER, *PBITMAPFILEHEADER; 
//typedef struct tagBITMAPINFOHEADER{
//  DWORD  biSize; 
//  LONG   biWidth; 
//  LONG   biHeight; 
//  WORD   biPlanes; 
//  WORD   biBitCount; 
//  DWORD  biCompression; 
//  DWORD  biSizeImage; 
//  LONG   biXPelsPerMeter; 
//  LONG   biYPelsPerMeter; 
//  DWORD  biClrUsed; 
//  DWORD  biClrImportant; 
//} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 
typedef struct tagBMPWINHDR {
   BITMAPFILEHEADER	bwh_bmfHeader;
   BITMAPINFOHEADER  bwh_binfHeader;
}BMPWINHDR, * PBMPWINHDR;
static BMPWINHDR _s_bmpwinhdr; // FIX20061105

LPTSTR   GetDIBHdr( LPTSTR lpf )
{
   FILE * fp  = fopen(lpf, "rb");
   LPTSTR lps = 0;   // (LPTSTR)&_s_bmfHeader;
   if(fp)
   {
      size_t  size = sizeof(BMPWINHDR);

      lps = (LPTSTR)&_s_bmpwinhdr;

      ZeroMemory(lps,size);

      if( fread( lps, 1, size, fp ) == size )
      {
         PBITMAPFILEHEADER pbmpfh = (PBITMAPFILEHEADER)lps;
         if(pbmpfh->bfType == DIB_HEADER_MARKER)
         {
            PBITMAPINFOHEADER pbmpih = (PBITMAPINFOHEADER)((PBITMAPFILEHEADER) pbmpfh + 1);
            if( pbmpih->biSize == sizeof(BITMAPINFOHEADER) )
               lps = (LPTSTR)pbmpih;
            else
               lps = 0;
         }
         else
            lps = 0;
      }
      else
         lps = 0;


      fclose(fp);
   }
   return lps;
}
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : EXPOPENHOOKPROC
// Return type: UINT_PTR CALLBACK 
// Arguments  : HWND hDlg == handle to child dialog box
//            : UINT uiMsg == message identifier 
//            : WPARAM wParam == message parameter 
//            : LPARAM lParam == message parameter
// Description: Handle CALLBACKs from the Open-File dialog
//              and show a 'preview' of the image.
///////////////////////////////////////////////////////////////////////////////
UINT_PTR CALLBACK EXPOPENHOOKPROC(
  HWND hDlg,      // handle to child dialog box
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
)
{
   UINT_PTR iRet = 0;
   HDC      hdc;
   LPTSTR   lptmp;
   INT      ir;
   LPTSTR   lpfilt;
   LPOPENFILENAME pofn;
#ifndef  NDEBUG
   if( ( uiMsg != WM_CTLCOLORSTATIC ) &&
       ( uiMsg != WM_NCHITTEST      ) &&
       ( uiMsg != WM_SETCURSOR      ) &&
       ( uiMsg != WM_MOUSEMOVE      ) )
   {
      sprtf( "Hook: h=%#x %s wP=%#x lP=%#x."MEOR, hDlg,
         GetWMStg( hDlg, uiMsg, wParam, lParam ),
         wParam, lParam );
   }
#endif   // #ifndef  NDEBUG

   switch(uiMsg)
   {
   case WM_INITDIALOG:
      lptmp = &gszTmp[0];
      *lptmp = 0;
      hwndParent = GetParent(hDlg);
      hwndFrame = GetDlgItem(hDlg, IDD_BITMAP); 
      if(hwndFrame)
      {
         GetClientRect(hwndFrame, &rcFrame);
         sprtf( "Got parent h=%#x frame h=%#x, rect %s."MEOR,
            hwndParent,
            hwndFrame,
            Rect2Stg( &rcFrame ) );
      }
      SetDlgItemText(hDlg, IDD_FORMAT, lptmp);
      SetDlgItemText(hDlg, IDD_WIDTH,  lptmp);
      SetDlgItemText(hDlg, IDD_HEIGHT, lptmp);
//      CDI( IDC_KEEPASPECT, gbKeepAspect );
//      ISCDI( IDC_KEEPASPECT, &gbKeepAspect, &gbChgKeepAsp );
      break;

   case WM_PAINT:
      if(hwndFrame)
      {
         if( hdc = GetDC(hwndFrame) )
         {
            SetDefText(hdc);
            ReleaseDC(hwndFrame,hdc);
            sprtf( "Filled hdc %#x rc %s with BLACK."MEOR,
               hdc,
               Rect2Stg( &rcFrame ) );
         }
      }
      break;

//#define CDN_FIRST   (0U-601U)
//#define CDN_LAST    (0U-699U)
// Notifications from Open or Save dialog
//#define CDN_INITDONE            (CDN_FIRST - 0x0000)
//#define CDN_SELCHANGE           (CDN_FIRST - 0x0001)
//#define CDN_FOLDERCHANGE        (CDN_FIRST - 0x0002)
//#define CDN_SHAREVIOLATION      (CDN_FIRST - 0x0003)
//#define CDN_HELP                (CDN_FIRST - 0x0004)
//#define CDN_FILEOK              (CDN_FIRST - 0x0005)
//#define CDN_TYPECHANGE          (CDN_FIRST - 0x0006)
//#define CDN_INCLUDEITEM         (CDN_FIRST - 0x0007)
   case WM_NOTIFY:
      {
         //DWORD    idctl = (DWORD)wParam;
         LPNMHDR     pnmh  = (LPNMHDR)lParam;
         LPOFNOTIFY  pofnn = (LPOFNOTIFY)pnmh;
         LPTSTR      pszFile;  // May be NULL

         pofn = pofnn->lpOFN;
         pszFile = pofnn->pszFile;  // May be NULL
         switch( pnmh->code )
         {
         case CDN_INITDONE:
            lptmp = &gszTmp[0];
            *lptmp = 0;
            if( pofn )
            {
               ir = pofn->nFilterIndex;
               if( ( lpfilt = (LPTSTR)pofn->lpstrFilter ) &&  // = pszFilter;
                   ( lpfilt = GetFiltPtr(lpfilt, ir) ) &&
                   ( *lpfilt ) )
               {
                     wsprintf(lptmp,"Filter%d:",ir);
                     strcat(lptmp,lpfilt);
                     strcat(lptmp," ");
                     strcat(lptmp, &lpfilt[ (strlen(lpfilt) + 1) ]);
               }
            }
            SetDlgItemText(hDlg,IDD_FORMAT,lptmp);
            sprtf( "Note: CDN_INITDONE. pofn is %#x (%s)"MEOR,
               pofn,
               lptmp );
            break;

         case CDN_TYPECHANGE:
            lptmp = &gszTmp[0];
            *lptmp = 0;
            //else if( pnmh->code == CDN_TYPECHANGE )
            {
               if(pofn)
               {
                  ir = pofn->nFilterIndex;
                  if( ( lpfilt = (LPTSTR)pofn->lpstrFilter ) &&  // = pszFilter;
                      ( lpfilt = GetFiltPtr(lpfilt, ir) ) &&
                      ( *lpfilt ) )
                  {
                     wsprintf(lptmp,"Filter%d:",ir);
                     strcat(lptmp,lpfilt);
                     strcat(lptmp," ");
                     strcat(lptmp, &lpfilt[ (strlen(lpfilt) + 1) ]);
                  }
                  else
                  {
                     wsprintf(lptmp,"Filter index %d.", ir);
                  }
                  sprtf( "CDN_TYPECHANGE: %d. I=%d [%s]"MEOR,
                        pnmh->code,
                        ir,
                        lptmp);
               }
               else
               {
                  sprtf( "CDN_TYPECHANGE: %d. pofn is NULL!"MEOR,
                     pnmh->code );
               }
            }

            SetDlgItemText(hDlg,IDD_FORMAT,lptmp);
            *lptmp = 0;
            SetDlgItemText(hDlg,IDD_NAME,  lptmp);

            break;

         case CDN_SELCHANGE:
            lptmp = &gszTmp[0];
            *lptmp = 0;
            SetDlgItemText(hDlg, IDD_WIDTH,  lptmp);
            SetDlgItemText(hDlg, IDD_HEIGHT, lptmp);
            SetDlgItemText(hDlg, IDD_COLORS, lptmp);
            //else  // if( pnmh->code == CDNSELCHANGE )
            {
               sprtf( "CDN_SELCHANGE: %d."MEOR, pnmh->code );
            }

            hdc = GetDC(hwndFrame); // IDD_BITMAP frame
            if( hwndParent )  // = GetParent(hDlg) )
            {
               LPWIN32_FIND_DATA pfd   = &gfd;
               LPTSTR            lpf   = &pfd->cFileName[0];   // gszSelFile[0];
               DWORD             dwi;
               *lpf = 0;
               ir = SendMessage( hwndParent, // handle to destination window
                  CDM_GETSPEC,     // message to send
                  (WPARAM) 256,    // size of buffer
                  (LPARAM) lpf );  // file name buffer (LPTSTR)
               sprtf( "CDM_GETSPEC to %#x returned [%s] (%d)."MEOR,
                  hwndParent,
                  lpf,
                  ir );
               if( ir > 0 )
               {
                  strcpy(gszSelFile, gszFolder);
                  if( *lpf )
                  {
                     strcat(gszSelFile,"\\");
                     strcat(gszSelFile,lpf );
                  }

                  lpf = &gszSelFile[0];
                  dwi = IsValidFile( lpf, pfd );
                  ir = 0;
                  if( dwi & IS_FILE )
                  {
                     LARGE_INTEGER li;
                     li.LowPart  = pfd->nFileSizeLow;
                     li.HighPart = pfd->nFileSizeHigh;
                     sprintf( lptmp,
                        "[%s] %s bytes at %s",
                        lpf,
                     GetI64Stg( &li ),
                     GetFDTStg( &pfd->ftLastWriteTime ) );
                     if( pfd->nFileSizeHigh ||
                        (pfd->nFileSizeLow > (2000000 * 2)) )
                     {
                        dwi |= 0x80000000;
                     }
                  }
                  else if( dwi & IS_FOLDER )
                  {
                     sprintf( lptmp,
                        "[%s] Is a valid FOLDER",
                        lpf );
                  }
                  else
                  {
                     sprintf( lptmp,
                        "[%s] *** IS NOT A VALID FOLDER/FILE! ***",
                        lpf );
                  }

                  if( dwi & IS_FILE )   // IDD_BITMAP frame
                  {
                     HANDLE   hDIB;
                     LPTSTR   lpDIB;
                     ir = GDI_ERROR;
                     if( hdc )
                     {
                        if( dwi & 0x80000000 )  // if too big ie VERY large, do NOT waste time
                        {
                           lpDIB = GetDIBHdr( lpf );
                           if(lpDIB)
                           {
                              LPTSTR   pTmp = gszTmpBuf1;
                              gdwDIBWidth  = DIBWidth(lpDIB);
                              gdwDIBHeight = DIBHeight(lpDIB);
                              gdwDIBBPP    = DIBBitCount(lpDIB);
                              sprintf( pTmp, "Width %d pixels", gdwDIBWidth );
                              SetDlgItemText(hDlg, IDD_WIDTH,  pTmp);
                              sprintf( pTmp, "Height %d pixels", gdwDIBHeight );
                              SetDlgItemText(hDlg, IDD_HEIGHT,  pTmp);
                              sprintf( pTmp, "%d BPP", gdwDIBBPP );
                              SetDlgItemText(hDlg, IDD_COLORS,  pTmp);
                           }
                           SetImageBigText( hdc );  // IDD_BITMAP frame
                        }
                        else
                        {
                           hDIB = GetDIB( lpf, gd_DP ); 
                           if(hDIB)
                           {
                              lpDIB = DVGlobalLock(hDIB); 
                              if(lpDIB)
                              {
                                 RECT  rc = rcFrame;
                                 gdwDIBWidth  = DIBWidth(lpDIB);
                                 gdwDIBHeight = DIBHeight(lpDIB);
                                 gdwDIBBPP    = DIBBitCount(lpDIB);
                                 FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH) );
                                 //ISCDI( IDC_KEEPASPECT, &gbKeepAspect, &gbChgKeepAsp );
                                 if( gbKeepAspect ) {
                                    SetDestRect( &rc, gdwDIBWidth, gdwDIBHeight );
                                 }
                        	      SetStretchBltMode( hdc, COLORONCOLOR );
                                 ir = StretchDIBits( hdc,		// hDC
                                    rc.left,		// DestX
                                    rc.top,     // DestY
                                    rc.right,   // nDestWidth
                                    rc.bottom,  // nDestHeight
                                    0,    // SrcX
                                    0,    // SrcY
                                    gdwDIBWidth,   // wSrcWidth
                                    gdwDIBHeight,  // wSrcHeight
                                    FindDIBBits(lpDIB),  
                                    (LPBITMAPINFO)lpDIB, // lpBitsInfo
                                    DIB_RGB_COLORS,		// wUsage
                                    SRCCOPY );
                                 DVGlobalUnlock(hDIB);
                              }
                              DVGlobalFree(hDIB);
                           }
                           if( ir == GDI_ERROR )
                           {
                              SetNoImageText(hdc);
                           }
                           else
                           {
                              sprintf( gszTmpBuf1, "Width %d pixels", gdwDIBWidth );
                              SetDlgItemText(hDlg, IDD_WIDTH,  gszTmpBuf1);
                              sprintf( gszTmpBuf1, "Height %d pixels", gdwDIBHeight );
                              SetDlgItemText(hDlg, IDD_HEIGHT,  gszTmpBuf1);
                              sprintf( gszTmpBuf1, "%d BPP", gdwDIBBPP );
                              SetDlgItemText(hDlg, IDD_COLORS,  gszTmpBuf1);
                           }
                        }
                     }
                  }
                  else if(hdc)
                  {
                     SetDefText(hdc);
                  }
               }
            }

            SetDlgItemText(hDlg, IDD_NAME, lptmp);

            if(hdc)
               ReleaseDC(hwndFrame,hdc);
            break;

         case CDN_FOLDERCHANGE:
            lptmp = &gszTmp[0];
            *lptmp = 0;
            //if( pnmh->code == CDN_FOLDERCHANGE )
            {
               ir = SendMessage( hwndParent,   // handle to destination window
                  CDM_GETFOLDERPATH,          // message to send
                  (WPARAM) 256,          // size of buffer
                  (LPARAM) lptmp );  // file name buffer (LPTSTR)
               sprtf( "CDN_FOLDERCHANGE: %d. to [%s] (ir=%d)"MEOR,
                  pnmh->code,
                  lptmp,
                  ir );
               if( ir > 0 )
               {
                  strcpy( gszFolder, lptmp );
               }
               else
               {
                  *lptmp = 0;
               }
            }

            SetDlgItemText(hDlg, IDD_NAME, lptmp);
            break;

         case CDN_SHAREVIOLATION:
               sprtf( "Note: CDN_SHAREVIOLATION."MEOR );
            break;
         case CDN_HELP:
               sprtf( "Note: CDN_HELP."MEOR );
            break;
         case CDN_FILEOK:
               sprtf( "Note: CDN_FILEOK."MEOR );
               if(pofn)
                  ShwPOFN( pofn );
            break;
//         case CDN_TYPECHANGE:
//               sprtf( "Note: CDN_TYPECHANGE."MEOR );
//            break;
         case CDN_INCLUDEITEM:
               sprtf( "Note: CDN_INCLUDEITEM."MEOR );
            break;
         default:
            {
               sprtf( "Note from %d from h = %#x id %d code %d."MEOR,
                  wParam,
                  pnmh->hwndFrom,
                  pnmh->idFrom,
                  pnmh->code );
            }
            break;
         }
      }
      break;

   case WM_DESTROY:
      hwndFrame  = 0;
      hwndParent = 0;
      break;
   }

   return iRet;
}

#endif   // ADDREVIEW2 & WIN32

// EOF - DvFile.c
