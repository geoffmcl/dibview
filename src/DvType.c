
// DvType.c
// try to get the difinitive file type
#include "Dv.h"
// #include "DvType.h"

//#define  ft_BadDIB      -5
//#define  ft_NoRead      -4
//#define  ft_NULL        -3
//#define  ft_NoSeek      -2
//#define  ft_NoOpen      -1
//typedef struct tagBITMAPINFO256 { 
//   BITMAPINFOHEADER bmiHeader; 
//   RGBQUAD          bmiColors[256];
//} BITMAPINFO256, * PBITMAPINFO256; 

static TCHAR _s_buf[264];
static BITMAPINFO256    bi256;
BOOL  IsTifBytOrd( char c1, char c2 )
{
   if(( (c1 == 'I') && (c2 == 'I') ) ||
      ( (c1 == 'M') && (c2 == 'M') ) )
      return TRUE;

   return FALSE;
}

// VER_BUILD29	// fixes to readrgb - FIX20030724
DWORD	GetGType( LPTSTR lpf, LPTSTR lps, DWORD Siz )
{
	DWORD	ft;
	char	c1, c2;
	ft = ft_Undef;
	if( lps && Siz )
	{
		c1 = lps[0];
		if( Siz > 1 )
			c2 = lps[1];
		else
			c2 = 0x1a;

		if( ( c1 == 'B') && ( c2 == 'M') )
				ft = ft_BMP;
		else if( c1 == 'P' )
				ft = ft_PPM;	/* Input is maybe PPM ... */
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
        else if( c1 == 'G' )
				ft = ft_GIF;
        else if (((c1 & 0xff) == 0xff) &&
            ((c2 & 0xff) == 0xd8))
            ft = ft_JPG;
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		else if( c1 == 'R' )
				ft = ft_RLE;	/* Not presently supported ... */
		else if( c1 == 0 )
				ft = ft_TARGA;
#ifdef  ADDTIFF6    // Add Tag Info File Format - v6 = TIFF/IT  FIX20020917
      else if( IsTifBytOrd( c1, c2 ) )
            ft = ft_TIF;
#endif   // ADDTIFF6
      else {
         extern	WORD	InIndex;
         extern	BOOL	fChgInInd;
         extern BOOL	SetInIndex( LPSTR lpe );
         // and *MATCHING* type list
         extern UINT g_uTypExt[];

         INT   savinind = InIndex;
         BOOL  savinchg = fChgInInd;

         // not determined by the first 2 bytes ... hmmmm
         //		_splitpath (gszFile, szDrive, szDir, szFname, szExt);
   		DVGetFPath( lpf, gszDrive, gszDir, gszFname, gszExt );
         if( SetInIndex( gszExt ) ) {

            // set TYPE per EXTENT
            ft = g_uTypExt[InIndex];
         }
			//	ft = ft_Undef;
         InIndex   = (WORD)savinind;
         fChgInInd = savinchg;

      }
	}

	return ft;
}



