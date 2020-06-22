
// Rgb.h
#ifndef  _Rgb_HH
#define  _Rgb_HH

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef   ADDRGB2
extern   HANDLE   ReadRGBInfo( LPTSTR pszFile, HFILE * fh,
                     int * width, int * height, int * components );
#endif   // ADDRGB2

#ifdef  __cplusplus
//extern "C" {
}
#endif

#endif   // #ifndef  _Rgb_HH
// eof - rgb.c
