

// DvKey.c

#include	"dv.h"

//extern	char	gszDiag[];

typedef	struct	{
	int		vk_Val;
	LPSTR	   vk_Stg;
} VKSTR;
typedef VKSTR FAR * LPVKSTR;

///*
// * Virtual Keys, Standard Set
// */
//#define VK_LBUTTON        0x01
//#define VK_RBUTTON        0x02
//#define VK_CANCEL         0x03
//#define VK_MBUTTON        0x04    /* NOT contiguous with L &
//RBUTTON */
//#define VK_BACK           0x08
//#define VK_TAB            0x09
//#define VK_CLEAR          0x0C
//#define VK_RETURN         0x0D
//#define VK_SHIFT          0x10
//#define VK_CONTROL        0x11
//#define VK_MENU           0x12
//#define VK_PAUSE          0x13
//#define VK_CAPITAL        0x14
//#define VK_ESCAPE         0x1B
//#define VK_SPACE          0x20
//#define VK_PRIOR          0x21
//#define VK_NEXT           0x22
//#define VK_END            0x23
//#define VK_HOME           0x24
//#define VK_LEFT           0x25
//#define VK_UP             0x26
//#define VK_RIGHT          0x27
//#define VK_DOWN           0x28
//#define VK_SELECT         0x29
//#define VK_PRINT          0x2A
//#define VK_EXECUTE        0x2B
//#define VK_SNAPSHOT       0x2C
//#define VK_INSERT         0x2D
//#define VK_DELETE         0x2E
//#define VK_HELP           0x2F
///* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 -
//0x39) */
///* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 -
//0x5A) */
//#define VK_LWIN           0x5B
//#define VK_RWIN           0x5C
//#define VK_APPS           0x5D
//#define VK_NUMPAD0        0x60
//#define VK_NUMPAD1        0x61
//#define VK_NUMPAD2        0x62
//#define VK_NUMPAD3        0x63
//#define VK_NUMPAD4        0x64
//#define VK_NUMPAD5        0x65
//#define VK_NUMPAD6        0x66
//#define VK_NUMPAD7        0x67
//#define VK_NUMPAD8        0x68
//#define VK_NUMPAD9        0x69
//#define VK_MULTIPLY       0x6A
//#define VK_ADD            0x6B
//#define VK_SEPARATOR      0x6C
//#define VK_SUBTRACT       0x6D
//#define VK_DECIMAL        0x6E
//#define VK_DIVIDE         0x6F
//#define VK_F1             0x70
//#define VK_F2             0x71
//#define VK_F3             0x72
//#define VK_F4             0x73
//#define VK_F5             0x74
//#define VK_F6             0x75
//#define VK_F7             0x76
//#define VK_F8             0x77
//#define VK_F9             0x78
//#define VK_F10            0x79
//#define VK_F11            0x7A
//#define VK_F12            0x7B
//#define VK_F13            0x7C
//#define VK_F14            0x7D
//#define VK_F15            0x7E
//#define VK_F16            0x7F
//#define VK_F17            0x80
//#define VK_F18            0x81
//#define VK_F19            0x82
//#define VK_F20            0x83
//#define VK_F21            0x84
//#define VK_F22            0x85
//#define VK_F23            0x86
//#define VK_F24            0x87
//#define VK_NUMLOCK        0x90
//#define VK_SCROLL         0x91
///*
// * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual
//keys.
// * Used only as parameters to GetAsyncKeyState() and
//GetKeyState().
// * No other API or message will distinguish left and right keys
//in this way.
// */
//#define VK_LSHIFT         0xA0
//#define VK_RSHIFT         0xA1
//#define VK_LCONTROL       0xA2
//#define VK_RCONTROL       0xA3
//#define VK_LMENU          0xA4
//#define VK_RMENU          0xA5
//#if(WINVER >= 0x0400)
//#define VK_PROCESSKEY     0xE5
//#endif /* WINVER >= 0x0400 */
//#define VK_ATTN           0xF6
//#define VK_CRSEL          0xF7
//#define VK_EXSEL          0xF8
//#define VK_EREOF          0xF9
//#define VK_PLAY           0xFA
//#define VK_ZOOM           0xFB
//#define VK_NONAME         0xFC
//#define VK_PA1            0xFD
//#define VK_OEM_CLEAR      0xFE

