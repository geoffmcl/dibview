
// DVList.h		// was ewmList.h
#ifndef	_DVList_H
#define	_DVList_H

#define  LE    LIST_ENTRY  // Added - 20 September, 2002
#define  PLE   PLIST_ENTRY    // THE LATEST - 29 November 2000

extern   HLOCAL	GMLocalAlloc( UINT Type, size_t Size );
extern   HLOCAL	GMLocalFree( HLOCAL hLoc );

//#define  MALLOC(a)      malloc(a)
//#define  MFREE(a)       free(a)
#ifdef   NDEBUG
#define  MALLOC(a)      GMLocalAlloc( LPTR, a )
#define  MFREE(a)       GMLocalFree(a)
#else // _DEBUG is ON
#ifdef   DBGMEM2
// big DEBUG stuff
extern   LPTSTR   GetDBStg( LPTSTR lpm, INT iln );
#define  MALLOC(a)      DVGAlloc( GetDBStg(__FILE__, __LINE__), GPTR, a )
#define  MFREE(a)       DVGFree(a)
#else // !DBGMEM2
// just use the LOCAL function
#define  MALLOC(a)      GMLocalAlloc( LPTR, a )
#define  MFREE(a)       GMLocalFree(a)
#endif   // DBGMEM2 y/n
#endif   // NDEBUG y/n


// bit values for DWORD wl_dwFlag
#define  flg_IsValid    0x00000001     // have checked validity, and is valid
#define  flg_NotValid   0x00000002     // checked validity and NOT valid file
#define  flg_IsLoaded   0x00000004     // currently LOADED in a child window
#define  flg_NoLoad     0x00000008     // load attempted, but FAILED
#define  flg_MDIOpen    0x00000010     // is in an OPEN MDI window
#define  flg_MDIFail    0x00000020     // tried to open a MDI, but FAILED
#define  flg_Reviewed   0x00000040     // reviewed for OPEN in timer

#define  flg_InAuto     0x00001000     // being processed by AUTO LOAD

#define  flg_SrchUsed   0x00100000     // was USED as a "search" mask
#define  flg_SrchOK     0x00200000     // and such a "search" yielded results

#define  flg_ToDelete   0x20000000     // IMD_OPTION4 - Marked for deletion
#define  flg_MarkDel    0x40000000     // temporary marking
#define  flg_IsDeleted  0x80000000     // deleted, but left in list

// any one of these and NOT written to INI
#define  flg_DELETE     ( flg_ToDelete | flg_MarkDel | flg_IsDeleted )

typedef struct tagMYWORKLIST {
   LIST_ENTRY  wl_sList;      // Flink and Blink members - MUST BE FIRST
   HWND        wl_hMDI;       // handle of MDI child where loaded
   DWORD       wl_dwFlag;     // various flags as above
   DWORD       wl_dwLen;      // length of file name
   TCHAR       wl_szFile[264]; // actual full path name (usually)
}MWL, * PMWL;

#define  Ptr2PWL(a)     (PMWL)a


// Saved as dc4wList.h
// (was Fc4wList.h, FixFList.h, DumpList.h and ewmList.h)

#ifndef	_dc4wList_H
#define	_dc4wList_H

// copied from fc4wList.h on 16 April, 2001
// copied from Fa4List.h on 4 April, 2001 and added InsertHeadList()
// Copied from FixfList.h on 15 March, 2001 for YAHU project

#ifdef  __cplusplus
extern  "C"
{
#endif  // __cplusplus


#ifndef  PLE
#define  PLE   PLIST_ENTRY
#endif   // !PLE

#define	ListCount(ListHead)\
{\
	int	_icnt = 0;\
	PLIST_ENTRY _EX_Flink;\
	PLIST_ENTRY _EX_ListHead;\
	_EX_ListHead = (ListHead);\
	_EX_Flink = _EX_ListHead->Flink;\
	while( _EX_Flink != _EX_ListHead )\
	{\
		_icnt++;\
		_EX_Flink = _EX_Flink->Flink;\
	}\
}

//    ((ListHead)->Flink == (ListHead))
#define ListCount2(lh,pi) \
{\
    int             _icnt = 0;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (lh);\
    _EX_Flink = _EX_ListHead->Flink;\
    while( _EX_Flink != _EX_ListHead )\
        {\
                _EX_Flink = _EX_Flink->Flink;\
                _icnt++;\
                if( ( _icnt == 0 ) ||\
                        ( !_EX_Flink ) )\
                {\
                                break;\
                }\
        }\
        *pi = _icnt;\
}


//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
// empty if FORWARD link points to SELF!
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

#define IsListHead(ListHead,ListNext) ((ListHead)->Flink == ListNext)
//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _RT_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
	_RT_Flink = _EX_Flink->Flink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    _RT_Flink;}

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
// simple. the current BACKWARD link of the HEAD (EX_Blink)
// becomes this entries BACKWARDS link (entry)->Blink.
// and this entried FORWARD link is the HEAD
// this etry is store in the FORWARD link of the last HEAD Blink,
// and stored as the BACKWARDS link of the HEAD.

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

