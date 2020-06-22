
// wjpeglib.h

#ifndef	_wjpeglib_h
#define	_wjpeglib_h

// Modification History
// ====================
//
// Updated - April, 1997 - Support for Black & White and
//	TRANSPARENT GIF's added.
//
// For INDIRECT Library load, and use GetProcAddress( ), to
// resolve the POINTER ...
//
// Add in cplusplus support - 16 May 1997
//

#ifdef	__cplusplus
extern	"C"
{
#endif	// __cplusplus

#define	PROBGOBJ	// Have a problem with return of GLOBAL OBJECT
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define	WJPEG4		// NOTE: Certain older FILE headers REMOVED
#define	BANDWGIF	// Black and White GIF files.
#define	TRANSGIF	// TRANSPARENT GIF to Bitmap.
#undef	PARTGIF		// Support passing PARTIAL DATA
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

#define	ADDOLDTO	// Add back FILE interfaces

// Primary - Either 32-bit, or more if available, needed,
// so everything is NEAR in the FLAT Model of MS Windows.
// This should be enough memory, without adding 64-bit overhead.
// ======================== at this time
#ifdef	WIN32
#define	MLPTR		*
#define	MHPTR		*
#define	MLIBCONV	PASCAL
#define	MLIBCALL	MLIBCONV
#else	// !WIN32 = WIN16
// BUT 16-Bit model reverts to ES:[BX], 20-bits, DOS real mode,
// which has the limit of 65,000 WORD handles, maximum!!!
// This is extra overhead, and should be AVOIDED, where poss.
// ie Use 32-bits in 32-bit eax, ebx, regs of Ix8x (80x86 compat.)
//
// On the other hand, if what you are doing does does not have
// a choice, THEN there is the extra overhead of emulating
// the 20-bit shifting in 16-bit Windows .... Avoid ...
// If poss. do not define PINT8 as NEAR (unsigned) pointer!.
// ==========================================================
#define	MLPTR		FAR *
#define	MHPTR		huge *
#define	MLIBCONV	FAR PASCAL
#define	MLIBCALL	MLIBCONV __loadds 
#endif	// WIN32 y/n

#ifndef RC_INVOKED
//
// No resourses, so perhaps speed Resource COMPILE = .Res out
//

// WINDOWS DLL considerations
// In 16-Bit Windows the EXPORTED DLL functions are placed in
// the DEFINITIONS (.DEF) file, but in Windows 32-bit EXPORTED
// functions must have an EXPORT32 monicer ... So ...
//

// IFF a constant reminder is needed,
// uncomment the following pragma...
#if defined( WIN32 )

//#pragma message( "DLL: WIN32 Port ..." )
#define EXPORT32 __declspec(dllexport)

#else	// !WIN32

//#pragma message( "DLL: 16-Bit Port ..." )
#define EXPORT32

#endif	// WIN32 y/n

// SOME ONLY Available in FULL Library, and some NEED
// a SPECIAL switch to bring them back

// Then a SPECIALISED BMP to JPEG Service
EXPORT32
WORD MLIBCALL WBMPTOJPG( HGLOBAL, DWORD, HGLOBAL, LPSTR );

typedef WORD (MLIBCONV *LPWBMPTOJPG) ( HGLOBAL, DWORD, HGLOBAL, LPSTR );


// NOTE WELL: These should be RETIRED soonest.
// There is no need for the Library to do FILE I/O
// TO BE RETIRED SOONEST
// =====================
#ifdef	ADDOLDTO

EXPORT32
HGLOBAL MLIBCALL WGIFTOBMP( HGLOBAL, DWORD, HGLOBAL, LPSTR );
typedef HGLOBAL (MLIBCONV *LPWGIFTOBMP) ( HGLOBAL, DWORD, HGLOBAL, LPSTR );

EXPORT32
HGLOBAL MLIBCALL WJPGTOBMP( HGLOBAL, DWORD, HGLOBAL, LPSTR );
typedef HGLOBAL (MLIBCONV *LPWJPGTOBMP) ( HGLOBAL, DWORD, HGLOBAL, LPSTR );
// =====================

#endif	// ADDOLDTO


// NEW Functions
// =============
// Two SIZE Functions

EXPORT32
DWORD MLIBCALL WGIFSIZE( HGLOBAL, DWORD, LPSIZE );

EXPORT32
DWORD MLIBCALL WJPGSIZE( HGLOBAL, DWORD, LPSIZE );

// Where :-
// HGLOBAL	hgInData;		// GIF or JPG INPUT data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// LPSIZE		lpCxCy;		// OUPUT Width and Height. (May be NULL).
// Description :-
// In these functions, the HGLOBAL and DWORD pass (some or all) the 
// DATA of the GIF and JPG respectively, and the functions return 
// the DWORD size of the Buffer required to HOLD the completed DIB.
// If some error occurs, like NOT valid GIF or JPG data, or the 
// size is in error or insufficient, etc, the functions would return
// ZERO. No error information is supplied. 
//
// If the function is successful, and if LPSIZE was not a NULL then 
// the int cx; and int cy; members of the tagSIZE structure would be 
// filled with the WIDTH and HEIGHT of the final DIB.
//
// ==========================================================

// Two Conversion Functions

EXPORT32
BOOL MLIBCALL WGIF2BMP( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

EXPORT32
BOOL MLIBCALL WJPG2BMP( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

// Where :-
// HGLOBAL	hgInData;		// The full GIF or JPG data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// HGLOBAL	hgInfo;		// INFORMATION Buffer (if not NULL).
// HGLOBAL	hgOutData;	// Handle of OUPUT memory.
// Description :-
// The hgInData contains the FULL GIF or JPG data, and ddDataSzie is the
// length of this data.
// hgInfo may be NULL, else if an error occurs ASCII Information will be
// placed in this buffer up to a maximum length of 1024 bytes.
// hgOutData must realise an OUPUT Buffer sufficient in SIZE to completely
// hold the resultant BMP. The structure of this output buffer will be -
// BITMAPINFOHEADER bmih;
// RGBQUAD aColors[];
// BYTE    aBitmapBits[];
// The data in this Ouput buffer will only be valid if the function returns
// FALSE (0). Otherwise an ERROR Number as shown below will be returned.
//
// ============================================================

// And for static library loading ... as in DibView
typedef DWORD (MLIBCONV *LPWGIFSIZE) ( HGLOBAL, DWORD, LPSIZE );
typedef DWORD (MLIBCONV *LPWJPGSIZE) ( HGLOBAL, DWORD, LPSIZE );
typedef BOOL  (MLIBCONV *LPWGIF2BMP) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
typedef BOOL  (MLIBCONV *LPWJPG2BMP) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

// COLOR_SPACE (short) values
#define	CS_UNKNOWN		0		// error/unspecified
#define	CS_GRAYSCALE	1		// monochrome (only 1 component)
#define	CS_RGB			2		// red/green/blue
#define	CS_YCbCr		3		// Y/Cb/Cr (also known as YUV)
#define	CS_YIQ			4		// Y/I/Q
#define	CS_CMYK			5		// C/M/Y/K

// Config data for decompression

typedef struct tagLIBCONFIG {
	WORD dcf_struct_size;	// Just SIZE of stucture ...
	// Decompress - 7 READ JPEG Configuration
	WORD dcf_out_color; // colorspace of output - Def=CS_RGB
	BOOL dcf_quantize_colors; // T if output is a colormapped format - Def=F
	BOOL dcf_two_pass_quantize;	// use two-pass color quantization? - Def=T
	BOOL dcf_use_dithering;		// want color dithering? - Def=TRUE
	int  dcf_desired_colors;	// max number of colors to use - Def=256
	BOOL dcf_do_block_smoothing; // T = apply cross-block smoothing - Def=F
	BOOL dcf_do_pixel_smoothing; // T = apply post-upsampling smoothing - Def=F
	// Compress - 9 WRITE JPEG Configuration
	WORD ccf_jpeg_color;	// Ouput color space. Def = CS_YCbCr
	WORD ccf_num_components;	// Number of color components 1 for mono Def=3
	WORD ccf_h_samp_factor;	// Def is  2x2 subsamples of chrominance
	WORD ccf_v_samp_factor;	// Both set to 1x1 for monochrome
	BOOL ccf_optimize_coding;	// Huffman optimisation Def=FALSE
	WORD ccf_restart_in_rows;	// Restart row count. Def=0
	WORD ccf_restart_intevals;	// Restart Huff MCU rows. Def=0
	WORD ccf_smoothing_factor;	// Def=0 Value 0 - 100 allowed
	WORD ccf_out_quality;	// Output quality. Range 1 - 100. Def=75
	// General Library Configuration
	BOOL cf_Safety_Check;	// Safety SIZE check at beginning of some services
	// Just a reserved area
	BYTE cf_reserved[32];	// Reserved block - all ZERO

} LIBCONFIG;

typedef LIBCONFIG MLPTR LPLIBCONFIG;

// NOTE: cf_Safety_Check default is TRUE. When TRUE the services WGIF2BMP,
//       WJPG2BMP and WGIFNBMP internally do WGIFSIZE, WJPGSIZE and
//       WGIFSIZX respectively to compare the size of the OUPTPUT Buffer
//       with the indicated SIZE required for the Bitmap to try to avoid
//       output Buffer overruns. But this represents a loss in speed of
//       conversion. When sure the User code correctly passes an output
//       Buffer of sufficient SIZE, then this can be SET Off through
//       WGETLCONFIG and WSETLCONFIG gaining a small increase in speed.
//
// ====================================================================

// DEFAULT Configuration Items for LIBRARY
// =======================================

#define	DefLibConfig	sizeof(LIBCONFIG),\
	CS_RGB, FALSE, TRUE, TRUE, 256, FALSE, FALSE,\
	CS_YCbCr, 3, 2, 2, FALSE, 0, 0, 0, 75,\
	TRUE,\
	0

// Internal above DEFAULT Values are as follows =============
// LIBCONFIG	LibConfig = {
//	sizeof(LIBCONFIG) = Manifest CONSTANT!
// Decompression Defaults.
// CS_RGB, FALSE, TRUE, TRUE, 256, FALSE, FALSE,
// Compression Defaults.
// CS_YCbCr, 3, 2, 2, FALSE, 0, 0, 75 };
// General Area
// TRUE
// Reserved area
// All 0's
// =========================================================

// Decompress - JPEG to BMP Config functions ... and
// Compress   - BMP to JPEG Config functions ...
// General    - Some general library items ...

EXPORT32
BOOL MLIBCALL WGETLCONFIG( LPLIBCONFIG );

EXPORT32
BOOL MLIBCALL WSETLCONFIG( LPLIBCONFIG );

typedef BOOL (MLIBCONV *LPWGETLCONFIG) ( LPLIBCONFIG );

typedef BOOL (MLIBCONV *LPWSETLCONFIG) ( LPLIBCONFIG );

#endif	// !RC_INVOKED

/* Multiple GIF Image Support */
/* ========================== */

// GIF Extensions
#define GIF_AppExt              0xFF
#define GIF_CommExt             0xFE
#define GIF_CtrlExt             0xF9
#define GIF_PTxtExt             0x01

#define GIF_Trailer             0x3B
#define GIF_ImgDesc             0x2C

// NOTE: ghMaxCount member can not be larger than 0x7fff!
// That's 32,767 images - A reasonable MAXIMUM!!!
// ======================================================
typedef struct tagGIFIMG {
	WORD	giWidth;
	WORD	giHeight;
	WORD	giCMSize;
	DWORD	giBMPSize;
}GIFIMG;
typedef GIFIMG MLPTR LPGIFIMG;

typedef struct tagGIFHDR {
	WORD	ghMaxCount;
	WORD	ghImgCount;
	WORD	ghWidth;
	WORD	ghHeight;
	WORD	ghCMSize;
	DWORD	ghBMPSize;
	GIFIMG ghGifImg[1];
}GIFHDR;
typedef GIFHDR MLPTR LPGIFHDR;

// Multiple and Transparent GIF Image Support
// ==========================================
#define		gie_Flag		0x8000	// This is in the ghMaxCount
// if the application expect an EXTENDED description!

typedef	struct	tagGIFIMGEXT {
	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
// Image Descriptor - Wdith and Height in above
	WORD	giLeft;		// Left (logical) column of image
	WORD	giTop;		// Top (logical) row
	BYTE	giBits;		// See below (packed field)
// Graphic Control Extension
	BYTE	gceExt;		// Extension into - Value = 0x21
	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
	DWORD	gceSize;	// Block Size (0x04 for GCE, Big for TEXT)
	BYTE	gceBits;	// See below (packed field)
	WORD	gceDelay;	// 1/100 secs to wait
	BYTE	gceIndex;	// Transparency Index (if Bit set)
	DWORD	gceColr;	// Only valid IF bit is SET
	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
	RGBQUAD	gceBkGrnd;	// Background Colour
	// NOTE: These 6 are NOT used or altered by the Library
	// --------------------------------------------------
	HANDLE	hDIB;		// Handle to the DIB
	HPALETTE hPal;		// Handle to the bitmap's palette.
	HBITMAP	hBitmap;	// Handle to the DDB.
	HFONT	hFont;		// Handle to TEXT Font (if any)
	LPVOID	lpNext;		// Pointer to Service
	DWORD	dwgiFlag;	// Various APPLICATION defined Flags
	// --------------------------------------------------
	DWORD	gceRes1;	// Reserved
	DWORD	gceRes2;	// ditto
}GIFIMGEXT;
typedef GIFIMGEXT MLPTR LPGIFIMGEXT;

typedef struct tagGIFHDREXT {
	WORD	gheMaxCount;	// gie_Flag + MAX. Count
	WORD	gheImgCount;	// Images in GIF
	WORD	gheWidth;		// Logical Screen Width
	WORD	gheHeight;		// Logical Screen Height
	WORD	gheCMSize;		// BMP Colour map size (byte count)
	DWORD	gheBMPSize;		// Estimated final BMP size
	BYTE	gheBits;		// See below (packed field)
	BYTE	gheIndex;		// Background Colour Index
	BYTE	ghePAR;			// Pixel Aspect Ration
	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
	RGBQUAD	gheBkGrnd;		// Background Colour
	// NOTE: These 5 are NOT used or altered by the Library
	// --------------------------------------------------
	HANDLE	hDIB;			// Handle to the DIB
	HPALETTE hPal;			// Handle to the bitmap's palette.
	HBITMAP	hBitmap;		// Handle to the DDB.
	HFONT	hFont;			// Handle to TEXT Font (if any)
	DWORD	dwghFlag;		// Application FLAG
	// --------------------------------------------------
	DWORD	gheRes1;		// Reserved
	DWORD	gheRes2;		// ditto
	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
}GIFHDREXT;
typedef GIFHDREXT MLPTR LPGIFHDREXT;

// GIFHDREXT.gheBits = GIF Logical Screen Descriptor Bits
// =================
#define		ghe_ColrMap		0x80	// A Global Color Map
#define		ghe_ColrRes		0x70	// Colour Resolution
#define		ghe_Sort		0x08	// Sorted Colour Map
#define		ghe_ColrSize	0x07	// Size of Colour Table ((n+1)^2)

// GIFIMGEXT gceBits = GIF Graphic Control Extension Bits
// =================
#define		gce_Reserved	0xe0	// Reserved 3 Bits
#define		gce_Disposal	0x1c	// Disposal Method 3 Bits
#define		gce_UserInput	0x02	// User Input Flag 1 Bit
#define		gce_TransColr	0x01	// Transparent Color Flag 1 Bit

// GIFIMGEXT.giBits = GIF Graphic Image Bits
// ================
#define		gie_ColrLoc		0x80	// Local Colour Table
#define		gie_Interlace	0x40	// Interlaced Scan lines
#define		gie_SortFlag	0x20	// Sorted Color Table3
#define		gie_Reserved	0x18	// 2 reserved bits
#define		gie_ColrSize	0x07	// Colr Table Size ((n+1)^2)

// GIFHDREXT Flag
// An "ANIMATED" GIF
#define		ghf_Netscape	0x00000001	// Contains "Netscape" Extension
#define		ghf_AppExt		0x00000002	// Had App Extension
#define		ghf_CommExt		0x00000004	// Had Comment Extension
#define		ghf_CtrlExt		0x00000008	// Had Graphic Control Extension
#define		ghf_PTxtExt		0x00000010	// Had plain text extension

#define		ghf_UnknExt		0x40000000	// Had an UNKNONW extension
#define		ghf_Incomplete	0x80000000	// Incomplete GIF data

// GIFIMGEXT Flag
#define		gie_Netscape	0x00000001	// Found NETSCAPE extension
#define		gie_GCE			0x00000002	// Graphic Control Extension
#define		gie_PTE			0x00000004	// Plain Text Extentsion
#define		gie_GID			0x00000008	// Image Descriptor
#define		gie_APP			0x00000010	// Application Extension
#define		gie_COM			0x00000020	// Comment Extension
#define		gie_UNK			0x80000000	// Undefined Extension

// For Library Service WGIFNBMPX - Get GIF *AND* Plain Text
// This header comes before the Plain Text Extension header,
// and the actual "plain text" data.
typedef	struct tagPTEHDR {	/* pt */
	DWORD	pt_Total;	// Total length of Plain Text Buffer
	DWORD	pt_Missed;	// Any TEXT missed (due to buffer size)
}PTEHDR;
typedef PTEHDR MLPTR LPPTEHDR;

EXPORT32
WORD MLIBCALL WGIFSIZX( HGLOBAL, DWORD, HGLOBAL );

typedef WORD (MLIBCONV *LPWGIFSIZX) ( HGLOBAL, DWORD, HGLOBAL );

/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hOutInfo;	// Handle to structure GIFHDR, with ghMaxCount
 *                      // member set to count of GIFIMG structures
 * ***** OR *****
 * HGLOBAL hOutInfo;	// Handle to structure GIFHDREXT, with gheMaxCount
 *                      // member set to count of GIFIMGEXT structures
 *						// plus the gie_Flag.
 * If SUCCESS then a ZERO return, otherwise and ERROR VALUE per the above.
 * NOTE 1: If successful, ghImgCount (or gheImgCount if gie_Flag set)
 * will contain the total number of IMAGES found. This COUNT will NOT
 * include Plain Text Extensions, although some information on them
 * will be included. For a FULL COUNT of BOTH IMAGES and Plain
 * Text Extensions the WGIFSIZXT( ) MUST be used.
 * NOTE 2: It is possible to call WGIFSIZX with ghMaxCount as ZERO,
 * with or without the gie_Flag depending on which structure used,
 * to return just the TOTAL IMAGE COUNT in the file.
 * ----------------------------------------------------------------- */

EXPORT32
BOOL MLIBCALL WGIFNBMP( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );

typedef BOOL  (MLIBCONV *LPWGIFNBMP) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );

/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hInfo;			// Handle to Information buffer (1K) May be NULL
 * HGLOBAL hOutputData;	// Handle to OUTPUT DATA Buffer
 * WORD    wImageNum;	// Image NUMBER to convert ...
 * If SUCCESS function will return ZERO. Otherwise an ERROR VALUE per
 * the above list will be returned.
 * Naturally OUTPUT Data is only valid if ZERO is returned.
 * NOTE 1: This function only returns a BITMAP exactly equivalent to
 * the specific IMAGE number requested. In other words it will be
 * at the size indicated in the Image Descriptor, and NOT that of
 * the Logical Screen. The information returned in the WGIFSIZX, with
 * the gie_Flag set would need to be used to place the image correctly
 * on the Logical Screen if the Image was less than the size of the
 * Logical Screen.
 * NOTE 2: This function will ONLY return converted IMAGES, and NOT
 * Plain Text Extension. The function WGIFNBMPX would need to be
 * used to return BOTH Images and Plain Text Extensions.
 * ------------------------------------------------------------------ */

EXPORT32
WORD MLIBCALL WGIFSIZXT( HGLOBAL, DWORD, HGLOBAL );

typedef WORD (MLIBCONV *LPWGIFSIZXT) ( HGLOBAL, DWORD, HGLOBAL );

/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hOutInfo;	// Handle to structure GIFHDREXT, with gheMaxCount
 *                      // member set to count of GIFIMGEXT structures
 *						// plus the gie_Flag.
 * If SUCCESS then a ZERO return, otherwise and ERROR VALUE per the above.
 * NOTE: If successful, the gheImgCount member of the structure
 * will contain the total number of IMAGES and Plain Text Extensions
 * found.
 * ----------------------------------------------------------------- */

EXPORT32
BOOL MLIBCALL WGIFNBMPX( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );

typedef BOOL  (MLIBCONV *LPWGIFNBMPX) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );

/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hInfo;			// Handle to Information buffer (1K) May be NULL
 * HGLOBAL hOutputData;	// Handle to OUTPUT DATA Buffer
 * WORD    wImageNum;	// Image NUMBER to convert ...
 * If SUCCESS function will return ZERO. Otherwise an ERROR VALUE per
 * the above list will be returned.
 * Naturally OUTPUT Data is only valid if ZERO is returned.
 * NOTE 1: When conversion of Image 1 is requested, this function
 * returns a BITMAP exactly equivalent to the Logical Screen size,
 * with the first Image, (or Plain Text Extension) correctly placed
 * into the BITMAP.
 * NOTE 2: However, each successive call will only return a BITMAP
 * equivalent to the next Image, or Plain Text Extension, thus the
 * application must correctly place this image onto the Logical
 * Screen.
 * ------------------------------------------------------------------ */

// ==========================


// ERROR NUMBER SECTION
// ====================
#define	MXERRBUF		260		// Minimum SIZE of ERROR Mesage BUFFER (if given)

// int Error returns
#define	BAD_FACTOR		5002
#define	BAD_SYNT			5003
#define	BAD_FACTOR2		5004
#define	BAD_FILENAME	5005
#define	BAD_EMPTY		5006
#define	BAD_FORMAT		5007
#define	BAD_UNGET		5008
#define	BAD_FOPEN		5009
#define	BAD_OOPEN		5010
#define	BAD_MEMORY		5011	/* "Insufficient memory!" */
#define	BAD_MEMORY2		5012	/* "Bogus free_small request!" */
#define	BAD_MEMORY3		5013	/* "Bogus free_medium request!" */
#define	BAD_MEMORY4		5014	/* "Image too wide for this memory impl...*/
#define	BAD_MEMORY5		5015	/* "Bogus free_small_sarray request!" */
#define	BAD_MEMORY6		5016	/* "Bogus free_small_barray request" */
#define	BAD_MEMORY7		5017	/* "Bogus access_big_sarray request" */
#define	BAD_MEMORY8		5018	/* "Virtual array controller messed up!" */
#define	BAD_MEMORY9		5019	/* "Bogus access_big_barray request" */
#define	BAD_MEMORY10	5020	/* "Bogus free_big_sarray request" */
#define	BAD_MEMORY11	5021	/* "Bogus free_big_barray request" */
#define	BAD_GIFOUT		5022	/* "GIF output got confused!" */
#define	BAD_BMPEOF		5023	/* "Premature EOF in BMP file" */
#define	BAD_BMPSIZ		5024	/* "Error: File too small. Not a BMP file!" */
#define	BAD_BMPREAD		5025	/* "Error: Bad read of input file!" */
#define	BAD_BMPSIZ2		5026	/* "Error: Conflict in SIZE. Not a BMP file!" */
#define	BAD_BMPRES1		5027	/* "Error: Reserved items NOT zero. Not a BMP file!" */
#define	BAD_BMPBM		5028	/* "ERROR: No BM signature! Not a BMP file!" */
#define	BAD_BMPBC		5029	/* "Error: Unsupported bit count! Not 1,4,8,24!" */
#define	BAD_BMPCM		5030	/* "Error: Unsupported Color map! Only RGBQUAD!" */
#define	BAD_BMPCOLR		5031	/* "Error: Color table mistake. Not a BMP file ..." */
#define	BAD_BMPWHP		5032	/* "ERROR: Width, Height or Planes! Not a BMP file!" */
#define	BAD_BMPNS		5033	/* "Compressed BMP file not presently handled ..." */
#define	BAD_WWRITE		5034	/* "Output File write error!" */
#define	BAD_PPMOUT		5035	/* "PPM output must be grayscale or RGB" */
#define	BAD_BMPOUT		5036	/* "BMP output is confused! Can not handle channels!" */
#define	BAD_BACKING		5037	/* "Backing store not supported" */
#define	BAD_GIFEOF		5038	/* "Premature EOF in GIF file" */
#define	BAD_GIFNOT		5039	/* "Not a GIF file" */
#define	BAD_GIFCNT		5040	/* "Too few images in GIF file" */
#define	BAD_GIFCS		5041	/* "Bogus codesize (input_code_size)" */
#define	BAD_PPMEOF		5042	/* "Premature EOF in PPM file" */
#define	BAD_PPMBD		5043	/* "Bogus data in PPM file" */
#define	BAD_PPMNOT		5044	/* "Not a PPM file" */
#define	BAD_HUFFTAB		5045	/* "Huffman table not defined!" */
#define	BAD_JFIFMAX		5046	/* "Maximum image dimension for JFIF is 65535 pixels" */
#define	BAD_WRITOUT		5047	/* "Output file write error --- out of disk space?" */
#define	BAD_TAREOF		5048	/* "Unexpected end of Targa file!" */
#define	BAD_TARGA		5049	/* "Invalid or unsupported Targa file" */
#define	BAD_TARSIZ		5050	/* "Targa Colormap too large" */
#define	BAD_HUFFDHT		5051	/* "Bogus DHT counts" */
#define	BAD_HUFFDAC		5052	/* "Bogus DAC index!" */
#define	BAD_HUFFTN		5053	/* "Bogus table number!" */
#define	BAD_HUFFDRI		5054	/* "Bogus length in DRI" */
#define	BAD_JFIFBN		5055	/* "Unsupported JFIF revision number!" */
#define	BAD_JFIFDNL		5056	/* "Empty JPEG image (DNL not supported)" */
#define	BAD_JFIFDP		5057	/* "Unsupported JPEG data precision" */
#define	BAD_JFIFSOF		5058	/* "Bogus SOF length" */
#define	BAD_JFIFSOS		5059	/* "Invalid component number in SOS" */
#define	BAD_JFIFNOT		5060	/* "Not a JPEG file" */
#define	BAD_JFIFSMT		5061	/* "Unsupported SOF marker type!" */
#define	BAD_JFIFUM		5062	/* "Unexpected marker!" */
#define	BAD_TARHDR		5063	/* "Could not write Targa header" */
#define	BAD_TARRGB		5064	/* "Targa output must be grayscale or RGB" */
#define	BAD_COMMAND		5065	/* "ERROR: Bad command line arguments given!" */
#define	BAD_JPGEOF		5066	/* "Premature EOF in JPG file!" */
#define	BAD_JPGNUL		5067	/* "Empty JPEG file" */
#define	BAD_JPGDATA		5068	/* "Corrupt JPEG data: premature end of data segment" */
#define	BAD_TARCF		5069	/* "Unsupported Targa colormap format" */
#define	BAD_JPGSAMP		5070	/* "Bogus sampling factors" */
#define	BAD_JPGTMC		5071	/* "Too many components for interleaved scan" */
#define	BAD_JPGICA		5072	/* "I'm confused about the image width" */
#define	BAD_JPGSFT		5073	/* "Sampling factors too large for interleaved scan" */
#define	BAD_NOTIMP		5074	/* "Not implemented yet" */
#define	BAD_JPGMSS		5075	/* "Multiple-scan support was not compiled" */
#define	BAD_JPGBIC		5076	/* "Bogus input colorspace" */
#define	BAD_JPGBJC		5077	/* "Bogus JPEG colorspace" */
#define	BAD_JPGUCC		5078	/* "Unsupported color conversion request" */
#define	BAD_HUFFTE		5079	/* "Missing Huffman code table entry" */
#define	BAD_HUFFUHT		5080	/* "Use of undefined Huffman table" */
#define	BAD_HUFFCST		5081	/* "Huffman code size table overflow" */
#define	BAD_CCIR		5082	/* "CCIR601 downsampling not implemented yet" */
#define	BAD_FRACTD		5083	/* "Fractional downsampling not implemented yet" */
#define	BAD_NOOUT		5084	/* "Unsupported output file format" */
#define	BAD_BMPCE		5085	/* "Cannot handle colormap entries for BMP!" */
#define	BAD_BMPNRGB		5086	/* "BMP output must be grayscale or RGB" */
#define	BAD_GIFNRGB		5087	/* "GIF output must be grayscale or RGB" */
#define	BAD_GIF256		5088	/* "GIF can only handle 256 colors" */
#define	BAD_TARCOLS		5089	/* "Too many colors for Targa output" */
#define	BAD_ARITH		5090	/* "Arithmetic coding not supported" */
#define	BAD_JPGQFC		5091	/* "Cannot quantize fewer than 2 colors" */
#define	BAD_JPGCQC		5092	/* "Cannot quantize more than %d color components" */
#define	BAD_JPGCRM		5093	/* "Cannot request more quantized colors" */
#define	BAD_NOIMP2		5094	/* "Should not get here!" */
#define	BAD_JPGCRL		5095	/* "Cannot request less than 8 quantized colors" */
#define	BAD_JPG2PQ		5096	/* "2-pass quantization only handles YCbCr input" */
#define	BAD_JPGHUFF		5097	/* "W: Corrupt JPEG data: bad Huffman code" */
#define	BAD_JPGDAT2		5098	/* "WARNING: Corrupt JPEG data: Extraneous bytes before marker" */
#define	BAD_MEMCNTS		5099	/* "ERROR: Increase small memory counter! " */
#define	BAD_MEMCNTF		5100	/* "ERROR: Increase large memory counter! " */
#define	BAD_MEMHEAP1	5101	/* "ERROR: Increase HEAP size for near memory! (1)" */
#define	BAD_JPGEOS		5102	/* "ERROR: Sorry, entropy optimization was not compiled! " */
#define	BAD_TARSNC		5103	/* "ERROR: Targa support was not compiled! " */
#define	BAD_QTABOPN		5104	/* "ERROR: Unable to open quantizing table file! " */
#define	BAD_QTABSIZ		5105	/* "ERROR: Too many tables in qtable file! " */
#define	BAD_QTABINC		5106	/* "ERROR: Incomplete table in qtable file! " */
#define	BAD_GIFPTR		5107	/* "ERROR: NULL Data pointer passed! " */
#define	BAD_G2B_ENTRY	5108	/* "ERROR: Input parameter to LIBRARY errant! " */
#define	BAD_MEMORY12	5109	/* "ERROR: Unable to ACCESS Global Memory! " */
#define	BAD_QFILE		5110	/* "ERROR: Bogus data in quantization file! "} */
#define	BAD_PTRNULL		5111	/* "ERROR: Pointer NOT intialized! " } */
#define	BAD_RETIRED		5112	//	"ERROR: Attempt to use a RETIRED interface! "
#define	BAD_NOGCOLR		5113	//	"ERROR: Bad GIF Image. No Global Color Table! "
#define	BAD_MEMHEAP2	5114	/* "ERROR: Increase HEAP size for near memory! (2)" */
#define	BAD_ZEROMEMH	5115	// "ERROR: No more memory handles! 

/* If SETLCONFIG Fails, one of the following errors maybe returned */
#define	CERR_UNDETERM		1
#define	CERR_NULLPTR		2
#define	CERR_INCOMPAT		3
#define	CERR_BADOCOLOR		4
#define	CERR_BADCCOLOR		5
#define	CERR_OUTQUAL		6

/* Care: RESOURCE.H commences ERR_?????? at 5500 ... */
/* however only presently listed in DibView - See Errors.h and .c */
/* ============================================================== */

// Close the CPP Block
#ifdef	__cplusplus
}
#endif	// __cplusplus

#endif	// _wjpeglib_h
// eof - WjpgeLib.h
