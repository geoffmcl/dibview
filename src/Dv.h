
// dv.h - all incusive include ...

#ifndef	_dv_h	// only include ONCE ... 
#define	_dv_h

// FIRST include is the VERSION.H,
// which also sets many FEATURES
#include "DvVers.h"	// ONLY contains defines!!!

// Define Dv16_App to ONLY link with OTHER libraries.
// ==================================================
#include "WinLibs.h"

#include "DvResS.h"	// MS Studeo maintained RESOURCE values
#include "DvRes.h"	// VERY common resource number (ONLY NUMBERS)

#include "DvDib.h"
#include "DvErrors.h"
#include "DvFile.h"
#include "DvView.h"
#include "DvOpts.h"
#include "DvChild.h"
#include "DvFrame.h"
#include "DvInit.h"
#include "DvPaint.h"
#include "DvPal.h"
#include "DvAbout.h"
#include "DvMem.h"
#include "DvWin32.h"
#include "DvRotate.h"	// Rotation of BITMAP
#include "DvEnum.h"
#include "DvData.h"		// Some DATA blocks
#include "DvFile2.h"
#include "DvFunc.h"
#include "DvWarn.h"
#include "DvDiag.h"
#include "DvClip.h"
#include "DvIni.h"      // expose some INI items
#include "WJpegLib.h"	// NOTE: This is an INDIRECT include!!!
#include "WJpgLib2.h"	// TWO new functions

#include	"DvInfo.h"	// Some IMAGE info - less than DIBINFO
						// AND small INFO one-line modeless DIALOG

#include	"DvWork.h"	// GLOBAL Work Structure
#include	"DvWait.h"	// set of 5-6 incrmental WAIT cursors
#include	"DvProgM.h"	// progress meter display 

#include "DvList.h"  // LINKED LIST - from ewmList.h
#include "DvUtil2.h" // some utility functions
#include "DvType.h"  // INT gmTypeOfFile(LPTSTR)
#include "DvTif.h"   // add TIFF6/geoTIFF/ TIFF/IT support
#include "DvGlaze.h" // glaze a window - GlazeWindow1(HDC,PRECT)
#include "DvMagnify2.h"  // allow magnification of a circle
#include "DvTrack.h" // mouse tracking code

// F20001126 - Appears LoadLibary() returns ZERO for ERROR, so change
#ifdef   WIN32
#define  MYHERROR    0  // per MSDN Oct 2000 documentation
#else // !WIN32
#define  MYHERROR    HINSTANCE_ERROR   // to be CHECKED in 16-bit
#endif   // WIN32 y/n

#endif	// _dv_h
// eof

