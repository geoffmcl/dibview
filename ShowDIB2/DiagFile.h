// DiagFile.h
#ifndef	_DiagFile_HH
#define	_DiagFile_HH

#ifdef  __cplusplus
extern "C" {
#endif

// terminate a line
#define  GEOL     "\n"
#define  MEOR     "\r\n"

// sprtf() formats an output, and writes it to a diag file (fixing Cr/Lf pairs)
extern	int   _cdecl sprtf( LPTSTR lpf, ... );

#ifdef  __cplusplus
}
#endif

#endif	// _DiagFile_HH
// eof - DiagFile.h
