

// DvDebug.c

#include	"Dv.h"		// All incusive include

#ifdef	_DEBUG
#ifndef	NDEBUG

extern	BOOL	FindNewName( LPSTR );
extern	DWORD	DVWrite( HANDLE, VOID MLPTR, DWORD );

//char	szDbgBmp[MAX_PATH+16] = { "TEMPD001.BMP" };

#endif	// !NDEBUG
#endif	// _DEBUG

//BITMAPINFOHEADER
//The BITMAPINFOHEADER structure contains information about the
//dimensions and color format of a device-independent bitmap
//(DIB). 
//typedef struct tagBITMAPINFOHEADER{ // bmih 
//   DWORD  biSize; 
//   LONG   biWidth; 
//   LONG   biHeight; 
//   WORD   biPlanes; 
//   WORD   biBitCount 
//   DWORD  biCompression; 
//   DWORD  biSizeImage; 
//   LONG   biXPelsPerMeter; 
//   LONG   biYPelsPerMeter; 
//   DWORD  biClrUsed; 
//   DWORD  biClrImportant; 
//} BITMAPINFOHEADER; 
//Members
//biSize 
//Specifies the number of bytes required by the structure. 
//biWidth 
//Specifies the width of the bitmap, in pixels. 
//biHeight 
//Specifies the height of the bitmap, in pixels. If biHeight is
//positive, the bitmap is a bottom-up DIB and its origin is the
//lower left corner. If biHeight is negative, the bitmap is a
//top-down DIB and its origin is the upper left corner. 
//biPlanes 
//Specifies the number of planes for the target device. This value
//must be set to 1. 
//biBitCount 
//Specifies the number of bits per pixel. This value must be 1, 4,
//8, 16, 24, or 32. 
//biCompression 
//Specifies the type of compression for a compressed bottom-up
//bitmap (top-down DIBs cannot be compressed). It can be one of
//the following values: 
//ValueDescriptionBI_RGBAn uncompressed format.BI_RLE8A run-length
//encoded (RLE) format for bitmaps with 8 bits per pixel. The
//compression format is a two-byte format consisting of a count
//byte followed by a byte containing a color index. For more
//information, see the following Remarks section.BI_RLE4An RLE
//format for bitmaps with 4 bits per pixel. The compression format
//is a two-byte format consisting of a count byte followed by two
//word-length color indices. For more information, see the
//following Remarks section.BI_BITFIELDSSpecifies that the bitmap
//is not compressed and that the color table consists of three
//doubleword color masks that specify the red, green, and blue
//components, respectively, of each pixel. This is valid when used
//with 16- and 32-bits-per-pixel bitmaps.
//biSizeImage 
//Specifies the size, in bytes, of the image. This may be set to 0
//for BI_RGB bitmaps. 
//biXPelsPerMeter 
//Specifies the horizontal resolution, in pixels per meter, of the
//target device for the bitmap. An application can use this value
//to select a bitmap from a resource group that best matches the
//characteristics of the current device. 
//biYPelsPerMeter 
//Specifies the vertical resolution, in pixels per meter, of the
//target device for the bitmap. 
//biClrUsed 
//Specifies the number of color indices in the color table that
//are actually used by the bitmap. If this value is zero, the
//bitmap uses the maximum number of colors corresponding to the
//value of the biBitCount member for the compression mode
//specified by biCompression. 
//If biClrUsed is nonzero and the biBitCount member is less than
//16, the biClrUsed member specifies the actual number of colors
//the graphics engine or device driver accesses. If biBitCount is
//16 or greater, then biClrUsed member specifies the size of the
//color table used to optimize performance of Windows color
//palettes. If biBitCount equals 16 or 32, the optimal color
//palette starts immediately following the three doubleword masks.
//If the bitmap is a packed bitmap (a bitmap in which the bitmap
//array immediately follows the BITMAPINFO header and which is
//referenced by a single pointer), the biClrUsed member must be
//either 0 or the actual size of the color table. 
//biClrImportant 
//Specifies the number of color indices that are considered
//important for displaying the bitmap. If this value is zero, all
//colors are important. 
//Remarks
//The BITMAPINFO structure combines the BITMAPINFOHEADER structure
//and a color table to provide a complete definition of the
//dimensions and colors of a DIB. For more information about DIBs,
//see the description of the BITMAPINFO data structure. 
//An application should use the information stored in the biSize
//member to locate the color table in a BITMAPINFO structure, as
//follows: 
//pColor = ((LPSTR)pBitmapInfo + 
//    (WORD)(pBitmapInfo->bmiHeader.biSize)); 
//Windows supports formats for compressing bitmaps that define
//their colors with eight or four bits per pixel. Compression
//reduces the disk and memory storage required for the bitmap. The
//following paragraphs describe these formats. 
//When the biCompression member is BI_RLE8, the bitmap is
//compressed by using a run-length encoding (RLE) format for an
//8-bit bitmap. This format can be compressed in encoded or
//absolute modes. Both modes can occur anywhere in the same
//bitmap. 
//�Encoded mode consists of two bytes: the first byte specifies
//the number of consecutive pixels to be drawn using the color
//index contained in the second byte. In addition, the first byte
//of the pair can be set to zero to indicate an escape that
//denotes an end of line, end of bitmap, or delta. The
//interpretation of the escape depends on the value of the second
//byte of the pair, which can be one of the following: 
//ValueMeaning0End of line.1End of bitmap.2Delta. The two bytes
//following the escape contain unsigned values indicating the
//horizontal and vertical offsets of the next pixel from the
//current position.
//�In absolute mode, the first byte is zero and the second byte is
//a value in the range 03H through FFH. The second byte represents
//the number of bytes that follow, each of which contains the
//color index of a single pixel. When the second byte is 2 or
//less, the escape has the same meaning as in encoded mode. In
//absolute mode, each run must be aligned on a word boundary. 
//The following example shows the hexadecimal values of an 8-bit
//compressed bitmap. 
//03 04 05 06 00 03 45 56 67 00 02 78 00 02 05 01 
//02 78 00 00 09 1E 00 01 
//This bitmap would expand as follows (two-digit values represent
//a color index for a single pixel): 
//04 04 04 
//06 06 06 06 06 
//45 56 67 
//78 78 
//move current position 5 right and 1 down 
//78 78 
//end of line 
//1E 1E 1E 1E 1E 1E 1E 1E 1E 
//end of RLE bitmap 
//When the biCompression member is BI_RLE4, the bitmap is
//compressed by using a run-length encoding format for a 4-bit
//bitmap, which also uses encoded and absolute modes: 
//�In encoded mode, the first byte of the pair contains the number
//of pixels to be drawn using the color indices in the second
//byte. The second byte contains two color indices, one in its
//high-order four bits and one in its low-order four bits. The
//first of the pixels is drawn using the color specified by the
//high-order four bits, the second is drawn using the color in the
//low-order four bits, the third is drawn using the color in the
//high-order four bits, and so on, until all the pixels specified
//by the first byte have been drawn. 
//�In absolute mode, the first byte is zero, the second byte
//contains the number of color indices that follow, and subsequent
//bytes contain color indices in their high- and low-order four
//bits, one color index for each pixel. In absolute mode, each run
//must be aligned on a word boundary. The end-of-line,
//end-of-bitmap, and delta escapes described for BI_RLE8 also
//apply to BI_RLE4 compression. 
//The following example shows the hexadecimal values of a 4-bit
//compressed bitmap. 
//03 04 05 06 00 06 45 56 67 00 04 78 00 02 05 01 
//04 78 00 00 09 1E 00 01 
//This bitmap would expand as follows (single-digit values
//represent a color index for a single pixel): 
//0 4 0 
//0 6 0 6 0 
//4 5 5 6 6 7 
//7 8 7 8 
//move current position 5 right and 1 down 
//7 8 7 8 
//end of line 
//1 E 1 E 1 E 1 E 1 
//end of RLE bitmap 
//If biHeight is negative, indicating a top-down DIB,
//biCompression must be either BI_RGB or BI_BITFIELDS. Top-down
//DIBs cannot be compressed.
//QuickInfo
//  Windows NT: Use version 3.1 and later. 
//  Windows: Use Windows 95 and later. 
//  Header: Declared in wingdi.h.
//See Also
////Bitmaps Overview, Bitmap Structures, BITMAPINFO 