VKSTR	sVkStr[] = {
//
// * Virtual Keys, Standard Set
//
	{ VK_LBUTTON        ,"VK_LBUTTON" },	// 0x01
	{ VK_RBUTTON        ,"VK_RBUTTON" },	// 0x02
	{ VK_CANCEL         ,"VK_CANCEL" },	// 0x03
	{ VK_MBUTTON        ,"VK_MBUTTON" },	// 0x04
	{ VK_BACK           ,"VK_BACK" },	// 0x08
	{ VK_TAB            ,"VK_TAB" },	// 0x09
	{ VK_CLEAR          ,"VK_CLEAR" },	// 0x0C
	{ VK_RETURN         ,"VK_RETURN" },	// 0x0D
	{ VK_SHIFT          ,"VK_SHIFT" },	// 0x10
	{ VK_CONTROL        ,"VK_CONTROL" },	// 0x11
	{ VK_MENU           ,"VK_MENU" },	// 0x12
	{ VK_PAUSE          ,"VK_PAUSE" },	// 0x13
	{ VK_CAPITAL        ,"VK_CAPITAL" },	// 0x14
	{ VK_ESCAPE         ,"VK_ESCAPE" },	// 0x1B
	{ VK_SPACE          ,"VK_SPACE" },	// 0x20
	{ VK_PRIOR          ,"VK_PRIOR" },	// 0x21
	{ VK_NEXT           ,"VK_NEXT" },	// 0x22
	{ VK_END            ,"VK_END" },	// 0x23
	{ VK_HOME           ,"VK_HOME" },	// 0x24
	{ VK_LEFT           ,"VK_LEFT" },	// 0x25
	{ VK_UP             ,"VK_UP" },	// 0x26
	{ VK_RIGHT          ,"VK_RIGHT" },	// 0x27
	{ VK_DOWN           ,"VK_DOWN" },	// 0x28
	{ VK_SELECT         ,"VK_SELECT" },	// 0x29
	{ VK_PRINT          ,"VK_PRINT" },	// 0x2A
	{ VK_EXECUTE        ,"VK_EXECUTE" },	// 0x2B
	{ VK_SNAPSHOT       ,"VK_SNAPSHOT" },	// 0x2C
	{ VK_INSERT         ,"VK_INSERT" },	// 0x2D
	{ VK_DELETE         ,"VK_DELETE" },	// 0x2E
	{ VK_HELP           ,"VK_HELP" },	// 0x2F
	{ VK_LWIN           ,"VK_LWIN" },	// 0x5B
	{ VK_RWIN           ,"VK_RWIN" },	// 0x5C
	{ VK_APPS           ,"VK_APPS" },	// 0x5D
	{ VK_NUMPAD0        ,"VK_NUMPAD0" },	// 0x60
	{ VK_NUMPAD1        ,"VK_NUMPAD1" },	// 0x61
	{ VK_NUMPAD2        ,"VK_NUMPAD2" },	// 0x62
	{ VK_NUMPAD3        ,"VK_NUMPAD3" },	// 0x63
	{ VK_NUMPAD4        ,"VK_NUMPAD4" },	// 0x64
	{ VK_NUMPAD5        ,"VK_NUMPAD5" },	// 0x65
	{ VK_NUMPAD6        ,"VK_NUMPAD6" },	// 0x66
	{ VK_NUMPAD7        ,"VK_NUMPAD7" },	// 0x67
	{ VK_NUMPAD8        ,"VK_NUMPAD8" },	// 0x68
	{ VK_NUMPAD9        ,"VK_NUMPAD9" },	// 0x69
	{ VK_MULTIPLY       ,"VK_MULTIPLY" },	// 0x6A
	{ VK_ADD            ,"VK_ADD" },	// 0x6B
	{ VK_SEPARATOR      ,"VK_SEPARATOR" },	// 0x6C
	{ VK_SUBTRACT       ,"VK_SUBTRACT" },	// 0x6D
	{ VK_DECIMAL        ,"VK_DECIMAL" },	// 0x6E
	{ VK_DIVIDE         ,"VK_DIVIDE" },	// 0x6F
	{ VK_F1             ,"VK_F1" },	// 0x70
	{ VK_F2             ,"VK_F2" },	// 0x71
	{ VK_F3             ,"VK_F3" },	// 0x72
	{ VK_F4             ,"VK_F4" },	// 0x73
	{ VK_F5             ,"VK_F5" },	// 0x74
	{ VK_F6             ,"VK_F6" },	// 0x75
	{ VK_F7             ,"VK_F7" },	// 0x76
	{ VK_F8             ,"VK_F8" },	// 0x77
	{ VK_F9             ,"VK_F9" },	// 0x78
	{ VK_F10            ,"VK_F10" },	// 0x79
	{ VK_F11            ,"VK_F11" },	// 0x7A
	{ VK_F12            ,"VK_F12" },	// 0x7B
	{ VK_F13            ,"VK_F13" },	// 0x7C
	{ VK_F14            ,"VK_F14" },	// 0x7D
	{ VK_F15            ,"VK_F15" },	// 0x7E
	{ VK_F16            ,"VK_F16" },	// 0x7F
	{ VK_F17            ,"VK_F17" },	// 0x80
	{ VK_F18            ,"VK_F18" },	// 0x81
	{ VK_F19            ,"VK_F19" },	// 0x82
	{ VK_F20            ,"VK_F20" },	// 0x83
	{ VK_F21            ,"VK_F21" },	// 0x84
	{ VK_F22            ,"VK_F22" },	// 0x85
	{ VK_F23            ,"VK_F23" },	// 0x86
	{ VK_F24            ,"VK_F24" },	// 0x87
	{ VK_NUMLOCK        ,"VK_NUMLOCK" },	// 0x90
	{ VK_SCROLL         ,"VK_SCROLL" },	// 0x91
	{ VK_LSHIFT         ,"VK_LSHIFT" },	// 0xA0
	{ VK_RSHIFT         ,"VK_RSHIFT" },	// 0xA1
	{ VK_LCONTROL       ,"VK_LCONTROL" },	// 0xA2
	{ VK_RCONTROL       ,"VK_RCONTROL" },	// 0xA3
	{ VK_LMENU          ,"VK_LMENU" },	// 0xA4
	{ VK_RMENU          ,"VK_RMENU" },	// 0xA5
//#if(WINVER >= ," },	// 0xER >= ," },    0400)
	{ VK_PROCESSKEY     ,"VK_PROCESSKEY" },	// 0xE5
//#endif /* WINVER >= ," },    * WINVER >= ,   // 0x0400 */
	{ VK_ATTN           ,"VK_ATTN" },	// 0xF6
	{ VK_CRSEL          ,"VK_CRSEL" },	// 0xF7
	{ VK_EXSEL          ,"VK_EXSEL" },	// 0xF8
	{ VK_EREOF          ,"VK_EREOF" },	// 0xF9
	{ VK_PLAY           ,"VK_PLAY" },	// 0xFA
	{ VK_ZOOM           ,"VK_ZOOM" },	// 0xFB
	{ VK_NONAME         ,"VK_NONAME" },	// 0xFC
	{ VK_PA1            ,"VK_PA1" },	// 0xFD
	{ VK_OEM_CLEAR      ,"VK_OEM_CLEAR" },	// 0xFE
	{ 0,                 0              }
};

