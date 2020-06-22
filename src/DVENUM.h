

// DvEnum.h
#ifndef	_DvEnum_h
#define	_DvEnum_h

// Defined function codes
// ======================
#define	ENU_RESTORE			1
#define	ENU_COUNT			2
#define	ENU_FINDFILES		3
#define	ENU_FINDHWND		4	// Check if a CHILD is still alive

typedef	struct tagENUSTR {  /* es */
	DWORD	es_code;
	HGLOBAL	es_hand;
	char	es_string[MAX_PATH];
	HWND	es_hwnd;
	DWORD	es_res1;
	DWORD	es_res2;
}ENUSTR;
typedef ENUSTR FAR * PENUSTR;

extern	void	EnumAllKids( PENUSTR );

#endif	// _DvEnum_h
// end - DvEnum.h
