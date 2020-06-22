
// DvTif.c
#include "Dv.h"
//#include "DvTif.h"

#ifdef   ADDTIFF6
#ifndef _TIFF_
#include "../tif6/tiff.h"
#endif

#ifdef   huge
#undef   huge
#endif
#define  huge
#define  _fmemcpy    memcpy
#define  _fmemcmp    memcmp
#define  _fmemset    memset

// forward references
LPTSTR   GetCompStg( DWORD val );
LPTSTR   GetPhotoStg( DWORD val );
LPTSTR   GetOrientStg( DWORD val );
HANDLE   ReadTIF6( FILE * fp );
int   ReadTIF6Tiled( LPTSTR pbuf, DWORD bufsize, FILE * pmf, BOOL IsMotor,  // was fp
                LPTSTR line, DWORD width );

TCHAR    gszTifInfo[1024]; // for the dialog information

BOOL     IsMotor = FALSE;
#define  IsMoto   IsMotor
#define  mftell      ftell
#define  mfseekc     fseek
#define  mfseek      fseek
#define  mGetMIlng   GetMIlng
#define  mgetc       getc

HANDLE LoadTIFImage( PRDIB prd )
{
   HANDLE   hDIB = NULL;   // return HANDLE (to global memory) of a DIB
   LPTSTR   lpf = prd->rd_pPath;
   LPTSTR   lpi = gszTifInfo;
   FILE * fp = fopen(lpf, "rb");
   if(fp)
   {
      hDIB = ReadTIF6( fp );
      fclose(fp);
   }
   return hDIB;
}

#ifndef  ucp
//typedef ucp unsigned char *
#define  cp    char *
#define  uc    unsigned char
#define  ucp   uc *
#endif   // ucp

#define NO_OPEN   -1
#define MEM_ERR   -2
#define BAD_READ  -3
#define BAD_COPY  -4
#define NO_BHND   -5
#define NO_GPTR   -6
#define BAD_CODE  -7
#define BAD_FIRST -8
#define BAD_BIT   -9
#define BAD_HEAD  -10
#define NO_BMP    -11
#define NO_PAL    -12
#define OS_RES    -13   // not supported by OS
#define NO_SUP    -14   // file format NOT supported

//GVFILE_API ReadTIF(LPSTR fname, HDC pichdc, HWND pichwnd, WORD dither,
//						   WORD print, WORD x, WORD y, WORD scale) {
unsigned long TileWidth, TileLength, TileOffsets, TileByteCounts;
unsigned char    buf[264];
// active RGB x 256 palette
unsigned char    palette[768];
BITMAPINFOHEADER bih;   //sBmpInfHdr;   // static BITMAP information header
GLOBALHANDLE     ghnd;

unsigned int GetMIw(FILE *fp, int tf)
{
	if(tf)
      return(((fgetc(fp) & 0xff) << 8) + (fgetc(fp) & 0xff));
	else
      return((fgetc(fp) & 0xff) + ((fgetc(fp) & 0xff) << 8));
}
unsigned long GetMIlng(FILE *fp, int tf)
{
	if(tf)
		 return(((unsigned long)(fgetc(fp) & 0xff) << 24) +
		  ((unsigned long)(fgetc(fp) & 0xff) << 16) +
		  ((unsigned long)(fgetc(fp) & 0xff) << 8) +
		  (unsigned long)(fgetc(fp) & 0xff));
	else
		return((unsigned long)(fgetc(fp) & 0xff) +
		  ((unsigned long)(fgetc(fp) & 0xff) << 8) +
		  ((unsigned long)(fgetc(fp) & 0xff) << 16) +
		  ((unsigned long)(fgetc(fp) & 0xff) << 24));
}

BOOL  IsErrorHand( DWORD dwh )
{
   if(( dwh == NO_OPEN  ) ||
      ( dwh == MEM_ERR  ) ||
      ( dwh == BAD_READ ) ||
      ( dwh == BAD_COPY ) ||
      ( dwh == NO_BHND  ) ||
      ( dwh == NO_GPTR  ) ||
      ( dwh == BAD_CODE ) ||
      ( dwh == BAD_FIRST) ||
      ( dwh == BAD_BIT  ) ||
      ( dwh == BAD_HEAD ) ||
      ( dwh == NO_BMP   ) ||
      ( dwh == NO_PAL   ) ||
      ( dwh == OS_RES   ) ||   // not supported by OS
      ( dwh == NO_SUP   ) )    // file format NOT supported
   {
      return TRUE;
   }

   return FALSE;
}

DWORD picbits, linewidth;
typedef struct {
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[256];
} SBITMAPINFO;
SBITMAPINFO bmpi; // note: set at maximum colours available in an RGBQUAD = 256

#define pixels2bytes(n) ((n + 7) / 8)

int SetLine(VOID)
{

	int j, width, icnt;
   //BITMAPINFOHEADER * pih = &sBmpInfHdr;
   BITMAPINFOHEADER * pih = &bih;

   pih->biSize          = sizeof(BITMAPINFOHEADER);
	// sBmpInfHdr.biSize = 40;
   pih->biPlanes        = 1;   //sBmpInfHdr.biPlanes = 1;
   pih->biCompression   = 0;   //sBmpInfHdr.biCompression = 0;
   pih->biSizeImage     = 0;   //sBmpInfHdr.biSizeImage = 0;
   pih->biXPelsPerMeter = 0;  //sBmpInfHdr.biXPelsPerMeter = 0;
   pih->biYPelsPerMeter = 0;  // sBmpInfHdr.biYPelsPerMeter = 0;
   pih->biClrUsed       = 0;  // sBmpInfHdr.biClrUsed = 0;
   pih->biClrImportant  = 0;  // sBmpInfHdr.biClrImportant = 0;
   for( j = 0; j < 256; j++ )
   {
       // transfer the colour to the BITMAPINFOHEADER colour section (256 fixed)
		bmpi.bmiColors[j].rgbRed      = palette[j * 3 + 0];
		bmpi.bmiColors[j].rgbGreen    = palette[j * 3 + 1];
		bmpi.bmiColors[j].rgbBlue     = palette[j * 3 + 2];
		bmpi.bmiColors[j].rgbReserved = 0;
	}

	if(pih->biBitCount == 1)
   {
      icnt  = 2;
		width = pixels2bytes(pih->biWidth);
   }
	else if(pih->biBitCount > 1 && pih->biBitCount <= 4)
   {
		width = pixels2bytes(pih->biWidth) << 2;
		pih->biBitCount = 4;
      icnt = 16;
	}
	else if(pih->biBitCount > 4 && pih->biBitCount <= 8)
   {
		width = pih->biWidth;
		pih->biBitCount = 8;
      icnt = 256;
	}
	else if(pih->biBitCount > 8)
   {
		width = pih->biWidth * 3;
		pih->biBitCount = 24;
      icnt = 0;
	}

   if(width & 0x003)
      width = (width | 3) + 1;

   pih->biSizeImage = pih->biHeight * width; // set IMAGE size into structure

//   bmpi.bmiHeader = sBmpInfHdr;   // copy the INFO header into the FILEINFO block.
   bmpi.bmiHeader = *pih;   // copy the INFO header into the FILEINFO block.

   return(width);
}

