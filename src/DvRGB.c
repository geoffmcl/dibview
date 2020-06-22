

// DvRGB.c
// read SGI rgba files
#include "dv.h"

#ifdef   ADD_SGI_RGB_READ

//	VER_BUILD29	// fixes to readrgb - FIX20030724
//#define  ft_RGB      8  // RGB (SGI texture files)
// readrgb.c
//#include <windows.h>
//#include <stdio.h>
//#include <stdlib.h> 
//#include <string.h>
//#include "readrgb.hxx"

// WIDTHBYTES takes # of bits in a scan line and rounds up to nearest
//  dword (32-bits). The # of bits in a scan line would typically be
//  the pixel width (bmWidth) times the BPP (bits-per-pixel = bmBitsPixel)
#define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)
//--------------  DIB header Marker Define -------------------------
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')	/* Simple "BM" ... */


typedef struct tagRGBHDR {
   unsigned short rgb_imagic;
   unsigned short rgb_type;
   unsigned short rgb_dim;
   unsigned short rgb_xsize, rgb_ysize, rgb_zsize;
}RGBHDR, * PRGBHDR;

RGBHDR   sRGBHdr;
#ifndef  dv_fmemset
#define  dv_fmemset(a,b,c) ZeroMemory(a,c)
#endif   // #ifndef  dv_fmemset

extern void InitBitmapInfoHeader( LPBITMAPINFOHEADER lpBmInfoHdr,
						  DWORD dwWidth,
						  DWORD dwHeight,
						  int nBPP );

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

void rgbtorgba(unsigned char *r,unsigned char *g,
   unsigned char *b,unsigned char *l,int n) 
{
   while (n--) {
      l[0] = r[0];
      l[1] = g[0];
      l[2] = b[0];
      l[3] = 0xff;
      l += 4; r++; g++; b++;
   }
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
} ImageRec;

