
/*
 * wdmaster.c
 *
 * Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains the main control for the JPEG decompressor.
 * The system-dependent (user interface) code should call jpeg_decompress()
 * after doing appropriate setup of the decompress_info_struct parameter.
 */

#include "winclude.h"


METHODDEF void
d_per_scan_method_selection (decompress_info_ptr cinfo)
/* Central point for per-scan method selection */
{
  /* MCU disassembly */
  jseldmcu(cinfo);
  /* Upsampling of pixels */
  jselupsample(cinfo);
}


LOCAL void
d_initial_method_selection (decompress_info_ptr cinfo)
/* Central point for initial method selection (after reading file header) */
{
  /* JPEG file scanning method selection is already done. */
  /* So is output file format selection (both are done by user interface). */

  /* Gamma and color space conversion */
  /* NB: this may change the component_needed flags */
  jseldcolor(cinfo);

  /* Color quantization selection rules */
#ifdef QUANT_1PASS_SUPPORTED
#ifdef QUANT_2PASS_SUPPORTED
  /* We have both, check for conditions in which 1-pass should be used */
  if (cinfo->num_components != 3 || cinfo->jpeg_color_space != CS_YCbCr)
    cinfo->two_pass_quantize = FALSE; /* 2-pass only handles YCbCr input */
  if (cinfo->out_color_space == CS_GRAYSCALE)
    cinfo->two_pass_quantize = FALSE; /* Should use 1-pass for grayscale out */
#else /* not QUANT_2PASS_SUPPORTED */
  cinfo->two_pass_quantize = FALSE; /* only have 1-pass */
#endif
#else /* not QUANT_1PASS_SUPPORTED */
#ifdef QUANT_2PASS_SUPPORTED
  cinfo->two_pass_quantize = TRUE; /* only have 2-pass */
#else /* not QUANT_2PASS_SUPPORTED */
  if (cinfo->quantize_colors) {
    ERREXIT(cinfo->emethods, "Color quantization was not compiled");
  }
#endif
#endif

#ifdef QUANT_1PASS_SUPPORTED
  jsel1quantize(cinfo);
#endif
#ifdef QUANT_2PASS_SUPPORTED
  jsel2quantize(cinfo);
#endif

  /* Cross-block smoothing */
#ifdef BLOCK_SMOOTHING_SUPPORTED
  jselbsmooth(cinfo);
#else
  cinfo->do_block_smoothing = FALSE;
#endif

  /* Entropy decoding: either Huffman or arithmetic coding. */
#ifdef D_ARITH_CODING_SUPPORTED
  jseldarithmetic(cinfo);
#else
	if (cinfo->arith_code) 
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_ARITH );	/* "Arithmetic coding not supported" */
		return;
#else
		ERREXIT(cinfo->emethods, "Arithmetic coding not supported");
#endif
	}
#endif
  jseldhuffman(cinfo);

  /* Pipeline control */
  jseldpipeline(cinfo);
  /* Overall control (that's me!) */
  cinfo->methods->d_per_scan_method_selection = d_per_scan_method_selection;
}


// Do computations that are needed before
// initial method selection
LOCAL void
initial_setup (decompress_info_ptr cinfo)
{
	short ci;
	jpeg_component_info MLPTR compptr;

	// Compute maximum sampling factors; check factor validity
	cinfo->max_h_samp_factor = 1;
	cinfo->max_v_samp_factor = 1;
	for( ci = 0; ci < cinfo->num_components; ci++ )
	{
		compptr = &cinfo->comp_info[ci];
		if( ( compptr->h_samp_factor <= 0 ) ||
			( compptr->h_samp_factor >  MAX_SAMP_FACTOR ) ||
			( compptr->v_samp_factor <= 0 ) ||
			( compptr->v_samp_factor > MAX_SAMP_FACTOR ) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_JPGSAMP );
			return;
#else
			ERREXIT(cinfo->emethods, "Bogus sampling factors");
#endif
		}
		cinfo->max_h_samp_factor = MAX( cinfo->max_h_samp_factor,
			compptr->h_samp_factor);
		cinfo->max_v_samp_factor = MAX( cinfo->max_v_samp_factor,
			compptr->v_samp_factor);
	}

	// Compute logical downsampled dimensions of components
	for( ci = 0; ci < cinfo->num_components; ci++ )
	{
		compptr = &cinfo->comp_info[ci];
		compptr->true_comp_width = ( cinfo->image_width *
			compptr->h_samp_factor +
			cinfo->max_h_samp_factor - 1 ) /
			cinfo->max_h_samp_factor;
		compptr->true_comp_height = ( cinfo->image_height *
			compptr->v_samp_factor +
			cinfo->max_v_samp_factor - 1 ) /
			cinfo->max_v_samp_factor;
	}
}