void	DumpDIB( HANDLE hDIB )
{
#ifdef	_DEBUG

#ifndef	NDEBUG

	DWORD				dwDIBSz, dwPALSz, dwOffBits;
	LPSTR				lpf;
	LPBITMAPINFOHEADER  lpbi;	// BITMAPINFOHEADER
	HANDLE				fh;
	OFSTRUCT			of;
	BITMAPFILEHEADER	hdr;
	int					iBPP, iClrUsed;
	DWORD				dw_srccols, dwsz;

	lpf = &gszDbgBmp[0];
	// Lock down the DIB
	if( ( !FindNewName( lpf ) ) ||
		( hDIB == NULL ) ||
		( ( dwDIBSz = GlobalSize( hDIB ) ) == 0 ) ||
		( ( lpbi = (LPBITMAPINFOHEADER)DVGlobalLock( hDIB ) ) == 0 ) ) // LOCK DIB HANDLE 
	{
		// no way
		return;
	}
	dwPALSz = PaletteSize((LPSTR)lpbi);
	dwOffBits = ( lpbi->biSize + dwPALSz );
	iBPP = lpbi->biBitCount;
	// Fix Bits per Pixel
	if( iBPP <= 1 )
		iBPP = 1;
	else if( iBPP <= 4 )
		iBPP = 4;
	else if( iBPP <= 8 )
		iBPP = 8;
	else
		iBPP = 24;
	switch( iBPP )
	{
	case 1:		// Monochrome
		iClrUsed = 2;
		dw_srccols = lpbi->biWidth / 8;
		if( lpbi->biWidth % 8 )
			dw_srccols++;
		break;
	case 4:
		iClrUsed = 16;
		dw_srccols = lpbi->biWidth / 2;
		if( lpbi->biWidth % 2 )
			dw_srccols++;
		break;
	case 8:
		iClrUsed = 256;
		dw_srccols = lpbi->biWidth;
		break;
	default:	// Default to 24-bit DIB
		iClrUsed = 0;		// Each 3 Bytes is a COLOR
		dw_srccols = lpbi->biWidth * 3;
		break;
	}
	while( (dw_srccols & 3) != 0 )
		dw_srccols++;

	dwsz  = lpbi->biSize;
	dwsz += ( iClrUsed * sizeof(RGBQUAD) );
	dwsz += ( dw_srccols * lpbi->biHeight );
	if( dwsz != dwDIBSz )
		iClrUsed = (int)-1;

	// CREATE the OUTPUT FILE
	fh = DVOpenFile( lpf, &of, OF_CREATE|OF_READWRITE );
	// If we GOT a file,
	if( ( fh == 0           ) ||
		 ( fh == INVALID_HANDLE_VALUE ) ) // HFILE_ERROR
	{
		// no way too
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for error exit of function
		DVOpenFile( lpf, &of, OF_DELETE );
		return;
	}

	hdr.bfType		= DIB_HEADER_MARKER;
	hdr.bfSize		= dwDIBSz + sizeof (BITMAPFILEHEADER);
	hdr.bfReserved1	= 0;
    hdr.bfReserved2	= 0;
	hdr.bfOffBits	= (DWORD) ( sizeof(BITMAPFILEHEADER) +
		dwOffBits );

	if( DVWrite( fh, (LPSTR)&hdr, sizeof(BITMAPFILEHEADER) ) !=
		sizeof( BITMAPFILEHEADER ) )
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for another error exit
		DVlclose( fh );
		DVOpenFile( lpf, &of, OF_DELETE );
		return;
	}

	if( DVWrite( fh, (VOID MLPTR)lpbi, dwDIBSz ) !=
		dwDIBSz )
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - yet another error
		DVlclose( fh );
		DVOpenFile( lpf, &of, OF_DELETE );
		return;
	}

	DVlclose( fh );
	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

#endif	// !NDEBUG

#endif	// _DEBUG

}

// eof - DvDebug.c
