

// Rgb.c
#include <windows.h>
#include "showdib.h"
#include "..\diagfile.h"
//#include  "rgb.h"

// the pixel width (bmWidth) times the BPP (bits-per-pixel = bmBitsPixel)
// #define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)
// or
// biSizeImage = ((((biWidth * biBitCount) + 31) & ~31) >> 3) * biHeight:
// where biBitCount equal - 1, 4, 8, or 24 bits per pixel (BPP).

// rowno is an integer in the range 0 to YSIZE-1
// channo is an integer in the range 0 to ZSIZE-1 
//  rleoffset = starttab[rowno+channo*YSIZE]
//  rlelength = lengthtab[rowno+channo*YSIZE]

#ifdef   ADDRGB2
// ******************************************************************

LPTSTR   g_perr = 0;
TCHAR    gszTmpBuf[1024];

#define  uc   unsigned char
#define  puc  uc *
#define  mymalloc(a)    LocalAlloc( LPTR, a )
#define  myfree(a)      LocalFree( a )

PDWORD Read_RGB_Texture(char * name, 
   int * width, int * height, int * components );

typedef struct _RgbImageRec {
   unsigned short imagic;  // must be 474 decimal
//   unsigned short type;
   unsigned char type1;    // STORAGE - 0 = uncomp, 1 = RLE comp
   unsigned char type2;    // BPC     - 1 = norm. 2 = 2-bytes
   unsigned short dim;
   unsigned short xsize, ysize, zsize;
   unsigned int min, max;
   unsigned int wasteBytes;
   char name[80];
   unsigned long colorMap;
   char  fill[404];
} RgbImageRec, * PRgbImageRec;

// internal representation
typedef struct _ImageRec {
   unsigned short imagic;
   unsigned short type;
   unsigned short dim;
   unsigned short xsize, ysize, zsize;
   unsigned int min, max;
   unsigned int wasteBytes;
   char name[80];
   unsigned long colorMap;
   FILE *file;
   unsigned char *tmp, *tmpR, *tmpG, *tmpB;
   unsigned long rleEnd;
   unsigned int *rowStart;
   int *rowSize;
} ImageRec, * PImageRec;


static   RgbImageRec ir;
#define  MBOK(a,b)   MessageBox(NULL, a, b, (MB_OK|MB_ICONINFORMATION))

unsigned short getshort( puc cp )
{
   unsigned short val;
   val  = (cp[0] << 8);
   val += (cp[1] << 0);
   return( val & 0xffff );
}

unsigned long getlong( puc cp )
{
   unsigned long val;
	val  = (cp[0] << 24);
   val += (cp[1] << 16);
   val += (cp[2] << 8 );
   val += (cp[3] << 0 );
	return val;
}

int _putlong(HFILE hf, LONG val)
{
   unsigned char buf[4];
	buf[0] = (uc)(val>>24);
	buf[1] = (uc)(val>>16);
	buf[2] = (uc)(val>>8);
	buf[3] = (uc)(val>>0);
	return( _write(hf, &buf[0], 4 ) );
}

VOID expandrow( puc optr, puc iptr, int z )
{
	uc pixel, count;
	optr += z;
	while(1)
   {
	    pixel = *iptr++;
	    count = (pixel & 0x7f);

       if( !count )
          return;

	    if( pixel & 0x80 )
       {
          while(count--)
             *optr++ = *iptr++;
	    }
       else
       {
          pixel = *iptr++;
       }

       while(count--)
          *optr++ = pixel;
 
   }
}

TCHAR    g_szBMPFile[264];

HFILE GetBMPFile( LPTSTR pszFile )
{
   OFSTRUCT of;
   LPTSTR   lpf = g_szBMPFile;
   HFILE    hf;

   strcpy( lpf, pszFile );
   strcat( lpf, ".BMP"  );
   hf = OpenFile(lpf, &of, (UINT)OF_CREATE|OF_READWRITE);
   if((hf == 0) || (hf == -1) )
   {
      sprtf( "WARNING: Failed to CREATE file"MEOR
         "[%s]"MEOR, lpf );
      return 0;   // failed to CREATE file
   }

   return hf;
}


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

