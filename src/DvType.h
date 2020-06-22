
// DvType.h
#ifndef  _DvType_h
#define  _DvType_h

// Internal Ouput/Input FILE TYPES
// ===============================
#define	ft_Undef    0
#define	ft_BMP      1
#define	ft_PPM      3	// Reduced OS/2 support!!!
#define	ft_RLE      4	// NOT YET SUPPORTED
#define	ft_TARGA    5	// NOT YET SUPPORTED
#define ft_TIF      7  // TIFF 6, TIFF/IT, GeoTIFF
//	VER_BUILD29	// fixes to readrgb - FIX20030724
#define  ft_RGB     8  // RGB (SGI texture files)
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define	ft_GIF      2
#define	ft_JPG      6
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

#define  ft_BadDIB      -5
#define  ft_NoRead      -4
#define  ft_NULL        -3
#define  ft_NoSeek      -2
#define  ft_NoOpen      -1

typedef struct tagBITMAPINFO256 { 
   BITMAPINFOHEADER bmiHeader; 
   RGBQUAD          bmiColors[256];
} BITMAPINFO256, * PBITMAPINFO256; 

extern   INT   gmTypeOfFile( LPTSTR lpf );


#endif   // _DvType_h
// eof = DvType.h