//
// This is the main entry point to the JPEG decompressor.
//
GLOBAL void
jpeg_decompress (decompress_info_ptr cinfo)
{
	// Init pass counts to 0 --- 
	// total_passes is adjusted in method selection
	cinfo->total_passes = 0;
	cinfo->completed_passes = 0;

	// Read the JPEG file header markers;
	// everything up through the first SOS
	// marker is read now.
	// NOTE: the user interface must have initialized the
	// read_file_header method pointer
	// (eg, by calling jselrjfif or jselrtiff).
	// The other file reading methods
	// (read_scan_header etc.) were probably
	// set at the same time, but could be set up
	// by read_file_header itself.
	(*cinfo->methods->read_file_header) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	/// INC_WINDOWS

	if( !( (*cinfo->methods->read_scan_header) (cinfo) ) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_JPGNUL );	// "Empty JPEG file"
		if( bErrFlag )
			goto DecompFree;
#else
		ERREXIT(cinfo->emethods, "Empty JPEG file");
#endif
	}

	// Give UI a chance to adjust decompression
	// parameters and select 
	// output file format based on info from file header.

	(*cinfo->methods->d_ui_method_selection) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	// Now select methods for decompression steps.
	initial_setup(cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	d_initial_method_selection(cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	// Initialize the output file & other modules as needed
	// (modules needing per-scan init are called by
	// pipeline controller)

	(*cinfo->methods->output_init) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	(*cinfo->methods->colorout_init) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	if( cinfo->quantize_colors )
		(*cinfo->methods->color_quant_init) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	// And let the pipeline controller do the rest.
	(*cinfo->methods->d_pipeline_controller) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	// Finish output file, release working storage, etc
	if( cinfo->quantize_colors )
		(*cinfo->methods->color_quant_term) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	(*cinfo->methods->colorout_term) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	(*cinfo->methods->output_term) (cinfo);

#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto DecompFree;
#endif	// INC_WINDOWS

	(*cinfo->methods->read_file_trailer) (cinfo);

#ifdef	_INC_WINDOWS
DecompFree:
#endif	// INC_WINDOWS

	(*cinfo->emethods->free_all) ();

	// My, that was easy, wasn't it?
}

#ifdef	_INC_WINDOWS
//
// This is the main entry point to the JPEG decomp SIZE.
//

GLOBAL void
jpeg_decompsize( decompress_info_ptr cinfo )
{
	// Init pass counts to 0
	// total_passes is adjusted in method selection
	cinfo->total_passes = 0;
	cinfo->completed_passes = 0;

	// Read the JPEG file header markers; everything up
	// through the first SOS marker is read now.
	// NOTE: the user interface must have initialized the
	// read_file_header method pointer (eg, by calling
	// jselrjfif or jselrtiff).
	// The other file reading methods (read_scan_header
	// etc.) were probably set at the same time, but
	// could be set up by read_file_header itself.

	(*cinfo->methods->read_file_header) (cinfo);

	if( bErrFlag )
	{
		goto DeSizeFree;
	}

	if( !((*cinfo->methods->read_scan_header) (cinfo)) )
	{
		ERRFLAG( BAD_JPGNUL );	// "Empty JPEG file"
		if( bErrFlag )
			goto DeSizeFree;
	}

	// Give UI a chance to adjust decompression
	// parameters and select output file format based
	// on info from file header.

	//	(*cinfo->methods->d_ui_method_selection) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;

	// Now select methods for decompression steps.
	//  initial_setup(cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  d_initial_method_selection(cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;

	// Initialize the output file & other modules as needed
	// (modules needing per-scan init are
	// called by pipeline controller)

	//  (*cinfo->methods->output_init) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  (*cinfo->methods->colorout_init) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  if (cinfo->quantize_colors)
	//    (*cinfo->methods->color_quant_init) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;

	// And let the pipeline controller do the rest.
	//  (*cinfo->methods->d_pipeline_controller) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;

	// Finish output file, release working storage, etc
	//  if (cinfo->quantize_colors)
	//    (*cinfo->methods->color_quant_term) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  (*cinfo->methods->colorout_term) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  (*cinfo->methods->output_term) (cinfo);
	//	if( bErrFlag )
	//		goto DeSizeFree;
	//  (*cinfo->methods->read_file_trailer) (cinfo);

DeSizeFree:
	(*cinfo->emethods->free_all) ();
	// My, that was easier, wasn't it?

}

#endif	// _INC_WINDOWS

// eof - wdmaster.c