INT   gmTypeOfFile( LPTSTR lpf )
{
   INT      typ = ft_Undef;
   FILE *   fp = fopen( lpf, "rb" );
   DWORD    max, num, pos;
   LPTSTR   lpb = _s_buf;

   if( fp == NULL )
      return   ft_NoOpen;

   if( fseek(fp, 0, SEEK_END) )
   {
      typ = ft_NoSeek;
      goto Done_File;
   }
   // note present max file = 4,294,967,295 bytes
   max = ftell(fp);
   if( max == 0 )
   {
      typ = ft_NULL;
      goto Done_File;
   }
   if( fseek(fp, 0, SEEK_SET) )
   {
      typ = ft_NoSeek;
      goto Done_File;
   }
   pos = ftell(fp);
   // first a common windows type
   // BMP or DIB file
   // typedef struct tagBITMAPFILEHEADER { 
   //    WORD    bfType; 
   //    DWORD   bfSize; 
   //    WORD    bfReserved1; 
   //    WORD    bfReserved2; 
   //    DWORD   bfOffBits; 
   // } BITMAPFILEHEADER, *PBITMAPFILEHEADER; 
   if( max > ( sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ) )
   {
      BITMAPFILEHEADER * ph = (BITMAPFILEHEADER *)lpb;
      BITMAPINFOHEADER * pi = (BITMAPINFOHEADER *)((PBITMAPFILEHEADER)ph + 1);
   	if(( fread((LPTSTR)ph,1,sizeof(BITMAPFILEHEADER),fp) != sizeof(BITMAPFILEHEADER) ) ||
   	   ( fread((LPTSTR)pi,1,sizeof(BITMAPINFOHEADER),fp) != sizeof(BITMAPINFOHEADER) ) )
      {
         typ = ft_NoRead;
         goto Done_File;
      }
      if( ph->bfType != DIB_HEADER_MARKER )
         goto Not_DIB;
      if( ph->bfSize != max )
      {
         typ = ft_BadDIB;
         goto Done_File;
      }
      if(( ph->bfOffBits == 0   ) ||
         ( ph->bfOffBits >= max ) )
      {
         typ = ft_BadDIB;
         goto Done_File;
      }
      // immediately following
      // typedef struct tagBITMAPINFO { 
      //    BITMAPINFOHEADER bmiHeader; 
      //    RGBQUAD          bmiColors[1]; 
      // } BITMAPINFO, *PBITMAPINFO; 
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
      //  } BITMAPINFOHEADER, *PBITMAPINFOHEADER;
      // *** OR ***
      //typedef struct _BITMAPCOREINFO { 
      //    BITMAPCOREHEADER  bmciHeader; 
      //    RGBTRIPLE         bmciColors[1]; 
      // } BITMAPCOREINFO, *PBITMAPCOREINFO; 
      //typedef struct tagBITMAPCOREHEADER {
      //  DWORD   bcSize; 
      //  WORD    bcWidth; 
      //  WORD    bcHeight; 
      //  WORD    bcPlanes; 
      //  WORD    bcBitCount; 
      //} BITMAPCOREHEADER, *PBITMAPCOREHEADER;
      if(( pi->biSize != sizeof(BITMAPINFOHEADER) ) &&
         ( pi->biSize != sizeof(BITMAPCOREHEADER) ) )
      {
         typ = ft_BadDIB;
         goto Done_File;
      }
      if( pi->biSize == sizeof(BITMAPINFOHEADER) )
      {
         bi256.bmiHeader = *pi;
         pi = &bi256.bmiHeader;
         num = 0;
         switch( pi->biBitCount )
         {
         case 0:  // BPP implied in JPEG or PNG
            break;   // no colout table
         case 1:  // monocrome
            num = 2; // two entries
            break;
         case 4:  // 16 colours
            num = 16;
            break;
         case 8:  // 256 colours
            num = 256;
            break;
         case 16: // 5-5-5 bit colour (hi-bit not used) = 2 ^ 16 colours
            break;
         case 24: // each 3 bytes is blue, green red resp. = 2 ^ 24 colours
            break;
         case 32:
            break;
         default:
            typ = ft_BadDIB;
            goto Done_File;
            break;
         }
      }
      else // if( pi->biSize == sizeof(BITMAPCOREHEADER) )
      {
         PBITMAPCOREINFO pci = (PBITMAPCOREINFO)pi;
         pi = &bi256.bmiHeader;
         pi->biSize = sizeof(BITMAPINFOHEADER);
         pi->biWidth = pci->bmciHeader.bcWidth;
         pi->biHeight = pci->bmciHeader.bcHeight;
         pi->biPlanes = pci->bmciHeader.bcPlanes;
         pi->biBitCount = pci->bmciHeader.bcBitCount;
      //  DWORD  biCompression; 
      //  DWORD  biSizeImage; 
      //  LONG   biXPelsPerMeter; 
      //  LONG   biYPelsPerMeter; 
      //  DWORD  biClrUsed; 
      //  DWORD  biClrImportant; 
         
      }

      typ = ft_BMP;
      goto Done_File;

   }

Not_DIB:

   if( fseek(fp, 0, SEEK_SET) )
   {
      typ = ft_NoSeek;
      goto Done_File;
   }

Done_File:

   fclose(fp);

   return typ;
}


// eof - DvType.c
