

// WWrBmp6.h

// Private version of data destination object

typedef struct {
  struct djpeg_dest_struct pub;	// public fields

  boolean is_os2;		// saves the OS2 format request flag

  jvirt_sarray_ptr whole_image;	// needed to reverse row order
  JDIMENSION data_width;	// JSAMPLEs per row
  JDIMENSION row_width;		// physical width of one row in the BMP file
  int pad_bytes;		// number of padding bytes needed per row
  JDIMENSION cur_output_row;	// next row# to write to virtual array
} bmp_dest_struct;

typedef bmp_dest_struct MLPTR bmp_dest_ptr;


// eof - WWrBmp6.h