static void ConvertShort(unsigned short *array, long length) 
{
   unsigned b1, b2;
   unsigned char *ptr;

   ptr = (unsigned char *)array;
   while (length--) {
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
   while (length--) {
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
   ImageRec *image;
   int swapFlag;
   int x;
   PRGBHDR  phdr = &sRGBHdr;

   endianTest.testWord = 1;
   if (endianTest.testByte[0] == 1) {
      swapFlag = 1;
   } else {
      swapFlag = 0;
   }

   image = (ImageRec *)malloc(sizeof(ImageRec));
   if (image == NULL) {
      fprintf(stderr, "Out of memory!\n");
      exit(1);
   }
   if ((image->file = fopen(fileName, "rb")) == NULL) {
      perror(fileName);
      exit(1);
   }

   if( fread(image, 1, 12, image->file) != 12 )
   {
      fprintf(stderr, "NOT an RGB file!\n");
      exit(1);
   }

   if (swapFlag) {
      memcpy( phdr, image, sizeof(RGBHDR) );
      sprtf( "PreHdr: magic %d type=%d dim=%d x=%d y=%d z=%d"MEOR,
         phdr->rgb_imagic,
         phdr->rgb_type,
         phdr->rgb_dim,
         phdr->rgb_xsize,
         phdr->rgb_ysize,
         phdr->rgb_zsize );
      ConvertShort(&image->imagic, 6);
   }

   memcpy( phdr, image, sizeof(RGBHDR) );

#ifdef   CONSOLE
   cout << "Header: magic " << phdr->rgb_imagic
      << " type " << phdr->rgb_type
      << " dim " << phdr->rgb_dim
      << " x " << phdr->rgb_xsize
      << " y " << phdr->rgb_ysize
      << " z " << phdr->rgb_zsize
      << endl;
#else // #ifdef   CONSOLE
   sprtf( "Header: magic %d type=%d dim=%d x=%d y=%d z=%d"MEOR,
      phdr->rgb_imagic,
      phdr->rgb_type,
      phdr->rgb_dim,
      phdr->rgb_xsize,
      phdr->rgb_ysize,
      phdr->rgb_zsize );
#endif      // #ifdef   CONSOLE

   image->tmp = (unsigned char *)malloc(image->xsize*256);
   image->tmpR = (unsigned char *)malloc(image->xsize*256);
   image->tmpG = (unsigned char *)malloc(image->xsize*256);
   image->tmpB = (unsigned char *)malloc(image->xsize*256);
   if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
      image->tmpB == NULL) {
      fprintf(stderr, "Out of memory!\n");
      exit(1);
   }

   if ((image->type & 0xFF00) == 0x0100) {
      x = image->ysize * image->zsize * sizeof(unsigned);
      image->rowStart = (unsigned *)malloc(x);
      image->rowSize = (int *)malloc(x);
      if (image->rowStart == NULL || image->rowSize == NULL) {
         fprintf(stderr, "Out of memory!\n");
         exit(1);
      }
      image->rleEnd = 512 + (2 * x);
      fseek(image->file, 512, SEEK_SET);
      fread(image->rowStart, 1, x, image->file);
      fread(image->rowSize, 1, x, image->file);
      if (swapFlag) {
          ConvertLong(image->rowStart, x/(int)sizeof(unsigned));
          ConvertLong((unsigned *)image->rowSize, x/(int)sizeof(int));
      }
   } else {
      image->rowStart = NULL;
      image->rowSize = NULL;
   }
   return image;
}

static void ImageClose(ImageRec *image) 
{
   fclose(image->file);
   free(image->tmp);
   free(image->tmpR);
   free(image->tmpG);
   free(image->tmpB);
   free(image->rowSize);
   free(image->rowStart);
   free(image);
}

static int ImageGetRow(ImageRec *image, 
   unsigned char *buf, int y, int z) 
{
   unsigned char *iPtr, *oPtr, pixel;
   unsigned int count;
   int offset;
   unsigned long size, total;

   total = 0;
   if ((image->type & 0xFF00) == 0x0100)
   {
      offset = image->rowStart[y+z*image->ysize];
      size   = image->rowSize[y+z*image->ysize];

      //fseek(image->file, (long)image->rowStart[y+z*image->ysize], SEEK_SET);
      //fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],image->file);
      //if( fseek(image->file, offset, SEEK_SET) != offset )
      //   return 0;
      fseek(image->file, offset, SEEK_SET);

      if( fread(image->tmp, 1, size, image->file) != size )
         return 0;

      iPtr = image->tmp;
      oPtr = buf;
      for (;;) // forever
      {
          pixel = *iPtr++; // get next byte
          count = (int)(pixel & 0x7F); // and count
          if (!count)
            return total;  // all done on this row

          total += count;
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
      offset = 512+(y*image->xsize)+(z*image->xsize*image->ysize);
      size   = image->xsize;

      //fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),SEEK_SET);
      //fread(buf, 1, image->xsize, image->file);
      if( fseek(image->file, offset, SEEK_SET) != offset )
         return 0;

      if( fread(buf, 1, size, image->file) != size )
         return 0;
   }

   return size;

}

//unsigned *read_texture(char *name, 
unsigned * read_rgb2bmp24(char * name,
   int *width, int *height, int *components )
{
   unsigned *base, *lptr;
   unsigned char *rbuf, *gbuf, *bbuf, *abuf;
   ImageRec *image;
   int y;

   image = ImageOpen(name);
    
   if(!image)
      return NULL;
   (*width)=image->xsize;
   (*height)=image->ysize;
   (*components)=image->zsize;
   base = (unsigned *)malloc(image->xsize*image->ysize*sizeof(unsigned));
   rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
   if(!base || !rbuf || !gbuf || !bbuf)
      return NULL;

   lptr = base;
//   for (y=0; y<image->ysize; y++)
//   {
      if (image->zsize>=4)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,gbuf,y,1) &&
            ImageGetRow(image,bbuf,y,2) &&
            ImageGetRow(image,abuf,y,3) )
         {
            rgbatorgba(rbuf,gbuf,bbuf,abuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else if(image->zsize==3)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,gbuf,y,1) &&
            ImageGetRow(image,bbuf,y,2) )
         {
            rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else if(image->zsize==2)
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0) &&
            ImageGetRow(image,abuf,y,1) )
         {
            latorgba(rbuf,abuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
      else
      {
   for (y=0; y<image->ysize; y++)
   {
         if(ImageGetRow(image,rbuf,y,0))
         {
            bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
            lptr += image->xsize;
         }
         else //return NULL;
         {
            free(base);
            base = 0;
            goto Exit_Read;
         }
   }
      }
//   }

Exit_Read:

   ImageClose(image);
   free(rbuf);
   free(gbuf);
   free(bbuf);
   free(abuf);

   return (unsigned *) base;
}

// input (and output file)
char *  _readrgb2bmp24( char * pfile, char * pbmp )
{
//   static char szbmpfile[264];
   int width, height, components, rows, cols;
   unsigned * base;
   int   size;
//   char * pfile = "D:/FG091/Scenery/ufo.rgb";
   int  palsz = 0;  // PaletteSize((LPSTR)lpbi);
   int  dibsz; // = sizeof(BITMAPINFOHEADER)   +
               // (sizeof(RGBQUAD) * palsz ) +
               //     bufsize;
   char * pdib;

#ifdef   CONSOLE
   cout << "Reading file [" << pfile << "]..." << endl;
#else // !#ifdef   CONSOLE
   sprtf( "Reading file [%s]..."MEOR, pfile );
#endif   // #ifdef   CONSOLE

   //image = read_texture( pfile,  // char *name, 
   base = read_rgb2bmp24( pfile, // char *name
      &width, &height, &components );

   if(!base) {
      sprtf( "Read FAILED in read_rgb2bmp24() service!"MEOR );
      return 0;
   }

   // base arranged in rows y, y=0 to ysize, in array
   // l[0] = r[0]; l[1] = g[0]; l[2] = b[0]; l[3] = a[0];
   cols = WIDTHBYTES(width * 24);   // = (((bits) + 31) / 32 * 4)
   rows = height;
   dibsz = sizeof(BITMAPINFOHEADER)   +
           (sizeof(RGBQUAD) * palsz ) +
           (cols * rows);  // =  bufsize;
   size  = sizeof(BITMAPFILEHEADER)   + dibsz;

   pdib  = (char *)malloc(size);
   if(!pdib)
   {
#ifdef   CONSOLE
      cout << "ERROR: Memory FAILED!" << endl;
#endif   // #ifdef   CONSOLE
      exit(-2);
      return 0;
   }
   else
   {
      BITMAPFILEHEADER * phdr = (BITMAPFILEHEADER *)pdib;
      LPBITMAPINFOHEADER pbi  = (LPBITMAPINFOHEADER)((BITMAPFILEHEADER *) phdr + 1);
      char * pbits            = (char *)((LPBITMAPINFOHEADER) pbi + 1);
//      char * pbmp             = szbmpfile;
      FILE * fp;
      int   y, x, row, col, alpha;
      char * pout;
      unsigned char * pin = (unsigned char *)base;
      for( y = 0; y < height; y++ ) // for each row
      {
         row = y;
         pout = &pbits[(row * cols)];
         for( x = 0; x < width; x++ )
         {
            col = x * 3;
            // note:intel positioning of the RGB bytes
            pout[col+2] = (char) pin[0];  // r
            pout[col+1] = (char) pin[1];  // g
            pout[col+0] = (char) pin[2];  // b
            alpha       =        pin[3];  // a
            pin += 4;
         }
      }

      InitBitmapInfoHeader( pbi,    // LPBITMAPINFOHEADER lpBmInfoHdr,
                           width,
                           height,
                           24 ); // 	  int nBPP )

      phdr->bfType  = DIB_HEADER_MARKER;   // simple "BM" signature
      phdr->bfSize  = size;   // file size
      phdr->bfReserved1 = 0;
      phdr->bfReserved2 = 0;
      phdr->bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER) + pbi->biSize + palsz;

      // strcpy(pbmp,pfile);
      // strcat(pbmp,".bmp");
      if(pbmp && *pbmp) {
         // WRITE a bitmap file
         fp = fopen(pbmp, "wb");
         if(fp)
         {
            fwrite(pdib, 1, size, fp);
            fclose(fp);
         }
         else
         {
   #ifdef   CONSOLE
            cout << "ERROR: Failed to create " << pbmp << "!" << endl;
   #else // !#ifdef   CONSOLE
            sprtf( "ERROR: Failed to create [%s]!"MEOR, pbmp );
   #endif   // #ifdef   CONSOLE
         }
      }
   }

   if(base)
      free(base);

   return pdib;

}