HANDLE   ReadRGBInfo( LPTSTR pszFile, HFILE * pfh,
                     int * width, int * height, int * components )
{
   static BITMAPFILEHEADER    hdr;
   static BITMAPINFOHEADER    bih;

   HFILE    hf = *pfh;
   DWORD    off;
   PRgbImageRec pir = &ir;
   DWORD    dwd,dwx,dwy,dwz,dwm, dwxx;
//   DWORD    dwiz;
   DWORD    dwi, dwh;
   DWORD    dwSizeImage;   // = ((((biWidth * biBitCount) + 31) & ~31) >> 3) * biHeight:
   LPTSTR   pstor, pbpc, pdim, pmap, pch;
//   unsigned long * starttab, * lengthtab;
   puc      pbits;
//   puc      prle;
   puc      pbs;     // source array
   puc      pbd;     // desitination array
   PDWORD   pimage;  // array of DWORDs RGB+a for width * height

   if( (hf == 0) || (hf == -1) )
   {
      g_perr = "File handle INVALID";
      return NULL;
   }

   if( 512 != sizeof(RgbImageRec) )
   {
      g_perr = "ERROR: Structure allignment error!";
      return NULL;
   }

   off = _llseek(hf, 0L, (UINT)FILE_BEGIN);
   off = _lread( hf, (LPTSTR)pir, (UINT)sizeof(RgbImageRec));
   if( sizeof(RgbImageRec) != off )
   {
      g_perr = "No header size in file";
      return NULL;
   }

   // check if it conforms
   if( getshort((puc)&pir->imagic) != 474 )  // != 0xda01 )   // = 474 intel reverse byte sense
   {
      g_perr = "No magic numer 474!";
      return NULL;   // that tears it ...
   }

   dwx = getshort( (puc)&pir->xsize );
   dwy = getshort( (puc)&pir->ysize );
   dwz = getshort( (puc)&pir->zsize );
   dwm = getlong(  (puc)&pir->colorMap );

   if(( dwx == 0 ) ||   // no width
      ( dwy == 0 ) )    // no height
   {
      g_perr = "Zero width or height!";
      return NULL;
   }

   if( dwz == 1 )
      pch = "B/W (greyscale)";
   else if( dwz == 3 )
      pch = "RGB Array";
   else if( dwz == 4 )
      pch = "RGB+Alpha";
   else
   {
      g_perr = "ZSize NOT 1, 3 or 4";
      return NULL;
   }

   dwxx  = WIDTHBYTES( dwx * 24 );  // 3 byte RGB;
   pbits = (puc)mymalloc( dwxx * dwy );
   if(!pbits)
   {
      g_perr = "ERROR: Memory for bits FAILED!";
      return NULL;
   }

   // STORAGE
   if( pir->type1 == 0 )
      pstor = "uncompressed";
   else if( pir->type1 == 1 )
      pstor = "RLE compressed";
   else
   {
      g_perr = "Storage NOT 0 or 1";
      myfree(pbits);
      return NULL;
   }

   // Bytes per Pixel
   if( pir->type2 == 1 )
      pbpc = "1";
   else if( pir->type2 == 2 )
      pbpc = "2";
   else
   {
      g_perr = "Not BPC 1 or 2!";
      myfree(pbits);
      return NULL;
   }

   dwd = getshort( (puc) &pir->dim );
   if( dwd == 1 )
      pdim = "One channel";
   else if( dwd == 2 )
      pdim = "Single channel";
   else if( dwd == 3 )
      pdim = "Multiple channels";
   else
   {
      g_perr = "Dimension NOT 1, 2, or 3!";
      return NULL;
   }

   if( dwm == 0 )
      pmap = "Normal";
   else if( dwm == 1 )
      pmap = "Dithered";
   else if( dwm == 2 )
      pmap = "Screen";
   else if( dwm == 3 )
      pmap = "Colormap";
   else
   {
      g_perr = "ColorMap NOT 0, 1, 2, or 3!";
      myfree(pbits);
      return NULL;
   }

   sprtf( "File %s has BMP %d x %d (z=%d)"MEOR, pszFile, dwx, dwy, dwz );
   sprtf( "Storage=%s BPC=%s Dimensions=%s Map=%s"MEOR,
      pstor, pbpc, pdim, pmap );

   pimage = Read_RGB_Texture( pszFile, width, height, components );
   if( !pimage )
   {
      g_perr = "Failed to LOAD RBG!";
      return NULL;
   }

   // ok, now WRITE a a BITMAP or DIB file
   // PDWORD pimage = array of DWORDs RGB+a for width * height
   hf = GetBMPFile( pszFile );
   if( !hf )
   {
      g_perr = "Failed to get BMP file!";
      myfree(pbits);
      myfree(pimage);
      return NULL;
   }
   if(( dwx != (DWORD)*width ) ||
      ( dwy != (DWORD)*height) )
   {
      g_perr = "STRUTH! RGB Routines MUST AGREE!";
      myfree(pbits);
      myfree(pimage);
      return NULL;
   }

   dwSizeImage = ( ( ( (dwx * 24) + 31 ) & ~31 ) >> 3 ) * dwy;
   if( dwSizeImage != ( dwxx * dwy ) )
   {
      g_perr = "YEEK! Sizes DO NOT AGREE!";
      myfree(pbits);
      myfree(pimage);
      return NULL;
   }

   bih.biSize        = sizeof(BITMAPINFOHEADER);
   bih.biWidth       = dwx;
   bih.biHeight      = dwy;
   bih.biPlanes      = 1;
   bih.biBitCount    = 24;
   bih.biCompression = BI_RGB;
   bih.biSizeImage   = dwSizeImage;
   bih.biXPelsPerMeter = 0;
   bih.biYPelsPerMeter = 0;
   bih.biClrUsed       = 0;
   bih.biClrImportant  = 0;


   hdr.bfType          = BFT_BITMAP;
   hdr.bfSize          = (DWORD)dwSizeImage + SIZEOF_BITMAPFILEHEADER_PACKED;
   hdr.bfReserved1     = 0;
   hdr.bfReserved2     = 0;
   hdr.bfOffBits       = (DWORD) (SIZEOF_BITMAPFILEHEADER_PACKED + bih.biSize);

   // watchout for the REVERSED scanline
   for( dwh = 0; dwh < dwy; dwh++ )
   {
      pbs = (puc) &pimage[ dwx  * (dwy - dwh - 1) ]; // point to the SOURCE ROW
      pbd =       &pbits [ dwxx * dwh ];
      // for each ROW in the image, process each scanline
      for( dwi = 0; dwi < dwx; dwi++ )
      {
         pbd[0] = pbs[2];
         pbd[1] = pbs[1];
         pbd[2] = pbs[0];
         pbd += 3;
         pbs += 4;
      }
   }

   off = _lwrite( hf, (LPCSTR)&hdr, SIZEOF_BITMAPFILEHEADER_PACKED );
   if( off != SIZEOF_BITMAPFILEHEADER_PACKED )
      goto Write_Err;
   off = _lwrite( hf, (LPCSTR)&bih, sizeof(BITMAPINFOHEADER) );
   if( off != sizeof(BITMAPINFOHEADER) )
      goto Write_Err;
   off = _lwrite( hf, pbits, dwSizeImage );
   if( off != dwSizeImage )
      goto Write_Err;

   goto Clean_Up;

Write_Err:
   g_perr = "YUK! File WRITE ERROR!";
   myfree(pbits);
   myfree(pimage);
   _lclose( hf );
   return NULL;

Clean_Up:

   _lclose( *pfh );  // close the RGB
   *pfh = hf;        // switch to BMP
   off = _llseek(hf, 0L, (UINT)FILE_BEGIN);
   strcpy(pszFile, g_szBMPFile );

   if(pbits)
      myfree(pbits);
   if(pimage)
      myfree(pimage);

   return( ReadDibBitmapInfo(hf) );
}