char	szVkascii[16];

// eof - DvKey.c
///* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 -
//0x39) */
///* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 -
//0x5A) */
LPSTR	GetVKStg( int nKey )
{
	LPSTR	lps;
	LPVKSTR	lpv;
	int		iv;
	char	chr[2];

	if( ( ( nKey >= '0' ) && ( nKey <= '9' ) ) ||
		( ( nKey >= 'A' ) && ( nKey <= 'Z' ) ) )
	{
		lps = &szVkascii[0];
		lstrcpy( lps, "VK_" );
		chr[0] = (char)nKey;
		chr[1] = 0;
		lstrcat( lps, &chr[0] );
	}
	else
	{
		lpv = &sVkStr[0];
		while( ( iv = lpv->vk_Val ) &&
			( lps = lpv->vk_Stg ) )
		{
			if( iv == nKey )
				break;
			lpv++;
			lps = 0;
		}
	}
	if( lps == 0 )
		lps = "Unknown";
	return lps;
}


void	ShowVKey( int nKey, SHORT pState, SHORT cState )
{
	LPSTR	lps;
	lps = &gszDiag[0];

	wsprintf( lps,
		"Had VKey [%s] Value=0x%x"MEOR,
		GetVKStg( nKey ),
		(nKey & 0xff ) );

	DO(lps);
}



// eof - DvKey.c
