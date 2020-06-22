
// Dlgopen.h
#ifndef  _Dlgopen_HH
#define  _Dlgopen_HH

#ifdef  __cplusplus
extern "C" {
#endif

/********** THE FOLLOWING ARE USED IN DLGOPEN.C  ************************/

/* IDs for controls in the DlgOpen dialog - see ..\resource.h */

#define DLGOPEN_OPTION          0xF000
#define DLGOPEN_1BPP            0x0001
#define DLGOPEN_4BPP            0x0002
#define DLGOPEN_8BPP            0x0004
#define DLGOPEN_24BPP           0x0008
#define DLGOPEN_RLE4            0x0010
#define DLGOPEN_RLE8            0x0020
#define DLGOPEN_RGB             0x0040

#define DLGOPEN_OPTION8         0x0080

/*  flags:
 *     The LOWORD is the standard FileOpen() flags (OF_*)
 *     the HIWORD can be any of the following:
 */
#define OF_MUSTEXIST    0x00010000  /* file must exist if the user hits Ok    */
#define OF_NOSHOWSPEC   0x00020000  /* DO NOT Show search spec in the edit box*/
#define OF_SHOWSPEC     0x00000000  /* Show the search spec in the edit box   */
#define OF_SAVE         0x00040000  /* Ok button will say "Save"              */
#define OF_OPEN         0x00080000  /* Ok button will say "Open"              */
#define OF_NOOPTIONS    0x00100000  /* Disable the options fold out           */

/* Attributes for DlgDirLst() */
#define ATTRFILELIST    0x0000        /* include files only          */
#define ATTRDIRLIST     0xC010        /* directories and drives ONLY */
#define CBEXTMAX        6             /* Number of bytes in "\*.txt" */


#define IDF(id)     ((id) & ~DLGOPEN_OPTION)  /* extracts flag from control ID */
#define FID(f)      ((f)  |  DLGOPEN_OPTION)  /* extracts control ID from flag */

#ifdef  __cplusplus
//extern "C" {
}
#endif

#endif   // #ifndef  _Dlgopen_HH
// eof - Dlgopen.h