#if   0  // cancel this

   // if RLE, then the table follows
   if( pir->type1 == 1 )
   {
      // the file offsets are to the RLE for each scanline.
      // This information only applies
      // if the value for STORAGE above is 1. 
      DWORD tablen = dwy * dwz * sizeof(LONG);

      if(( dwy == 0 ) ||
         ( dwz == 0 ) )
      {
         g_perr = "RLE: y or z value is ZERO!";
         return NULL;
      }

      prle = (puc) mymalloc( dwxx );  //one scanline of data
      if(!prle)
      {
         g_perr = "ERROR: Memory for rle FAILED!";
         return NULL;
      }

      starttab  = (unsigned long *)mymalloc(tablen);
      lengthtab = (unsigned long *)mymalloc(tablen);
      if( !starttab || !lengthtab )
      {
         g_perr = "ERROR: Memory FAILED!";
         return NULL;
      }
      // we have already read in header, but
      off = _llseek( hf, sizeof(RgbImageRec), FILE_BEGIN);
      if( off != sizeof(RgbImageRec) )
      {
         g_perr = "Failed to seek to start table!";
         return NULL;
      }
      off = _lread( hf, starttab, tablen);
      if( off != tablen )
      {
         g_perr = "Unable to read start table!";
         return NULL;
      }
      off = _lread( hf, lengthtab, tablen);
      if( off != tablen )
      {
         g_perr = "Unable to read length table!";
         return NULL;
      }
      if( pir->type2 == 1 )
      {
         DWORD rleoffset;  // = starttab[rowno+channo*YSIZE]
         DWORD rlelength;  // = lengthtab[rowno+channo*YSIZE]
         dwiz = 0;
         for( dwi = 0; dwi < dwy; dwi++ )
         {
            rleoffset = getlong((puc) &starttab[dwi  + dwiz*dwy] );
            rlelength = getlong((puc) &lengthtab[dwi + dwiz*dwy] );
            if( !rleoffset || !rlelength )
            {
               g_perr = "EEK! Offset or length ZERO!";
               return NULL;
            }

            off = _llseek(hf, rleoffset, (UINT)FILE_BEGIN);
            if( off != rleoffset )
            {
               g_perr = "ERROR: Failed in file seek!";
               return NULL;
            }
            off = _lread( hf, (LPTSTR)prle, rlelength);
            pb  = (puc) &pbits[(WIDTHBYTES( dwx * 3 ) * dwi)];
            expandrow( pb, prle, dwiz );
         }
      }
      else
      {
         g_perr = "BPC=2: Yet to be implemented!";
         return NULL;
      }
   }
