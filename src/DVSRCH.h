
// DvSrch.h (was srsearch.h)
#ifndef	_DvSrch_h
#define	_DvSrch_h

/* ==============================
 * 	Dos structure - at DTA
 *	----------------------
 *dsrchstr	struc
 * dsr_resv	db	21 dup(?)
 * dsr_attr	db	?
 * dsr_wtime	dw	?
 * dsr_wdate	dw	?
 * dsr_size	dd	?
 * dsr_name	db	13 dup(?)
 *dsrchstr	ends
 * ============================== */

typedef	struct tagFILEFIND {	/* ff */
	int	ffAttr;
	unsigned int	ffTime;
	unsigned int	ffDate;
	unsigned long	ffSize;
	unsigned int	ffFNLen;
	char	ffByte[260];
} FILEFIND;

typedef FILEFIND MLPTR LPFILEFIND;

typedef	struct tagDGETFILE {	/* gf */
	int	gfMxCnt;
	int	gfFCount;
	int	gfDCount;
	int	gfAttr;
	char	gfMask[260];
	char	gfDire[260];	/* And the DIRECTORY "global" mask ... */
	LPFILEFIND	gfName;	/* For full file name, attribute, etc ... */
} DGETFILE;

typedef DGETFILE MLPTR LPDGETFILE;

/* DOS File System Attributes */
#define	dfReadOnly		0x01		/* Read ONLY */
#define	dfHidden			0x02		/* Hidden */
#define	dfSystem			0x04		/* System File */
#define	dfVolume			0x08		/* Volume label */
#define	dfDirectory		0x10		/* Directory */
#define	dfArchive		0x20		/* Archive attribute */

#endif	// _DvSrch_h
// eof - DvSrch.h (was srsearch.h)
// Note: must change the ASM 16-bit code

