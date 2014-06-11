#include "rendera.h"
#include <jpeglib.h>
#include <setjmp.h>

static void write_uint8(uint8_t num, FILE *out)
{
  fputc(num, out);
}

static void write_uint16(uint16_t num, FILE *out)
{
  #if BYTE_ORDER == BIG_ENDIAN
  fputc((num >> 8) & 0xff, out);
  fputc(num & 0xff, out);
  #else
  fputc(num & 0xff, out);
  fputc((num >> 8) & 0xff, out);
  #endif
}

static void write_uint32(uint32_t num, FILE *out)
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

void save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Image");
  fc->filter("JPEG Image\t*.{jpg,jpeg}\nPNG Image\t*.png\nBitmap Image\t*.bmp\nTarga Image\t*.tga");
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
  
  if(strcasecmp(ext, ".tga") == 0)
    save_tga(fn);
  else
    return;

  delete fc;
}

// jpeg structures
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

// bmp structures
#pragma pack(1)
typedef struct
{
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
}
BITMAPFILEHEADER;

#pragma pack(1)
typedef struct
{
  uint32_t biSize;
  uint32_t biWidth;
  uint32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biXPelsPerMeter;
  uint32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
}
BITMAPINFOHEADER;

#pragma pack(1)
typedef struct
{
  uint8_t id_length;
  uint8_t color_map_type;
  uint8_t data_type;
  uint16_t color_map_origin;
  uint16_t color_map_length;
  uint8_t color_map_depth;
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint8_t bpp;
  uint8_t descriptor;
}
TARGA_HEADER;

/*
  uint8_t id_length;
  uint8_t color_map_type;
  uint8_t data_type;
  uint16_t color_map_origin;
  uint16_t color_map_length;
  uint8_t color_map_depth;
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
  uint8_t bpp;
  uint8_t descriptor;
*/

void save_tga(const char *fn)
{
  FILE *out = fl_fopen(fn, "wb");
  if(!out)
    return;

  Bitmap *bmp = Bitmap::main;
  int w = bmp->w - 64;
  int h = bmp->h - 64;

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

  int *p = bmp->row[32] + 32;
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
    p += 64;

    fwrite(linebuf, 1, w * 3, out);
  }

  delete[] linebuf;
  fclose(out);
}