static   TCHAR _s_szbuf[256];
LE sStgList = { &sStgList, &sStgList };
LE sInfoList = { &sInfoList, &sInfoList };

#define  CHKMEM(a)   if(!a)   \
   {  fclose(fp);             \
      chkme( "C:ERROR: MEMORY FAILED!"MEOR ); \
      return((HANDLE)MEM_ERR); \
   }

VOID  KillTifLists( VOID )
{
   PLE      ph    = &sStgList;  // get head of a list of strings found in the TIFF
   PLE      phInf = &sInfoList;
   KillLList(ph);    // remove any previous strings
   KillLList(phInf); // and info lines

}

#define  OUTINFO  \
{\
   DWORD len = (DWORD)strlen(lpi);\
   if(len) {\
         pnInf = (PLE)MALLOC( sizeof(LE) + len + 1 );\
         CHKMEM(pnInf);\
         pstg = (LPTSTR)((PLE)pnInf + 1);\
         strcpy(pstg,lpi);\
         sprtf(pstg);\
         InsertTailList(phInf,pnInf);\
         *lpi = 0; }\
}


HANDLE ReadTIF6( FILE * fp )
{
   LPTSTR   lpi   = gszTifInfo;
   PLE      ph    = &sStgList;  // get head of a list of strings found in the TIFF
   PLE      phInf = &sInfoList;
   BITMAPINFOHEADER * pih = &bih;
   PLE      pn, pnInf;
	DWORD    c, i, j, k, n, row = 0, t;
	int      colormap = FALSE;
//	BOOL     IsMotor = FALSE;
	char     *line;
    long    bufsize;
//    unsigned long length, offset, pos, strtstrip, max;
   DWORD length, offset, pos, strtstrip, max;
   unsigned int  numtags, photo = 0, rows = 0, numstrips = 1, compress = 1;
   unsigned int  bitsamples = 1, bitspersample = 1, tag, type;
//   unsigned char * gptr;
   unsigned char * gptr;
//    int           bytes;
   DWORD           bytes;
   int   vers;
//   ucp   pcopy = 0;  // if a COPY of the bitmap data is made
//   HANDLE   hcopy = 0;  // global alloacted (moveable) memory
   DWORD    icnt = 0;
   TIFFDirEntry   * ptd = 0;   // pointer to copy of TIFF directory entries
   LPTSTR      lpt, pstg;  // type of data in tag, and any ASCII string
   LONG        value;

   numstrips = 0; // there can be no 'default' for this, like
   strtstrip = 0;
   TileWidth = TileLength = TileOffsets = TileByteCounts = 0;

//	strcpy(path, fname);
//	if((fp = fopen(path, "rb")) == NULL) return(NO_OPEN);

   /* Set the end of the file: */
   fseek( fp, 0L, SEEK_END );
   max = ftell( fp );
   fseek( fp, 0L, SEEK_SET );
	pos = ftell(fp);

   sprintf(lpi, "Processing file of %u bytes ..."MEOR, max );

	fread(buf, 1, 2, fp);
   if(!_fmemcmp(buf, "II", 2) && !_fmemcmp(buf, "MM", 2)) {
    	fclose(fp);
      sprintf(EndBuf(lpi), "NOT TIFF6 - Does NOT begin II or MM!"MEOR );
      OUTINFO;
    	return((HANDLE)BAD_HEAD);
   }

   //if(_fmemcmp(buf, "II", 2) == 0) IsMotor = TRUE;
   if(_fmemcmp(buf, "MM", 2) == 0)
   {
      strcat(lpi, "Type is MM = Motorola big endians"MEOR );
      IsMotor = TRUE;
   }
   else
   {
      strcat(lpi, "Type is II = Intel little endians"MEOR );
      IsMotor = FALSE;
   }

   memset(palette, 0, 768);
   memset(pih,     0, sizeof(BITMAPINFOHEADER));   //sBmpInfHdr - ensure NOTHING here

    //getw(fp); this was here, but there should be 2! getw(fp);
   vers = GetMIw(fp, IsMotor);  // better

   sprintf(EndBuf(lpi), "Version like # %d (should be 42!)"MEOR, vers );

   offset = GetMIlng(fp, IsMotor);

   if( (offset + 4) >= max )
   {
    	fclose(fp);
      sprintf(EndBuf(lpi), "NOT TIFF6 - Offset %u beyond file %u"MEOR,
         offset, max );
    	return((HANDLE)BAD_HEAD);
   }

   fseek(fp, offset, SEEK_SET);
   numtags = GetMIw(fp, IsMotor);
   if(( numtags == 0 ) || ( ( offset + (numtags * 12) ) > max ) ) {
    	fclose(fp);
      sprintf(EndBuf(lpi), "NOT TIFF6 - Number of tags %u (0 or beyond file end)"MEOR,
         numtags );
    	return((HANDLE)BAD_HEAD);
   }

   ptd = (TIFFDirEntry *)malloc( sizeof(TIFFDirEntry) * numtags );
   if(!ptd)
   {
      fclose(fp);
      chkme( "C:ERROR: MEMORY FAILED!"MEOR );
      return((HANDLE)MEM_ERR);
   }

   for( t = 0; t < numtags; t++ )
   {
      ptd[t].tdir_tag    = GetMIw(fp, IsMotor);
      ptd[t].tdir_type   = GetMIw(fp, IsMotor);
      ptd[t].tdir_count  = GetMIlng(fp, IsMotor);
      ptd[t].tdir_offset = GetMIlng(fp, IsMotor);
   }

   KillLList(ph);    // remove any previous strings
   KillLList(phInf); // and info lines

   pnInf = (PLE)MALLOC( (UINT)(sizeof(LE) + strlen(lpi) + 1 ));
   CHKMEM(pnInf);
   pstg = (LPTSTR)((PLE)pnInf + 1);
   strcpy(pstg,lpi);
   sprtf(pstg);
   InsertTailList(phInf,pnInf);
   *lpi = 0;
   for(t = 0; t < numtags; ++t)
   {
      length = (DWORD)strlen(lpi);
      if(length)
      {
         pnInf = (PLE)MALLOC( sizeof(LE) + length + 1 );
         CHKMEM(pnInf);
         pstg = (LPTSTR)((PLE)pnInf + 1);
         strcpy(pstg,lpi);
         sprtf(pstg);
         InsertTailList(phInf,pnInf);
         *lpi = 0;
      }
      // extract the collected tags
      tag    = ptd[t].tdir_tag;
      type   = ptd[t].tdir_type;
      length = ptd[t].tdir_count;
      offset = ptd[t].tdir_offset;

      pos = ftell(fp);
      pstg = 0;      // no associated ASCII string yet
      /* Tag data type information.
       * Note: RATIONALs are the ratio of two 32-bit integer values. */
      value = -1;   // not valid value - ie no rational
      switch(type)
      {
      case TIFF_NOTYPE: //	= 0,	/* placeholder */
         lpt = "placholder";
         break;
      case TIFF_BYTE:   //	= 1,	/* 8-bit unsigned integer */
         lpt = "BYTE";
         break;
      case TIFF_ASCII:  //	= 2,	/* 8-bit bytes w/ last byte null */
         lpt = "ASCII";
         if( length )
         {
            INT   cnt, val;
            cnt = 0;
            fseek(fp, offset, SEEK_SET);
            pn = (PLE)MALLOC( sizeof(LE) + length + 1 );
            if(!pn)
            {
               chkme( "C:ERROR: Memory failed!"MEOR );
               return (HANDLE)MEM_ERR;
            }
            pstg = (LPSTR)((PLE)pn + 1);
            do
            {
               val = getc(fp);
               if( val != EOF )
               {
                  // note: this should also ADD the terminating ZERO byte/char
                  pstg[cnt++] = (char)val;
               }
            } while( val && ( val != -1 ) );

            if( val == -1 )
            {
               sprtf( "This is NOT a TIF file 6! OUT OF ASCII BEFORE ZERO"MEOR );
               //CloseMapFile(pmf);
               return (HANDLE)BAD_CODE;
            }
            // NB: NOTE *TBD* multiple zero terminated strings
            // ***********************************************
            if( (DWORD)cnt != length )
               sprtf( "WARNING: Not all text processed. %d vs %d."MEOR, length, cnt );
            // ***********************************************
            cnt--;   // back to count of characters found.
            InsertTailList(ph,pn);
            fseek(fp, pos, SEEK_SET);
         }
         break;
      case TIFF_SHORT:  //	= 3,	/* 16-bit unsigned integer */
         lpt = "SHORT";
         break;
      case TIFF_LONG:   //	= 4,	/* 32-bit unsigned integer */
         lpt = "LONG";
         break;
      case TIFF_RATIONAL:  //	= 5,	/* 64-bit unsigned fraction */
         lpt = "RATIONAL";
         {
            LONG  num, den;
            //sprintf(lpb, "Off %d, Len %d", off, len );
            mfseekc(fp, offset, SEEK_SET);
            num = mGetMIlng(fp, IsMoto);
            den = mGetMIlng(fp, IsMoto);
            if(den)
            {
               value = num / den;
            }
            else
            {
               sprintf(EndBuf(lpi), "WARNING: Denom. is ZERO! N=%d"MEOR,
                     num );
            }
         }
         break;
      case TIFF_SBYTE:  //	= 6,	/* !8-bit signed integer */
         lpt = "SBYTE";
         break;
      case TIFF_UNDEFINED: //	= 7,	/* !8-bit untyped data */
         lpt = "undefined";
         break;
      case TIFF_SSHORT: //	= 8,	/* !16-bit signed integer */
         lpt = "SHORT";
         break;
      case TIFF_SLONG:  //	= 9,	/* !32-bit signed integer */
         lpt = "LONG";
         break;
      case TIFF_SRATIONAL: //	= 10,	/* !64-bit signed fraction */
         lpt = "SRATIONAL";
         break;
      case TIFF_FLOAT:  //	= 11,	/* !32-bit IEEE floating point */
         lpt = "float";
         break;
      case TIFF_DOUBLE: //	= 12	/* !64-bit IEEE floating point */
         lpt = "double";
         break;
      default:
         lpt = _s_szbuf;
         sprintf(lpt, "*UNLISTED Type = %d!*", type );
      }
//       TIFFDataType;

      switch(tag)
      {
      case TIFFTAG_SUBFILETYPE:  // 254 - subfile data descriptor
//#define	    FILETYPE_REDUCEDIMAGE	0x1	/* reduced resolution version */
//#define	    FILETYPE_PAGE		0x2	/* one page of many */
//#define	    FILETYPE_MASK		0x4	/* transparency mask */
         sprintf(EndBuf(lpi), "TT: subfile desc. %s",
            ((offset & FILETYPE_REDUCEDIMAGE) ? "reduced resolution version" :
            (offset & FILETYPE_PAGE) ? "one page of many" :
            (offset & FILETYPE_MASK) ? "transparency mask" : "unlisted") );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;
      case TIFFTAG_OSUBFILETYPE: // 255 - +kind of data in subfile
//#define	    OFILETYPE_IMAGE		1	/* full resolution image data */
//#define	    OFILETYPE_REDUCEDIMAGE	2	/* reduced size image data */
//#define	    OFILETYPE_PAGE		3	/* one page of many */
         sprintf(EndBuf(lpi), "TT: osubfile desc. %s",
            ((offset == OFILETYPE_IMAGE) ? "1 = full res. image" :
             (offset == OFILETYPE_REDUCEDIMAGE) ? "2 = reduced image" :
             (offset == OFILETYPE_PAGE) ? "one page of many" : "unlisted!") );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;

      case TIFFTAG_IMAGEWIDTH:   // 256 -  image width in pixels
         pih->biWidth = offset;  // sBmpInfHdr.biWidth   = offset;
         sprintf(EndBuf(lpi), "TT: image width = %u pixels", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;

      case TIFFTAG_IMAGELENGTH:  // 257 - image height in pixels
         pih->biHeight = offset; // sBmpInfHdr.biHeight = offset;
         sprintf(EndBuf(lpi), "TT: image height = %u pixels", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;

      case TIFFTAG_BITSPERSAMPLE:   // 258 = bits per channel (sample)
         if( length > 1L )
         {
            pos = ftell(fp);
            fseek(fp, offset, SEEK_SET);
        		bitspersample = GetMIw(fp, IsMotor);
        		fseek(fp, pos, SEEK_SET);
        		
         }
         else
            bitspersample = offset;

         sprintf(EndBuf(lpi), "TT: bitspersample = %u", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;

      case TIFFTAG_COMPRESSION:  // 259 - data compression technique
//#define	    COMPRESSION_NONE		1	/* dump mode */
         compress = offset;
         line = GetCompStg( compress );
         sprintf(EndBuf(lpi), "TT: compression = %s. (%#x)", line, offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;

      case TIFFTAG_PHOTOMETRIC:  // 262 = photometric interpretation
//#define	    PHOTOMETRIC_MINISWHITE	0	/* min value is white */
         photo = offset;
         line = GetPhotoStg( photo );
         sprintf(EndBuf(lpi), "TT: photometric = %s. (%#x)", line, offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;

      case TIFFTAG_THRESHHOLDING:   // 263	/* +thresholding used on data */
//#define	    THRESHHOLD_BILEVEL		1	/* b&w art scan */
//#define	    THRESHHOLD_HALFTONE		2	/* or dithered scan */
//#define	    THRESHHOLD_ERRORDIFFUSE	3	/* usually floyd-steinberg */
         sprintf(EndBuf(lpi), "TT: thresholding. %s",
            ((offset == THRESHHOLD_BILEVEL) ? "1 = b&w art scan" :
             (offset == THRESHHOLD_HALFTONE) ? "2 = dithered scan" :
             (offset == THRESHHOLD_ERRORDIFFUSE) ? "3 = usually floyd-steinberg" :
             "unlisted!") );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );

         break;
      case TIFFTAG_CELLWIDTH: // 264 /* +dithering matrix width */
         sprintf(EndBuf(lpi), "TT: dither width = %u", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;

      case TIFFTAG_CELLLENGTH:   //	265 /* +dithering matrix height */
         sprintf(EndBuf(lpi), "TT: dither height = %u", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;
      case TIFFTAG_FILLORDER: // 266	/* data order within a byte */
//#define	    FILLORDER_MSB2LSB		1	/* most significant -> least */
//#define	    FILLORDER_LSB2MSB		2	/* least significant -> most */
         sprintf(EndBuf(lpi), "TT: bit order = %s (%u)",
            (( offset == 1 ) ? "most2least" :
         ( offset == 2 ) ? "least2most" : "not listed" ),
            offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;
      case TIFFTAG_DOCUMENTNAME: // 269	/* name of doc. image is from */
      case TIFFTAG_IMAGEDESCRIPTION:   //	270	/* info about image */
      case TIFFTAG_MAKE:   //		271	/* scanner manufacturer name */
      case TIFFTAG_MODEL:  //			272	/* scanner model name/number */
      case TIFFTAG_PAGENAME:  // 285	/* page name image is from */
         {
            LPTSTR   pdesc;
            if( tag == TIFFTAG_DOCUMENTNAME ) // 269	/* name of doc. image is from */
               pdesc = "document name";
            else if( tag == TIFFTAG_IMAGEDESCRIPTION)   //	270	/* info about image */
               pdesc = "image info";
            else if( tag == TIFFTAG_MAKE)   //		271	/* scanner manufacturer name */
               pdesc = "scanner mfg name";
            else if( tag == TIFFTAG_MODEL)  //			272	/* scanner model name/number */
               pdesc = "scanner model";
            else // if( tag == TIFFTAG_PAGENAME )  // 285 - note moved position
            if(pstg)
            {
               sprintf(EndBuf(lpi), "TT: %s = "MEOR
                  "[%s]",
                  pdesc,
                  pstg );
            }
            else
            {
               sprintf(EndBuf(lpi), "TT: %s = NULL", pdesc );
            }
            sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         }
         break;

      case TIFFTAG_STRIPOFFSETS: // 273 /* offsets to data strips */
         //	if(type == 4) strtstrip = offset;
        	//	else strtstrip = offset & 0xffffL;
         sprintf(EndBuf(lpi), "TT: strip offsets at %u", offset );
         strtstrip = offset;
         numstrips = length;
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;
      case TIFFTAG_ORIENTATION:  // 274	/* +image orientation */
//#define	    ORIENTATION_TOPLEFT		1	/* row 0 top, col 0 lhs */
         line = GetOrientStg( photo );
         sprintf(EndBuf(lpi), "TT: orientation = %s. (%#x)", line, offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;

      case TIFFTAG_SAMPLESPERPIXEL: // 277	/* samples per pixel */
         bitsamples = offset;
         sprintf(EndBuf(lpi), "TT: samples per pixel = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
        	break;

      case TIFFTAG_ROWSPERSTRIP: // 278	/* rows per strip of data */
     		rows = offset;
         sprintf(EndBuf(lpi), "TT: rows per strip = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_STRIPBYTECOUNTS: //		279	/* bytes counts for strips */
         sprintf(EndBuf(lpi), "TT: bytes counts for strips at %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_MINSAMPLEVALUE:  // 280	/* +minimum sample value */
         sprintf(EndBuf(lpi), "TT: min. samp. val. = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_MAXSAMPLEVALUE:  // 281	/* +maximum sample value */
         sprintf(EndBuf(lpi), "TT: max. samp. val. = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_XRESOLUTION:  // 282	/* pixels/resolution in x */
         sprintf(EndBuf(lpi), "TT: pix/res. in x at %u ", offset );
         // expect RATIONAL
         if(value != -1)
            sprintf(EndBuf(lpi), "= %d.", value );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_YRESOLUTION:  //	283	/* pixels/resolution in y */
         sprintf(EndBuf(lpi), "TT: pix/res. in y at %u ", offset );
         if(value != -1)
            sprintf(EndBuf(lpi), "= %d.", value );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_PLANARCONFIG: //		284	/* storage organization */
//#define	    PLANARCONFIG_CONTIG		1	/* single image plane */
//#define	    PLANARCONFIG_SEPARATE	2	/* separate planes of data */
         sprintf(EndBuf(lpi), "TT: storage prg. = %s.",
            (( offset == 1 ) ? "single" :
         ( offset == 2) ? "separate" : "out of range" ) );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;

//      case TIFFTAG_PAGENAME:  // 285	/* page name image is from */
      case TIFFTAG_XPOSITION: //		286	/* x page offset of image lhs */
         sprintf(EndBuf(lpi), "TT: x page offset of image lhs = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
      case TIFFTAG_YPOSITION: // 287	/* y page offset of image lhs */
         sprintf(EndBuf(lpi), "TT: y page offset of image lhs = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
     		break;
#if 0
#define	TIFFTAG_FREEOFFSETS		288	/* +byte offset to free block */
#define	TIFFTAG_FREEBYTECOUNTS		289	/* +sizes of free blocks */
#define	TIFFTAG_GRAYRESPONSEUNIT	290	/* $gray scale curve accuracy */
#define	    GRAYRESPONSEUNIT_10S	1	/* tenths of a unit */
#define	    GRAYRESPONSEUNIT_100S	2	/* hundredths of a unit */
#define	    GRAYRESPONSEUNIT_1000S	3	/* thousandths of a unit */
#define	    GRAYRESPONSEUNIT_10000S	4	/* ten-thousandths of a unit */
#define	    GRAYRESPONSEUNIT_100000S	5	/* hundred-thousandths */
#define	TIFFTAG_GRAYRESPONSECURVE	291	/* $gray scale response curve */
#define	TIFFTAG_GROUP3OPTIONS		292	/* 32 flag bits */
#define	TIFFTAG_T4OPTIONS		292	/* TIFF 6.0 proper name alias */
#define	    GROUP3OPT_2DENCODING	0x1	/* 2-dimensional coding */
#define	    GROUP3OPT_UNCOMPRESSED	0x2	/* data not compressed */
#define	    GROUP3OPT_FILLBITS		0x4	/* fill to byte boundary */
#define	TIFFTAG_GROUP4OPTIONS		293	/* 32 flag bits */
#define TIFFTAG_T6OPTIONS               293     /* TIFF 6.0 proper name */
#define	    GROUP4OPT_UNCOMPRESSED	0x2	/* data not compressed */
#define	TIFFTAG_RESOLUTIONUNIT		296	/* units of resolutions */
#define	    RESUNIT_NONE		1	/* no meaningful units */
#define	    RESUNIT_INCH		2	/* english */
#define	    RESUNIT_CENTIMETER		3	/* metric */
#define	TIFFTAG_PAGENUMBER		297	/* page numbers of multi-page */
#define	TIFFTAG_COLORRESPONSEUNIT	300	/* $color curve accuracy */
#define	    COLORRESPONSEUNIT_10S	1	/* tenths of a unit */
#define	    COLORRESPONSEUNIT_100S	2	/* hundredths of a unit */
#define	    COLORRESPONSEUNIT_1000S	3	/* thousandths of a unit */
#define	    COLORRESPONSEUNIT_10000S	4	/* ten-thousandths of a unit */
#define	    COLORRESPONSEUNIT_100000S	5	/* hundred-thousandths */
#define	TIFFTAG_TRANSFERFUNCTION	301	/* !colorimetry info */
#define	TIFFTAG_SOFTWARE		305	/* name & release */
#define	TIFFTAG_DATETIME		306	/* creation date and time */
#define	TIFFTAG_ARTIST			315	/* creator of image */
#define	TIFFTAG_HOSTCOMPUTER		316	/* machine where created */
#define	TIFFTAG_PREDICTOR		317	/* prediction scheme w/ LZW */
#define	TIFFTAG_WHITEPOINT		318	/* image white point */
#define	TIFFTAG_PRIMARYCHROMATICITIES	319	/* !primary chromaticities */
#endif   // 0 *TBD*

      case TIFFTAG_COLORMAP:  // 320	/* RGB map for pallette image */
         //pos = ftell(fp);
			colormap = TRUE;
			fseek(fp, offset, SEEK_SET);
			icnt = (1 << bitspersample);
         for( i = 0; i < icnt; ++i )
         {
					if(i >= 256)
                  break;
					c = GetMIw(fp, IsMotor);
					if((c & 0xff00) && !(c & 0x00ff))
                  c = c >> 8;
					palette[i * 3] = (char)c;
         }
			
         for(i = 0; i < icnt; ++i)
         {
					if(i >= 256) break;
					c = GetMIw(fp, IsMotor);
					if((c & 0xff00) && !(c & 0x00ff))
                  c = c >> 8;
					palette[i * 3 + 1] = (char)c;
         }

			for(i = 0; i < icnt; ++i)
         {
					if(i >= 256)
                  break;
					c = GetMIw(fp, IsMotor);
					if((c & 0xff00) && !(c & 0x00ff))
                  c = c >> 8;
					palette[i * 3 + 2] = (char)c;
         }

//            _showpal( &palette[0] ); // just a diagnostic dump of the colours
			fseek(fp, pos, SEEK_SET);

         sprintf(EndBuf(lpi), "TT: RGB palette = %u colours.", icnt );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
			break;

      case TIFFTAG_HALFTONEHINTS:   //	321	/* !highlight+shadow info */
         sprintf(EndBuf(lpi), "TT: hightlight+shadow info at %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
			break;

      case TIFFTAG_TILEWIDTH: //		322	/* !rows/data tile */
         TileWidth      = offset;
         sprintf(EndBuf(lpi), "TT: Tile Rows = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
			break;

      case TIFFTAG_TILELENGTH:   //		323	/* !cols/data tile */
         TileLength     = offset;
         sprintf(EndBuf(lpi), "TT: Tile Columns = %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;
      case TIFFTAG_TILEOFFSETS:  //		324	/* !offsets to data tiles */
         TileOffsets    = offset;
         sprintf(EndBuf(lpi), "TT: Tile Offset at %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;
      case TIFFTAG_TILEBYTECOUNTS:  //		325	/* !byte counts for tiles */
         TileByteCounts = offset;
         sprintf(EndBuf(lpi), "TT: Tile Byte Counts at %u.", offset );
         sprintf(EndBuf(lpi), " (typ=%s)"MEOR, lpt );
         break;

		}
	}


   length = (DWORD)strlen(lpi);
   if(length)
   {
      pnInf = (PLE)MALLOC( sizeof(LE) + length + 1 );
      CHKMEM(pnInf);
      pstg = (LPTSTR)((PLE)pnInf + 1);
      strcpy(pstg,lpi);
      sprtf(pstg);
      InsertTailList(phInf,pnInf);
      *lpi = 0;
   }
	pih->biBitCount = (WORD)picbits = bitspersample * bitsamples;

	bytes = (pih->biWidth * pih->biBitCount + 7) >> 3;
	linewidth = SetLine();

	if(colormap != TRUE) {

	    if(pih->biBitCount == 1) {
	    	if(photo != 1)
	    		memcpy(palette, "\377\377\377\000\000\000", 6);
	    	else
				memcpy(palette, "\000\000\000\377\377\377", 6);
         sprintf(EndBuf(lpi), "TT: Created palette with 2 entries."MEOR );
		}
		else {
			//n = 255 / ((1 << sBmpInfHdr.biBitCount) - 1);
			//for(i = 0; i < (DWORD)(1 << sBmpInfHdr.biBitCount); ++i)
         icnt = (1 << pih->biBitCount);
			n = 255 / (icnt - 1);
			for(i = 0; icnt; ++i)
			  	memset(palette + i * 3, i * n, 3);
         sprintf(EndBuf(lpi), "TT: Created palette with %d entries."MEOR, icnt );
		}
	}

   length = (DWORD)strlen(lpi);
   if(length)
   {

      pnInf = (PLE)MALLOC( sizeof(LE) + length + 1 );
      CHKMEM(pnInf);
      pstg = (LPTSTR)((PLE)pnInf + 1);
      strcpy(pstg,lpi);
      sprtf(pstg);
      InsertTailList(phInf,pnInf);
      *lpi = 0;

      //onInf->ir_id
   }

   if((line = (cp)malloc(max(bytes, linewidth))) == NULL) {
	    fclose(fp);
       chkme( "C:ERROR: MEMORY FAILED!"MEOR );
	    return((HANDLE)MEM_ERR);
   }

   bufsize = pih->biHeight * linewidth;   // size of the block
//   ghnd = GlobalAlloc(GMEM_MOVEABLE, bufsize);
   // now ADD the BITMAPINFOHEADER + RGBQUAD * num_of_colors(palsz)
//   if((gptr = (ucp)GlobalLock(ghnd)) == NULL) {
//    	free(line);
//    	fclose(fp);
//    	return((HANDLE)MEM_ERR);
//   }
   {
      LPSTR ptmp;
      DWORD palsz = PaletteSize((LPSTR)pih);
      DWORD bitso = sizeof(BITMAPINFOHEADER)   +
                    palsz; // =(sizeof(RGBQUAD) * num_of_colors)
      DWORD dibsz = bitso + bufsize;
      LPSTR pdib  = 0;
      ghnd = DVGlobalAlloc( GHND, dibsz );
      if(ghnd)
         pdib = (LPSTR)DVGlobalLock(ghnd);
      if(!pdib)
      {
         if(ghnd)
            DVGlobalFree(ghnd);
         ghnd = 0;
         free(line);
    	   fclose(fp);
         chkme( "C:ERROR: MEMORY FAILED!"MEOR );
    	   return((HANDLE)MEM_ERR);
      }
      else  // we have our DIB memory pointer
      {
         BITMAPINFOHEADER * pbmih = (BITMAPINFOHEADER *)pdib;
         *pbmih = *pih; // sBmpInfHdr; // copy BMP/DIB HEADER INFO
         if(palsz)
         {
            PBITMAPINFO pbi  = (PBITMAPINFO)&bmpi;
            ptmp = pdib + sizeof(BITMAPINFOHEADER);  // bump to COLOUR/COLOR table
            memcpy( ptmp, &pbi->bmiColors, palsz );   // =(sizeof(RGBQUAD) * num_of_colors)
         }
      }
      gptr = pdib + bitso; // set pointer to BITS array
   }

   if(rows == 0)
       rows = pih->biHeight;

   if( numstrips && strtstrip &&   // if a TIFF strip image
       pih->biWidth && pih->biHeight &&
       ( ( compress == 1 ) || ( compress == 0x8005 ) ) )
   {
      sprtf( "Processing %d strips, each for %d column bytes."MEOR,
         numstrips, rows );

   	for(j = 0; j < numstrips; ++j) {
   
   	    fseek(fp, strtstrip + (j * 4), SEEK_SET);
   
   	    if(numstrips > 1)
             fseek(fp, GetMIlng(fp, IsMotor), SEEK_SET);
   
   	    for(k = 0; k < rows; ++k) {
   			++row;
   
   			if(compress == 1)
   				fread(line, 1, bytes, fp);
            else if(compress == 0x8005) {
               	n = 0;
   				do {
   					c = fgetc(fp);
   					if(c & 0x80) {
   						if(c != 0x80) {
   							i = ((~c) & 0xff) + 2;
   							c = fgetc(fp);
   							while((i--) && (n < bytes)) line[n++] = (char)c;
   						}
   					}
   					else {
   						i = (c & 0xff) + 1;
   						while((i--) && (n < bytes)) line[n++] = fgetc(fp);
   					}
   				} while(n < bytes);
   			}
   
   			if(row > (DWORD)pih->biHeight)
               continue;
   
   			if(compress == 1 || compress == 0x8005) {
   				if(pih->biBitCount == 24)
   					for(i = 0; i < bytes - 3; i += 3) {
   						c = line[i];
   						line[i] = line[i + 2];
   						line[i + 2] = (char)c;
   					}
   
   				if(_fmemcpy(gptr + (pih->biHeight - row) * linewidth, line, bytes) == NULL) {
   					DVGlobalUnlock(ghnd);
   	        		DVGlobalFree(ghnd);
   					free(line);
   					fclose(fp);
   					return((HANDLE)BAD_COPY);
   				}
   			}
   		}
   	}
   }
   else if( TileWidth && TileLength && TileOffsets && TileByteCounts &&
      pih->biWidth && pih->biHeight )
   {
      // unpack a tiled TIFF6
      // some restrictions *tbd*
      if(( compress == 1        ) &&   // presently ONLY uncompressed data
         ( colormap == TRUE     ) &&
         ( pih->biBitCount == 8 ) ) // and only 8 BPP
      {
         ReadTIF6Tiled( gptr, bufsize, fp, IsMotor, line, linewidth );
      }
      else
      {
   		DVGlobalUnlock(ghnd);
   	   DVGlobalFree(ghnd);
         ghnd = 0;
   		free(line);
   		fclose(fp);
   		return((HANDLE)NO_SUP);   // unsupported format
      }
   }

	free(line);
	fclose(fp);

#if   0  // cancel
   i = Display(gptr, pichdc, pichwnd, dither, print, x, y, scale);
   if( i < 0 )
   {
       // NOTE: The CreateDIBitmap function has limitations
       if( IS_NT )
       {
          // MS 'suggest' it will work, so
       }
       else // !NT = if( IS_WIN95 )
       {
          // Windows 95/98/Me: The created bitmap cannot exceed 16MB in size.
          if( bufsize > 16 * 1024 * 1024 )
          {
#if   ( !defined(_USRDLL) && !defined(FILE32_EXPORTS) && defined(GVTEST) )
             q_write_dib( pcopy, bufsize );
#endif   // #if   ( !defined(_USRDLL) && !defined(FILE32_EXPORTS) && defined(GVTEST) )
             //return(OS_RES);
             i = OS_RES;   // os states it does NOT support size
          }
       }
   }

   if(pcopy)
   {
      DVGlobalUnlock(hcopy);
      i = (INT)hcopy;  //LocalFree(pcopy);
   }

#endif   // #if   0  // cancel

   //return(Display(gptr, pichdc, pichwnd, dither, print, x, y, scale));
	GlobalUnlock(ghnd);
	//GlobalFree(ghnd);

   //i = (INT)ghnd;

   return (HANDLE)ghnd; // return HANDLE to DIB memory - direct to GDI ...

}


typedef struct tagVAL2STG {   // value to string
   DWORD    val;
   LPTSTR   ptr;
}VAL2STG, * PVAL2STG;

VAL2STG  sCompStgs[] = {
   { COMPRESSION_NONE, "1 = dump mode" },
   { COMPRESSION_CCITTRLE, "2 = CCITT modified Huffman RLE" },
   { COMPRESSION_CCITTFAX3, "3 = CCITT Group 3 fax encoding" },
   { COMPRESSION_CCITT_T4,  "3 = CCITT T.4 (TIFF 6 name)" },
   { COMPRESSION_CCITTFAX4, "4 = CCITT Group 4 fax encoding" },
   { COMPRESSION_CCITT_T6,  "4 = CCITT T.6 (TIFF 6 name)" },
   { COMPRESSION_LZW,       "5 = Lempel-Ziv  & Welch" },
   { COMPRESSION_OJPEG,     "6 = !6.0 JPEG" },
   { COMPRESSION_JPEG,        "7 = pJPEG DCT compression" },
   { COMPRESSION_NEXT,  "32766 = NeXT 2-bit RLE" },
   { COMPRESSION_CCITTRLEW, "32771 = #1 w/ word alignment" },
   { COMPRESSION_PACKBITS, "32773 = Macintosh RLE" },
   { COMPRESSION_THUNDERSCAN, "32809 = ThunderScan RLE" },
/* codes 32895-32898 are reserved for ANSI IT8 TIFF/IT <dkelly@etsinc.com) */
   { COMPRESSION_IT8CTPAD, "32895 = IT8 CT w/padding" },
   { COMPRESSION_IT8LW, "32896 = IT8 Linework RLE" },
   { COMPRESSION_IT8MP, "32897 = IT8 Monochrome picture" },
   { COMPRESSION_IT8BL, "32898 = IT8 Binary line art" },
/* compression codes 32908-32911 are reserved for Pixar */
   { COMPRESSION_PIXARFILM, "32908 - Pixar companded 10bit LZW" },
   { COMPRESSION_PIXARLOG, "32909 - Pixar companded 11bit ZIP" },
   { COMPRESSION_DEFLATE, "32946 - Deflate compression" },
   { COMPRESSION_ADOBE_DEFLATE, "8 = Deflate compression, as recognized by Adobe" },
/* compression code 32947 is reserved for Oceana Matrix <dev@oceana.com> */
   { COMPRESSION_DCS, "32947 = Kodak DCS encoding" },
   { COMPRESSION_JBIG, "34661 = ISO JBIG" },
   { COMPRESSION_SGILOG, "34676 = SGI Log Luminance RLE" },
   { COMPRESSION_SGILOG24, "34677 = SGI Log 24-bit packed" },
   // end of table
   { 0, 0 }
};

static TCHAR   _s_sznotlisted[64];
LPTSTR   Val2Stg( DWORD val, PVAL2STG pv )
{
   LPTSTR   lps = pv->ptr;
   while(lps)
   {
      if( pv->val == val )
         return lps;
      pv++;
      lps = pv->ptr;
   }
   return NULL;
}

LPTSTR   GetCompStg( DWORD val )
{
   PVAL2STG pv = &sCompStgs[0];
   LPTSTR   lps = Val2Stg( val, pv );
   if(lps)
      return lps;
   lps = _s_sznotlisted;
   sprintf(lps, "Compress %u - not listed", val );
   return lps;
}

//      case TIFFTAG_PHOTOMETRIC:  // 262 = photometric interpretation
VAL2STG  sPhotoStgs[] = {
   { PHOTOMETRIC_MINISWHITE, "0 = min value is white" },
   { PHOTOMETRIC_MINISBLACK, "1 = min value is black" },
   { PHOTOMETRIC_RGB, "2 = RGB color model" },
   { PHOTOMETRIC_PALETTE, "3 = color map indexed" },
   { PHOTOMETRIC_MASK, "4 = $holdout mask" },
   { PHOTOMETRIC_SEPARATED, "5 = !color separations" },
   { PHOTOMETRIC_YCBCR, "6 = !CCIR 601" },
   { PHOTOMETRIC_CIELAB, "8 = !1976 CIE L*a*b*" },
   { PHOTOMETRIC_ITULAB, "10 = ITU L*a*b*" },
   { PHOTOMETRIC_LOGL, "32844 = CIE Log2(L)" },
   { PHOTOMETRIC_LOGLUV, "32845 = CIE Log2(L) (u',v')" },
   // end of table
   { 0, 0 }
};

LPTSTR   GetPhotoStg( DWORD val )
{
   PVAL2STG pv = &sPhotoStgs[0];
   LPTSTR   lps = Val2Stg( val, pv );
   if(lps)
      return lps;
   lps = _s_sznotlisted;
   sprintf(lps, "Photo %u - not listed", val );
   return lps;
}

//      case TIFFTAG_ORIENTATION:  // 274	/* +image orientation */
VAL2STG  sOrientStgs[] = {
   { ORIENTATION_TOPLEFT, "1 = row 0 top, col 0 lhs" },
   { ORIENTATION_TOPRIGHT, "2 = row 0 top, col 0 rhs" },
   { ORIENTATION_BOTRIGHT, "3 = row 0 bottom, col 0 rhs" },
   { ORIENTATION_BOTLEFT, "4 = row 0 bottom, col 0 lhs" },
   { ORIENTATION_LEFTTOP, "5 = row 0 lhs, col 0 top" },
   { ORIENTATION_RIGHTTOP, "6 = row 0 rhs, col 0 top" },
   { ORIENTATION_RIGHTBOT, "7	= row 0 rhs, col 0 bottom" },
   { ORIENTATION_LEFTBOT, "8 = row 0 lhs, col 0 bottom" },
   // last line
   { 0, 0 }
};
LPTSTR   GetOrientStg( DWORD val )
{
   PVAL2STG pv = &sOrientStgs[0];
   LPTSTR   lps = Val2Stg( val, pv );
   if(lps)
      return lps;
   lps = _s_sznotlisted;
   sprintf(lps, "Orientation %u - not listed", val );
   return lps;
}

// ===============
//#define  mftell      ftell
//#define  mfseekc     fseek
//#define  mfseek      fseek
//#define  mGetMIlng   GetMIlng
//#define  mgetc       getc

int   ReadTIF6Tiled( LPTSTR pbuf, DWORD bufsize, FILE * pmf, BOOL IsMotor,  // was fp
                LPTSTR line, DWORD width )
{
   int   iret = 0;
   BITMAPINFOHEADER * pbmp = &bih;  // &sBmpInfHdr;
   DWORD    ta,td, bytes, row, rows, k, n, rem;
//   LPTSTR   line, pbuf;
   DWORD    num, den, off, pos;
   INT      i;
   char     c;
   //DWORD    bufsize, width;
//         g_bmi.bmiHeader = *pbmp;   // copy in the HEADER
   if( !( TileWidth && TileLength && TileOffsets && TileByteCounts ) )
      return 0;

   ta    = (( pbmp->biWidth  + (TileWidth  - 1) ) / TileWidth  ); // tile count across
   td    = (( pbmp->biHeight + (TileLength - 1) ) / TileLength ); // tile count down
   num   = ta * td; // get total TILE count
   //int GetLineLen( PBITMAPINFOHEADER pbih )
//   width = GetLineLen( pbmp );
   bytes = ((pbmp->biWidth * pbmp->biBitCount) + 7) >> 3;

   off = TileOffsets;
   //sprtf( "Image is %d tiles (arrayed %d x %d), each of (%dx%d) at offset %#x."MEOR,
   //   ( ta * td ),
   //   ta, td,
   //   TileWidth,
   //   TileLength,
   //   TileOffsets );

	pos   = mftell(pmf);
   //ptmp  = (LPTSTR)pmf->pVoid; // get buffer
   //ptmp += off;
	//mfseekc(pmf, off, SEEK_SET);
//   num = ta * td; // set TILE count
   // perform a check run through of each of the offsets
//   for( i = 0; i < (INT)num; i++ )
//   {
//   	mfseekc(pmf, (off + ( i * 4 )), SEEK_SET);
//      den = mGetMIlng( pmf, IsMoto );
      //ptmp  = (LPTSTR)pmf->pVoid; // get buffer
      //ptmp += den;
//   	mfseekc(pmf, den, SEEK_SET);
//   }

   // make some allocations
//   line  = (LPTSTR)MALLOC( max(width,bytes) );
//   if(!line)
//   {
//      sprtf( "C:ERROR: Memory failed!"MEOR );
//      exit(-1);
//   }

   //pbuf  = (LPTSTR)MALLOC( ( pbmp->biHeight * GetLineLen( pbmp ) ) );
   //bufsize = pbmp->biHeight * GetLineLen( pbmp );
//   bufsize = pbmp->biHeight * width;
//   pbuf  = (LPTSTR)MALLOC( bufsize );
//   if( !pbuf)
//   {
//      sprtf( "C:ERROR: Memory failed!"MEOR );
//      exit(-1);
//   }

   rows  = pbmp->biHeight;
   row   = 0;

   sprtf( "Processing %d rows, each for %d column bytes, of %d tiles (%dx%d)."MEOR,
      rows, bytes,
      (ta * td),
      ta, td );

   off   = 0;
   for( k = 0; k < rows; k++ )
   {
      row++;   // bump the row being processed
      rem = ( k % TileLength );  // get remainder
      if( rem == 0 )
         i = (k / TileLength) * ta;  // get pointer to FIRST in this set
      off = rem * TileWidth; // compute row offset within a tile

      for( n = 0; n < bytes; n++ )  // process one big ROW for image bytes wide
      {
         if( ( n % TileWidth ) == 0 )  // time to change the TILE
         {
            // compute the tile required for these bytes
   			num = ( i + ( n / TileWidth ) );
   			//den = ( off + ( num * sizeof(PVOID) ) );
   			den = ( TileOffsets + ( num * sizeof(PVOID) ) );
   			mfseekc(pmf, den, SEEK_SET);
            // get this tiles offset
            den  = mGetMIlng( pmf, IsMoto );

            // add the offset for the row needed from this tile
            den  += off;   // bump to the particular ROW in this tile
            //ptmp  = (LPTSTR)pmf->pVoid; // get buffer
            //ptmp += den;
   	      mfseekc(pmf, den, SEEK_SET);
         }

         c       = mgetc(pmf);
         line[n] = c;

         //if(line[n] != -1)
         //   c = 0;
      }

      // insert line of bitmap into the appropriate place in the big buffer
   	//if(_fmemcpy(gptr + (sBmpInfHdr.biHeight - row) * linewidth, line, bytes) == NULL) {
   	if( !memcpy(pbuf + (pbmp->biHeight - row) * width, line, bytes) )
         break;
   }

   //MFREE(line);
   //Write_DIB_File( pbuf, bufsize );
   //MFREE(pbuf);

	mfseek(pmf, pos, SEEK_SET);
//   iret = (INT)pbuf;

   iret = 1;

   return iret;
}


#endif   // ADDTIFF6
// eof - DvTif.c
