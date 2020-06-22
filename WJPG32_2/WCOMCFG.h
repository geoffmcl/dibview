
// WComCfg.h

//  A config structure
typedef	struct	tagJCOMCFG {
	long	jc_size;	// = sizeof( JCOMCFG )
	int		jc_desired_number_of_colors;	//  256;
	BOOL	jc_quantize_colors;	// = FALSE;
	int		jc_dct_method;	// = JDCT_IFAST;
	int		jc_dither_mode;	// JDITHER_FS;
	int		jc_fast;		// 0
	// Select recommended processing options for quick-and-dirty output
	BOOL	jc_two_pass_quantize;	// = TRUE;
	BOOL	jc_do_fancy_upsampling;	// = TRUE;
	BOOL	jc_do_block_smoothing;	// = TRUE;
	int		jc_scale_num;			// = 1;		/* 1:1 scaling */
	int		jc_scale_denom;			// = 1;
}JCOMCFG;

typedef JCOMCFG MLPTR LPJCOMCFG;

#define		DEF_J2_CFG	\
	sizeof( JCOMCFG ), \
	(int)256, \
	FALSE,	\
	JDCT_ISLOW, \
	JDITHER_FS, \
	(int)0, \
	TRUE, \
	TRUE, \
	TRUE, \
	(int)1, \
	(int)1


//EXTERN(void) jpeg_stdio_src JPP((j_decompress_ptr cinfo,
//								void MLPTR infile));
// Library declarations
// EXTERN(int)	CopyConfig6 JPP(( j_decompress_ptr cinfo ));
//EXTERN(int)	GetConfig6 JPP(( LPJCOMCFG cinfo ));
//EXTERN(int)	SetConfig6 JPP(( LPJCOMCFG ci ));
EXPORT32 int MLIBCALL WGETCONFIG6( LPJCOMCFG ci );
EXPORT32 int MLIBCALL WSETCONFIG6( LPJCOMCFG ci );

typedef int  (MLIBCONV *LPGETCONFIG6) ( LPJCOMCFG );
typedef int  (MLIBCONV *LPSETCONFIG6) ( LPJCOMCFG );

// eof - WComCfg.h