#define  bark  chkme("Check this ERROR!"MEOR)


HANDLE LoadRGBImage( PRDIB prd )
{
   static char szbmpfile[264];
   HANDLE   hDIB = NULL;   // return HANDLE (to global memory) of a DIB
   LPTSTR   lpf = prd->rd_pPath;
   char * pdib = _readrgb2bmp24( lpf, "temprgb1.bmp" );
   if(pdib) {
      HANDLE   ghnd;
      UINT width, height, size, dibsz;
      // time to build the memory DIB to retrun for display
      //BITMAPINFOHEADER * pih
      BITMAPFILEHEADER * phdr = (BITMAPFILEHEADER *)pdib;
      LPBITMAPINFOHEADER pbi  = (LPBITMAPINFOHEADER)((BITMAPFILEHEADER *) phdr + 1);
      char * pbits            = (char *)((LPBITMAPINFOHEADER) pbi + 1);
      char * pdib2;
		width  = pbi->biWidth * 3;
      height = pbi->biHeight;
		if( pbi->biBitCount != 24 ) bark;
      if(width & 0x003)
         width = (width | 3) + 1;
      size  = height * width; // set IMAGE size into structure
      if( pbi->biSizeImage != size ) bark;
      if( pbits != (pdib + phdr->bfOffBits) ) bark;

      dibsz = sizeof(BITMAPINFOHEADER) + size;
      ghnd = DVGlobalAlloc( GHND, dibsz );
      if(ghnd)
         pdib2 = (LPSTR)DVGlobalLock(ghnd);
      if(pdib2) {
         LPBITMAPINFOHEADER pbi2  = (LPBITMAPINFOHEADER)pdib2;
         char * pbits2 = (char *)((LPBITMAPINFOHEADER)pbi2 + 1);
         memcpy(pbi2,pbi,sizeof(BITMAPINFOHEADER));
         memcpy(pbits2,pbits,size);
         DVGlobalUnlock(ghnd);
         hDIB = ghnd;   // pass back the memory DIB
      } else {
         if(ghnd)
            DVGlobalFree(ghnd);
         ghnd = 0;
         return 0;
      }

   }
   return hDIB;
}
#endif   // #ifdef   ADD_SGI_RGB_READ
// eof - DvRGB.c (was readrgb.c)

