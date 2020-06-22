/* ****************************************************************
 * DvTrans.c
 * General utilities
 ****************************************************************** */

#include <windows.h>
#include "dv.h"
#include "DvTrans.h"

//Here are the six general steps required to 
// create a transparent bitmap (in VB):
//1. Store the area, or background, where the bitmap is going to be drawn.
//2. Create a monochrome mask of the bitmap that identifies the transparent areas
//of the bitmap by using a white pixel to indicate transparent areas and a black
//pixel to indicate non-transparent areas of the bitmap.
//3. Combine the pixels of the monochrome mask with the background bitmap
//   using the And binary operator. The area of the background where the
//   non-transparent portion of the bitmap will appear is made black.
//4. Combine an inverted copy of the monochrome mask (step 2) with the source
//bitmap using the And binary operator. The transparent areas of the source
//bitmap will be made black.
//5. Combine the modified background (step 3) with the modified source bitmap
//(step 4) using the Xor binary operator. The background will show through the
//transparent portions of the bitmap.
//6. Copy the resulting bitmap to the background
//The follow is a sub-routine in VB that effects the above �transparency�
//======================
//Sub TransparentBlt (dest As Control,
//	ByVal srcBmp As Integer,
//	ByVal destX As Integer, ByVal destY As Integer,
//  ByVal TransColor As Long)
BOOL	dvTransparentBlt( HDC hDC, HBITMAP srcBmp,
					   int destX, int destY,
					   COLORREF	dwTransColor )
{
	BOOL	rFlg = FALSE;
// Const PIXEL = 3
//      Dim destScale As Integer
//      Dim srcDC As Integer  'source bitmap (color)
// Dim saveDC As Integer 'backup copy of source bitmap
//      Dim maskDC As Integer 'mask bitmap (monochrome)
// Dim invDC As Integer  'inverse of mask bitmap (monochrome)
// Dim resultDC As Integer 'combination of source bitmap & background
//      Dim bmp As bitmap 'description of the source bitmap
//      Dim hResultBmp As Integer 'Bitmap combination of source & background
//      Dim hSaveBmp As Integer 'Bitmap stores backup copy of source bitmap
//      Dim hMaskBmp As Integer 'Bitmap stores mask (monochrome)
// Dim hInvBmp As Integer  'Bitmap holds inverse of mask (monochrome)
// Dim hPrevBmp As Integer 'Bitmap holds previous bitmap selected in DC
//      Dim hSrcPrevBmp As Integer  'Holds previous bitmap in source DC
//      Dim hSavePrevBmp As Integer 'Holds previous bitmap in saved DC
//      Dim hDestPrevBmp As Integer 'Holds previous bitmap in destination DC
//      Dim hMaskPrevBmp As Integer 'Holds previous bitmap in the mask DC
//      Dim hInvPrevBmp As Integer 'Holds previous bitmap in inverted mask DC
//      Dim OrigColor As Long 'Holds original background color from source DC
//      Dim Success As Integer 'Stores result of call to Windows API
//      If TypeOf dest Is PictureBox Then 'Ensure objects are picture boxes
//        destScale = dest.ScaleMode 'Store ScaleMode to restore later
//        dest.ScaleMode = PIXEL 'Set ScaleMode to pixels for Windows GDI
//        'Retrieve bitmap to get width (bmp.bmWidth) & height (bmp.bmHeight)
//typedef struct tagBITMAP {  // bm  
//   LONG   bmType; 
//   LONG   bmWidth; 
//   LONG   bmHeight; 
//   LONG   bmWidthBytes; 
//   WORD   bmPlanes; 
//   WORD   bmBitsPixel; 
//   LPVOID bmBits; 
//} BITMAP; 
	int		i;
	BITMAP	bmp;
	HDC	srcDC, saveDC, maskDC, invDC, resultDC;
	HBITMAP	hMaskBmp, hInvBmp, hResultBmp,hPrevBmp, hSaveBmp;
	HGDIOBJ	hSrcPrevBmp, hSavePrevBmp;
	HGDIOBJ	hMaskPrevBmp, hInvPrevBmp, hDestPrevBmp;
	COLORREF	OrigColor, dwOldTransColor;

	// Clear some tool handles
	srcDC = 0;
	saveDC = 0;
	maskDC = 0;
	invDC = 0;
	resultDC = 0;
	hMaskBmp = 0;
	hInvBmp = 0;
	hResultBmp = 0;
	hSaveBmp = 0;

	// Success = GetObj(srcBmp, Len(bmp), bmp)
	i = GetObject( srcBmp,	// handle to graphics object of interest
		sizeof( BITMAP ),	// size of buffer for object information 
		&bmp );				// pointer to buffer for object information
	if( i )
	{
		int	iSuccess;

		// Create NEEDED Objects
		srcDC = CreateCompatibleDC(hDC);	//'Create DC to hold stage
		saveDC = CreateCompatibleDC(hDC);	//'Create DC to hold stage
		maskDC = CreateCompatibleDC(hDC);	//'Create DC to hold stage
		invDC = CreateCompatibleDC(hDC);	//'Create DC to hold stage
		resultDC = CreateCompatibleDC(hDC);	//'Create DC to hold stage

//        'Create monochrome bitmaps for the mask-related bitmaps:
		hMaskBmp = CreateBitmap( bmp.bmWidth, bmp.bmHeight, 1, 1, 0);
		hInvBmp = CreateBitmap(bmp.bmWidth, bmp.bmHeight, 1, 1, 0);
//        'Create color bitmaps for final result & stored copy of source
		hResultBmp = CreateCompatibleBitmap(hDC, bmp.bmWidth,
			bmp.bmHeight );
		hSaveBmp = CreateCompatibleBitmap(hDC, bmp.bmWidth,
			bmp.bmHeight );

		if( srcDC && saveDC && maskDC && invDC && resultDC &&
			hMaskBmp && hInvBmp && hResultBmp &&
			hSaveBmp )
		{
			hSrcPrevBmp = SelectObject( srcDC, srcBmp );	//'Select bitmap in DC
			hSavePrevBmp = SelectObject(saveDC, hSaveBmp);	//'Select bitmap in DC
			hMaskPrevBmp = SelectObject(maskDC, hMaskBmp);	//'Select bitmap in DC
			hInvPrevBmp = SelectObject(invDC, hInvBmp);	//'Select bitmap in DC
			hDestPrevBmp = SelectObject(resultDC, hResultBmp); //'Select bitmap

			iSuccess = BitBlt(saveDC, 0, 0, bmp.bmWidth, bmp.bmHeight, srcDC,
				0, 0, SRCCOPY);	//'Make backup of source bitmap to restore later

//        'Create mask: set background color of source to transparent color.
			OrigColor = SetBkColor(srcDC, dwTransColor);
			iSuccess = BitBlt(maskDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				srcDC,
				0, 0, SRCCOPY);
			dwOldTransColor = SetBkColor(srcDC, OrigColor);
// 'Create inverse of mask to AND w/ source & combine w/ background.
			iSuccess = BitBlt(invDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				maskDC,
				0, 0, NOTSRCCOPY);
// 'Copy background bitmap to result & create final transparent bitmap
			iSuccess = BitBlt(resultDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				hDC, destX, destY, SRCCOPY);
// 'AND mask bitmap w/ result DC to punch hole in the background by
//        'painting black area for non-transparent portion of source bitmap.
			iSuccess = BitBlt(resultDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				maskDC, 0, 0, SRCAND);
// 'AND inverse mask w/ source bitmap to turn off bits associated
//        'with transparent area of source bitmap by making it black.
			iSuccess = BitBlt(srcDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				invDC,
				0, 0, SRCAND);
// 'XOR result w/ source bitmap to make background show through.
			iSuccess = BitBlt(resultDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				srcDC, 0, 0, SRCPAINT);

			// OUPUT Result to HDC
			iSuccess = BitBlt( hDC, destX, destY,
				bmp.bmWidth, bmp.bmHeight,
				resultDC, 0, 0, SRCCOPY);	//'Display transparent bitmap on backgrnd

			iSuccess = BitBlt(srcDC, 0, 0, bmp.bmWidth, bmp.bmHeight,
				saveDC,
				0, 0, SRCCOPY);	//'Restore backup of bitmap.

			hPrevBmp = SelectObject(srcDC, hSrcPrevBmp);	//'Select orig object
			hPrevBmp = SelectObject(saveDC, hSavePrevBmp); //'Select orig object
			hPrevBmp = SelectObject(resultDC, hDestPrevBmp); //'Select orig object
			hPrevBmp = SelectObject(maskDC, hMaskPrevBmp); //'Select orig object
			hPrevBmp = SelectObject(invDC, hInvPrevBmp); //'Select orig object
// dest.ScaleMode = destScale 'Restore ScaleMode of destination.
			rFlg = TRUE;
// End If
		}

		if( hSaveBmp )
			DeleteObject(hSaveBmp);	//'Deallocate system resources.
		if( hMaskBmp )
			DeleteObject(hMaskBmp);	//'Deallocate system resources.
		if( hInvBmp )
			DeleteObject(hInvBmp);	//'Deallocate system resources.
		if( hResultBmp )
			DeleteObject(hResultBmp); //'Deallocate system resources.
		if( srcDC )
			DeleteDC(srcDC);	//'Deallocate system resources.
		if( saveDC )
			DeleteDC(saveDC);	//'Deallocate system resources.
		if( invDC )
			DeleteDC(invDC);	//'Deallocate system resources.
		if( maskDC )
			DeleteDC(maskDC);	//'Deallocate system resources.
		if( resultDC )
			DeleteDC(resultDC);	//'Deallocate system resources.

	}	// If we got the srcBmp INFORMATION
	return rFlg;	// TRUE = Successful PAINT done.
//   End Sub
////======================
}	// dvTransparentBlt( ... )

// DvTrans.c