#endif   // 0

// *********** net source ***********

// Source Code
// A straightforward C library that reads SGI RGB files:
//  readrgb.h and readrgb.c and turns them into a 4 byte RGB and alpha channel.
//#include <stdio.h>
//#include <stdlib.h> 
//#include <string.h>

void bwtorgba(unsigned char *b,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = *b;
      l[1] = *b;
      l[2] = *b;
      l[3] = 0xff;
      l += 4; b++;
   }
}

void latorgba(unsigned char *b, unsigned char *a,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = *b;
      l[1] = *b;
      l[2] = *b;
      l[3] = *a;
      l += 4; b++; a++;
   }
}

puc rgbtorgba(unsigned char *r,unsigned char *g,
   unsigned char *b,unsigned char *l,int n) 
{
   while (n--)
   {
      l[0] = r[0];
      l[1] = g[0];
      l[2] = b[0];
      l[3] = 0xff;   // just use 0xff
      l += 4;        // bump to next 4 bytes of OUTPUT
      r++; g++; b++;
   }
   return l;
}

void rgbatorgba(unsigned char *r,unsigned char *g,
   unsigned char *b,unsigned char *a,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = r[0];
      l[1] = g[0];
      l[2] = b[0];
      l[3] = a[0];
      l += 4; r++; g++; b++; a++;
   }
}

void ImageClose(ImageRec *image);

static void ConvertShort(unsigned short *array, long length) 
{
   unsigned b1, b2;
   unsigned char *ptr;

   ptr = (unsigned char *)array;
   while (length--)
   {
      b1 = *ptr++;
      b2 = *ptr++;
      *array++ = (b1 << 8) | (b2);
   }
}

static void ConvertLong(unsigned *array, long length) 
{
   unsigned b1, b2, b3, b4;
   unsigned char *ptr;

   ptr = (unsigned char *)array;
   while (length--)
   {
      b1 = *ptr++;
      b2 = *ptr++;
      b3 = *ptr++;
      b4 = *ptr++;
      *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
   }
}

