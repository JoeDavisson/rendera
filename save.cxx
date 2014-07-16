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

/*
static int file_exists(const char *s)
{
  FILE *temp = fopen(s, "r");

  if(temp)
  {
    fclose(temp);
    return 1;
  }
  else
  {
    fclose(temp);
    return 0;
  }
}
*/

void save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Image");
//  fc->filter("JPEG Image\t*.{jpg,jpeg}\nPNG Image\t*.png\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->filter("Bitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  //fc->options(Fl_Native_File_Chooser::PREVIEW);
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

/*
  if(file_exists(fn))
  {
    fl_message_title("Replace File?");
    if(fl_choice("Replace File?", "No", "Yes", NULL) == 1)
      return;
  }
*/
  
  if(strcasecmp(ext, ".bmp") == 0)
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

// jpeg structures
/*
struct my_error_mgr
{
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

static void jpg_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  (*cinfo->err->output_message)(cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}
*/

void save_bmp(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  Bitmap *bmp = Bitmap::main;
  int overscroll = Bitmap::overscroll;
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
  int overscroll = Bitmap::overscroll;
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