// Insert this pEntry as the FIRST IN THE LIST
// 1. Extract current Flink of the pHead
// 2. Extract current Blink of current FLink;
// 3. Set pEntry as Flink of pHead
// 4. Set pEntry as Blink of previous Flink
// 5. Set Blink of pEntry as pHead
// 6. Set Flink of pEntry previous Flink
#define InsertHeadList(pHead,pEntry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = pHead->Flink;\
    _EX_Blink = _EX_Flink->Blink;\
    pHead->Flink     = (pEntry);\
    _EX_Flink->Blink = (pEntry);\
    (pEntry)->Blink = pHead;\
    (pEntry)->Flink = _EX_Flink;\
    }

// Insert Entry as Blink of List
// =============================
// Extract the Blink from the List
// Extract the FLink from the Blink
// Insert  the Entry as the Flink
// Put the previous Flink and Entry Flink
#define InsertBefore(List,Entry) {\
    PLE _EX_Blink;\
    PLE _EX_Flink;\
    PLE _EX_List;\
    _EX_List = (List);\
    _EX_Blink = _EX_List->Blink;\
    _EX_Flink = _EX_Blink->Flink;\
    _EX_Blink->Flink = (Entry);\
    _EX_List->Blink = (Entry);\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_Blink;\
    }

// Insert Entry as Flink of List
// =============================
// extract Flink from list
// extract Blink from Flink
// make Entry the Flink of list
// and  Entry the Blink of Flink
// make Flink of list Flink of Entry
// and  Blink of Flink as Blink of Entry
#define InsertAfter(List,Entry) {\
    PLE _EX_Blink;\
    PLE _EX_Flink;\
    PLE _EX_List;\
    _EX_List = (List);\
    _EX_Flink = _EX_List->Flink;\
    _EX_Blink = _EX_Flink->Blink;\
    _EX_List->Flink = (Entry);\
    _EX_Flink->Blink = (Entry);\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_Blink;\
    }

//#define	Traverse_List2(ListHead,ListEntry)
//	for(PLIST_ENTRY _s_ListHead = (ListHead), _s_Flink = _s_ListHead->Flink, ListEntry = _s_Flink;_s_Flink != _s_ListHead; _s_Flink = _s_Flink->Flink, ListEntry = _s_Flink )
//static PLIST_ENTRY _SX_Flink;
//static PLIST_ENTRY _SX_ListHead;
//#define	Traverse_List(ListHead)	for(_SX_ListHead = (ListHead),_SX_Flink = _SX_ListHead->Flink;_SX_Flink != _SX_ListHead; _SX_Flink = _SX_Flink->Flink )
#define  Traverse_List(pListHead,pListNext)\
   for( pListNext = pListHead->Flink; pListNext != pListHead; pListNext = pListNext->Flink )

#define  InitLList(a)\
   {\
      (a)->Flink = a;\
      (a)->Blink = a;\
   }

#define  FreeLList(pListHead,pListNext)\
         while( !IsListEmpty( pListHead ) )\
         {\
            pListNext = RemoveHeadList(pListHead);\
            MFREE(pListNext);\
         }

#define  KillLList(a)\
   {\
      PLE   _pNxt;\
      FreeLList(a,_pNxt);\
   }


#ifdef  __cplusplus
}
#endif  // __cplusplus

#endif	// #ifndef	_dc4wList_H
// eof - dc4wList.h (was fc4wList.h, FA4List.h, YahuList.h, FixFList.h and earlier)
#endif	// ifndef	_DVList_H
// eof - DVList.h (was ewmList.h)