static ImageRec *ImageOpen(const char *fileName)
{
   union {
      int testWord;
      char testByte[4];
   } endianTest;
   ImageRec * image;
   int swapFlag;
   int x;
   LPTSTR   lpb = gszTmpBuf;

   endianTest.testWord = 1;
   if (endianTest.testByte[0] == 1) {
      swapFlag = 1;  // running on INTEL platform, so lots of SWAPS
   } else {
      swapFlag = 0;
   }

   image = (ImageRec *)mymalloc(sizeof(ImageRec));
   if( image == NULL )
   {
#ifdef   CONSOLE
      fprintf(stderr, "Out of memory!\n");
      exit(1);
#else // !CONSOLE
      sprintf( lpb, "While processing file [%s]"MEOR
         "an allocation of %d bytes of memory FAILED!",
         fileName, sizeof(ImageRec) );
      sprtf( "ERROR: %s"MEOR, lpb );
      MBOK( lpb, "MEMORY FAILED!" );
      return NULL;
#endif   // CONSOLE y/n
   }

   ZeroMemory(image, sizeof(ImageRec));

   if( (image->file = fopen(fileName, "rb")) == NULL )
   {
#ifdef   CONSOLE
      perror(fileName);
      exit(1);
#else // !CONSOLE
      sprintf(lpb, "Unable to open file"MEOR
         "[%s]!", fileName );
      sprtf( "ERROR: %s"MEOR, lpb );
      MBOK( lpb, "OPEN FILE FAILED!" );
      ImageClose(image);
      return NULL;
#endif   // CONSOLE y/n
   }

   fread(image, 1, 12, image->file);

   if(swapFlag)
      ConvertShort(&image->imagic, 6);

   image->tmp  = (unsigned char *)mymalloc(image->xsize*256);
   image->tmpR = (unsigned char *)mymalloc(image->xsize*256);
   image->tmpG = (unsigned char *)mymalloc(image->xsize*256);
   image->tmpB = (unsigned char *)mymalloc(image->xsize*256);

   if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
      image->tmpB == NULL)
   {
#ifdef   CONSOLE
      fprintf(stderr, "Out of memory!\n");
      exit(1);
#else // !CONSOLE
      sprintf( lpb, "While processing file [%s]"MEOR
         "an allocation of memory FAILED!",
         fileName );
      sprtf( "ERROR: %s"MEOR, lpb );
      MBOK( lpb, "MEMORY FAILED!" );
      ImageClose(image);
      return NULL;
#endif   // CONSOLE y/n
   }

   if( (image->type & 0xFF00) == 0x0100 )
   {
      // it is RLE encoded
      x = image->ysize * image->zsize * sizeof(unsigned);

      image->rowStart = (unsigned *)mymalloc(x);
      image->rowSize = (int *)mymalloc(x);

      if (image->rowStart == NULL || image->rowSize == NULL)
      {
#ifdef   CONSOLE
         fprintf(stderr, "Out of memory!\n");
         exit(1);
#else // !CONSOLE
         sprintf( lpb, "While processing file [%s]"MEOR
            "an allocation of memory FAILED!",
            fileName );
         sprtf( "ERROR: %s"MEOR, lpb );
         MBOK( lpb, "MEMORY FAILED!" );
         ImageClose(image);
         return NULL;
#endif   // CONSOLE y/n
      }

      image->rleEnd = 512 + (2 * x);
      fseek(image->file, 512, SEEK_SET);
      fread(image->rowStart, 1, x, image->file);
      fread(image->rowSize, 1, x, image->file);
      if(swapFlag)
      {
          ConvertLong(image->rowStart, x/(int)sizeof(unsigned));
          ConvertLong((unsigned *)image->rowSize, x/(int)sizeof(int));
      }
   }
   else
   {
      image->rowStart = NULL;
      image->rowSize = NULL;
   }

   return image;

}

void ImageClose(ImageRec *image) 
{
   if(image)
   {
      if(image->file)
         fclose(image->file);
   
      if(image->tmp)
         myfree(image->tmp);
   
      if(image->tmpR)
         myfree(image->tmpR);
   
      if(image->tmpG)
         myfree(image->tmpG);
   
      if(image->tmpB)
         myfree(image->tmpB);
   
      if(image->rowSize)
         myfree(image->rowSize);
   
      if(image->rowStart)
         myfree(image->rowStart);
   
      myfree(image);
   }
}

void ImageGetRow(ImageRec *image, 
   unsigned char *buf, int y, int z) 
{
   unsigned char *iPtr, *oPtr, pixel;
   int count;

   if ((image->type & 0xFF00) == 0x0100)
   {
      fseek(image->file, (long)image->rowStart[y+z*image->ysize], SEEK_SET);
      fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],
         image->file);

      iPtr = image->tmp;
      oPtr = buf;
      for (;;)
      {
          pixel = *iPtr++;
          count = (int)(pixel & 0x7F);
          if (!count)
          {
            return;
          }
          if (pixel & 0x80)
          {
            while (count--)
            {
                *oPtr++ = *iPtr++;
            }
          }
          else
          {
            pixel = *iPtr++;
            while (count--)
            {
                *oPtr++ = pixel;
            }
         }
      }
   }
   else
   {
      fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),
         SEEK_SET);
      fread(buf, 1, image->xsize, image->file);
   }
}

