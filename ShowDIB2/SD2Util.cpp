
// SD2Util.cpp
#pragma warning(disable:4996)
#include <windows.h>
#include "showdib\showdib.h"

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : SplitFN
// Return type: void 
// Arguments  : LPTSTR pPath
//            : LPTSTR pFile
//            : LPTSTR pFullName
// Description: Split the pFullName into a PATH, including the final \, and
//              a clean FILE NAME ONLY
///////////////////////////////////////////////////////////////////////////////
VOID  SplitFN( LPTSTR pPath, LPTSTR pFile, LPTSTR pFullName )  // 23Aug2002 - add last chk.
{
   int      i, j, k;
   TCHAR    c;

   j = 0;
   if( pFullName )
      j = strlen(pFullName);
   if(j)
   {
      c = pFullName[ j - 1 ]; // get LAST character
      if( ( c == ':' ) || ( c == '\\' ) )
      {
         if(pPath) // if pPath given
            strcpy(pPath, pFullName);

         if(pFile)
            *pFile = 0;

         return;  // out of here
      }
      
      k = 0;
      for( i = 0; i < j; i++ )
      {
         c = pFullName[i];
         if( ( c == ':' ) || ( c == '\\' ) )
         {
            k = i;
         }
      }
      if(k)  // get LAST ':' or '\'
      {
         if( k < j )
            k++;
         if( pPath ) // if pPath given
         {
            strncpy(pPath,pFullName,k);
            pPath[k] = 0;
         }
         if( pFile ) // if pFile given
         {
            strcpy(pFile, &pFullName[k]);
         }
      }
      else
      {
         if( pFile ) // if pFile given
            strcpy(pFile, pFullName);  // then there is NO PATH
         if( pPath )
            *pPath = 0;
      }
   }
}

BOOL  Chk4Stg( LPTSTR lpd, LPTSTR lps )
{
   BOOL     bret = FALSE;
   LPTSTR ptmp = &gszTmpBuf[0];
   LPTSTR   p;
   DWORD  dwi;

   strcpy(ptmp, lpd);
   dwi = strlen(ptmp);
   if(dwi)
   {
      dwi--;
      if(ptmp[dwi] == '\\')
      {
         ptmp[dwi] = 0;
         p = strrchr(ptmp, '\\');
         if(p)
         {
            p++;
            //if( strcmpi(p, "DEBUG") == 0 )
            if( strcmpi(p, lps) == 0 )
            {
               *p = 0;
               strcpy(lpd,ptmp);    // use this
               bret = TRUE;
            }
         }
      }
   }
   return bret;
}

BOOL  Chk4Debug( LPTSTR lpd )
{
   return( Chk4Stg( lpd, "DEBUG" ) );
}
BOOL  Chk4Release( LPTSTR lpd )
{
   return( Chk4Stg( lpd, "RELEASE" ) );
}

DWORD  GetModulePath( LPTSTR lpb )
{
   LPTSTR   p;

   *lpb = 0;   // start with nothing

   GetModuleFileName( NULL, lpb, 256 );   // system sevice

   p = strrchr( lpb, '\\' );  // get LAST folder

   if( p )  // we got one
      p++;  // bump to next char
   else
      p = lpb; // CAN ONLY USE GIVEN

   *p = 0;

#ifdef  NDEBUG
   Chk4Release( lpb );
#else
   Chk4Debug( lpb );
#endif   // NDEBUG y/n

   return( strlen(lpb) );
}


// eof = SD2Util.cpp
