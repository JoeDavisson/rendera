#include "rendera.h"
#include <jpeglib.h>
#include <setjmp.h>

static inline void write_uint8(uint8_t num, FILE *out)
{
  fputc(num, out);
}

static inline void write_uint16(uint16_t num, FILE *out)
{
  #if BYTE_ORDER == BIG_ENDIAN
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
  #else
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  #endif
}

static inline void write_uint32(uint32_t num, FILE *out)
{
  #if BYTE_ORDER == BIG_ENDIAN
  fputc((num >> 24) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
  #else
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  fputc((num >> 16) & 0xff, out);
  fputc((num >> 24) & 0xff, out);
  #endif
}

static int file_exists(const char *s)
{
  FILE *temp = fopen(s, "r");

  if(temp)
  {
    fclose(temp);
    return 1;
  }

  return 0;
}

void save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Image");
  fc->filter("PNG Image\t*.png\nJPEG Image\t*.jpg\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc->show();

  const char *fn = fc->filename();

  // get file extension
  char ext[16];
  char *p = (char *)fn + strlen(fn) - 1;
  while(p >= fn)
  {
    if(*p == '.')
    {
      strcpy(ext, p);
      break;
    }

    p--;
  }

  if(file_exists(fn))
  {
    fl_message_title("Replace File?");
    if(fl_choice("Replace File?", "No", "Yes", NULL) == 0)
      return;
  }
  
  if(strcasecmp(ext, ".png") == 0)
    save_png(fn);
  else if(strcasecmp(ext, ".jpg") == 0)
    save_jpg(fn);
  else if(strcasecmp(ext, ".bmp") == 0)
    save_bmp(fn);
  else if(strcasecmp(ext, ".tga") == 0)
    save_tga(fn);
  else
  {
    delete fc;
    return;
  }

  delete fc;
}

void save_bmp(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  Bitmap *bmp = Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  int w = bmp->w - overscroll * 2;
  int h = bmp->h - overscroll * 2;
  int pad = w % 4;

  // BITMAPFILEHEADER
  write_uint8('B', out);
  write_uint8('M', out);
  write_uint32(14 + 40 + ((w + pad) * h) * 3, out);
  write_uint16(0, out);
  write_uint16(0, out);
  write_uint32(14 + 40, out);

  // BITMAPINFOHEADER
  write_uint32(40, out);
  write_uint32(w, out);
  write_uint32(-h, out);
  write_uint16(1, out);
  write_uint16(24, out);
  write_uint32(0, out);
  write_uint32(0, out);
  write_uint32(300 * 39.370079 + .5, out);
  write_uint32(300 * 39.370079 + .5, out);
  write_uint32(0, out);
  write_uint32(0, out);

  int *p = bmp->row[overscroll] + overscroll;
  unsigned char *linebuf = new unsigned char[w * 3 + pad];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int xx = 0;
    for(x = 0; x < w; x++)
    {
      linebuf[xx + 0] = (*p >> 16) & 0xff;
      linebuf[xx + 1] = (*p >> 8) & 0xff;
      linebuf[xx + 2] = *p & 0xff;
      p++;
      xx += 3;
    }
    for(x = 0; x < pad; x++)
      linebuf[xx++] = 0;
    p += overscroll * 2;

    fwrite(linebuf, 1, w * 3, out);
  }

  delete[] linebuf;
  fclose(out);
}

void save_tga(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  Bitmap *bmp = Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  int w = bmp->w - overscroll * 2;
  int h = bmp->h - overscroll * 2;

  write_uint8(0, out);
  write_uint8(0, out);
  write_uint8(2, out);
  write_uint16(0, out);
  write_uint16(0, out);
  write_uint8(0, out);
  write_uint16(0, out);
  write_uint16(0, out);
  write_uint16(w, out);
  write_uint16(h, out);
  write_uint8(24, out);
  write_uint8(32, out);

  int *p = bmp->row[overscroll] + overscroll;
  unsigned char *linebuf = new unsigned char[w * 3];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int xx = 0;
    for(x = 0; x < w; x++)
    {
      linebuf[xx + 0] = (*p >> 16) & 0xff;
      linebuf[xx + 1] = (*p >> 8) & 0xff;
      linebuf[xx + 2] = *p & 0xff;
      p++;
      xx += 3;
    }
    p += overscroll * 2;

    fwrite(linebuf, 1, w * 3, out);
  }

  delete[] linebuf;
  fclose(out);
}

void save_png(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
  {
    fclose(out);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, 0);
    fclose(out);
    return;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(out);
    return;
  }

  Bitmap *bmp = Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  int w = bmp->w - overscroll * 2;
  int h = bmp->h - overscroll * 2;

  png_init_io(png_ptr, out);
  png_set_IHDR(png_ptr, info_ptr, w, h, 8,
               PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  png_bytep linebuf = new png_byte[w * 3];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int *p = bmp->row[y + overscroll] + overscroll;
    // copy row
    for(x = 0; x < w * 3; x += 3)
    {
      linebuf[x + 0] = getr(*p); 
      linebuf[x + 1] = getg(*p); 
      linebuf[x + 2] = getb(*p); 
      p++;
    }

    png_write_row(png_ptr, linebuf);
  }

  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);
  delete[] linebuf;
  fclose(out);
}

void save_jpg(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  JSAMPLE *linebuf;
  int row_stride;

  Bitmap *bmp = Bitmap::main;
  int overscroll = Bitmap::main->overscroll;
  int w = bmp->w - overscroll * 2;
  int h = bmp->h - overscroll * 2;


  linebuf = new JSAMPLE[w * 3];
  if(!linebuf)
    return;

  if((out = fopen(fn, "wb")) == NULL)
    return;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  jpeg_stdio_dest(&cinfo, out);

  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
//FIXME need quality dialog
  jpeg_set_quality(&cinfo, 100, TRUE);

  jpeg_start_compress(&cinfo, TRUE);

  row_stride = w * 3;

  int x, y;

  int *p = bmp->row[overscroll] + overscroll;

  while(cinfo.next_scanline < cinfo.image_height)
  {
    // copy row
    for(x = 0; x < w * 3; x += 3)
    {
      linebuf[x + 0] = getr(*p); 
      linebuf[x + 1] = getg(*p); 
      linebuf[x + 2] = getb(*p); 
      p++;
    }

    row_pointer[0] = &linebuf[0];
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
    p += overscroll * 2;
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  delete[] linebuf;
  fclose(out);
}
