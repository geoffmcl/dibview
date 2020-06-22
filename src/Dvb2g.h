
// DvB2G.h
#ifndef	_DvB2G_h
#define	_DvB2G_h

// #include	"WJPEGLIB.h"

#define JPTR	MLPTR
#define METHODDEF static	/* a function called through method pointers */
#define LOCAL	  static	/* a function used only in its module */
#define GLOBAL			/* a function referenced thru EXTERNs */
#define SIZEOF(obj)  ((size_t) sizeof(obj)) 
#define ERRFLAG( a )
#define METHOD( type, methodname, arglist)  type (*methodname) arglist

// these are defined in Wconfig.h also ...
typedef	short	INT16;
#ifndef _BASETSD_H_		/* <basetsd.h> defines as int which is ok */
typedef long INT32;
#endif	/* _BASETSD_H_ */

typedef	BYTE	UINT8;
#ifndef	WIN32
typedef	int		boolean;
#endif	// !WIN32
typedef	WORD	UINT16;

typedef BYTE  JSAMPLE;	/* One BYTE (unsigned) */
typedef JSAMPLE MLPTR JSAMPROW;	/* ptr to one image row of pixel samples. */
typedef JSAMPROW MLPTR JSAMPARRAY;	/* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY MLPTR JSAMPIMAGE;	/* a 3-D sample array: top index is color */

#define GETJSAMPLE(value)  ((value) & 0xFF) 

struct External_methods_struct1 {
	int trace_level;	/* level of detail of tracing messages */
	long max_memory_to_use;	/* maximum amount of memory to use */
//	METHOD(void MLPTR, alloc_medium, (size_t));
};
typedef struct External_methods_struct1 JPTR external_methods_ptr;

/* Methods used during JPEG decompression. */
typedef struct Decompress_methods_struct1 JPTR decompress_methods_ptr;

typedef struct tagDecompress_info_struct1 {
	WORD	di_Len;
	HFILE	output_file;
	int data_precision;
	long image_width;	/* input image width */
	long image_height;	/* input image height */
	short out_color_space; /* colorspace of output */
	short final_out_comps;	/* # of color components sent to put_pixel_rows */
	boolean quantize_colors; /* T if output is a colormapped format */
	int desired_number_of_colors;	/* max number of colors to use */
	decompress_methods_ptr methods; /* Points to list of methods to use */
	external_methods_ptr emethods; /* Points to list of methods to use */
	HGLOBAL hgDib;
	LPSTR   lpDib;
	DWORD   ddDSz;
	HGLOBAL hgOut;
	LPSTR   lpOut;
	DWORD	  ddOSz;
	DWORD   ddOut;
	LPSTR	  lpFil;
}Decompress_info_struct1;

typedef Decompress_info_struct1 JPTR decompress_info_ptr;

struct Decompress_methods_struct1 {
	int	dms_len;
	METHOD(boolean, output_init, (decompress_info_ptr));
	METHOD(void, put_color_map, (decompress_info_ptr, int, JSAMPARRAY));
	METHOD(void, put_pixel_rows, (decompress_info_ptr, int, JSAMPIMAGE));
	METHOD(void, output_term, (decompress_info_ptr));
};

#endif	// _DvB2G_h
// eof - DV32 File - BMP2GIF conversion
// ====================================