// RGBQUAD
//unsigned * read_texture(char *name, 
PDWORD Read_RGB_Texture(char * name, 
   int * width, int * height, int * components ) 
{
   LPTSTR   lpb = gszTmpBuf;
   PDWORD   base;
   PDWORD   lptr;
   puc   rbuf;
   puc   gbuf;
   puc   bbuf;
   puc   abuf;
   puc   l;
   ImageRec * image;
   int y;

   image = ImageOpen(name);
   if(!image)
      return NULL;

   if( !image->xsize || !image->ysize || !image->zsize )
   {
      sprintf( lpb, "While processing file [%s]"MEOR
            "either x, y, or z componet is NULL!",
            name );
      sprtf( "ERROR: %s"MEOR, lpb );
      MBOK( lpb, "BAD RGB IMAGE!" );
      ImageClose(image);
      return NULL;
   }

   (*width)     = image->xsize;
   (*height)    = image->ysize;
   (*components)= image->zsize;

   // get ONE large block of memory
   y = (image->xsize*image->ysize*sizeof(DWORD)) +
         (4 * (image->xsize*sizeof(unsigned char)));

   base = (PDWORD)mymalloc(y);

//   base = (unsigned *)mymalloc(image->xsize*image->ysize*sizeof(unsigned));
//   rbuf = (unsigned char *)mymalloc(image->xsize*sizeof(unsigned char));
//   gbuf = (unsigned char *)mymalloc(image->xsize*sizeof(unsigned char));
//   bbuf = (unsigned char *)mymalloc(image->xsize*sizeof(unsigned char));
//   abuf = (unsigned char *)mymalloc(image->xsize*sizeof(unsigned char));
//   if( !base || !rbuf || !gbuf || !bbuf )
   if( !base )
   {
      sprintf( lpb, "While processing file [%s]"MEOR
         "an allocation of %d bytes of memory FAILED!",
         name, y );
      sprtf( "ERROR: %s"MEOR, lpb );
      MBOK( lpb, "MEMORY FAILED!" );
      return NULL;
   }

   abuf = (puc)base;
   rbuf = &abuf[(image->xsize*image->ysize*sizeof(DWORD))];
   gbuf = &rbuf[(image->xsize*sizeof(unsigned char))];
   bbuf = &gbuf[(image->xsize*sizeof(unsigned char))];
   abuf = &bbuf[(image->xsize*sizeof(unsigned char))];

   lptr = base;
   for( y=0; y<image->ysize; y++ )
   {
      if( image->zsize >= 4 )
      {
         ImageGetRow(image,rbuf,y,0);
         ImageGetRow(image,gbuf,y,1);
         ImageGetRow(image,bbuf,y,2);
         ImageGetRow(image,abuf,y,3);
         rgbatorgba(rbuf,gbuf,bbuf,abuf,(unsigned char *)lptr,image->xsize);
         lptr += image->xsize;
      }
      else if(image->zsize==3)
      {
         ImageGetRow(image,rbuf,y,0);
         ImageGetRow(image,gbuf,y,1);
         ImageGetRow(image,bbuf,y,2);
         l = rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
         lptr += image->xsize;   // NOTE: bump by 4 * uc, or DWORDs
         if((PDWORD)l != lptr )
            sprtf( "EEK: What is wrong here %#x vs %#x?"MEOR, l, lptr );
      }
      else if(image->zsize==2)
      {
         ImageGetRow(image,rbuf,y,0);
         ImageGetRow(image,abuf,y,1);
         latorgba(rbuf,abuf,(unsigned char *)lptr,image->xsize);
         lptr += image->xsize;
      }
      else
      {
         ImageGetRow(image,rbuf,y,0);
         bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
         lptr += image->xsize;
      }
   }

   ImageClose(image);

   //myfree(rbuf);
   //myfree(gbuf);
   //myfree(bbuf);
   //myfree(abuf);

   return base; // return the DWORD array of RGB+a for width * height

}

// **********************************
#endif   // ADDRGB2


// eof - rgb.c
