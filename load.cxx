#include "rendera.h"
#define HAVE_BOOLEAN
#include <jpeglib.h>
#include <setjmp.h>

extern Gui *gui;

// jpeg structures
struct my_error_mgr
{
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

void load(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Load Image");
  fc->filter("PNG Image\t*.png\nJPEG Image\t*.{jpg,jpeg}\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
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
  
  FILE *in = fl_fopen(fn, "rb");
  if(!in)
  {
    delete fc;
    return;
  }

  unsigned char header[8];
  if(fread(&header, 1, 8, in) != 8)
  {
    fclose(in);
    delete fc;
    return;
  }

  fclose(in);

  if(png_sig_cmp(header, 0, 8) == 0)
    load_png(fn, Bitmap::main, 64);
  else if(memcmp(header, (const unsigned char[2]){ 0xff, 0xd8 }, 2) == 0)
    load_jpg(fn, Bitmap::main, 64);
  else if(memcmp(header, "BM", 2) == 0)
    load_bmp(fn, Bitmap::main, 64);
  else if(strcasecmp(ext, ".tga") == 0)
    load_tga(fn, Bitmap::main, 64);
  else
  {
    delete fc;
    return;
  }

  delete fc;

  delete Map::main;
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

  gui->view->zoom_fit(gui->view->fit);
  gui->view->draw_main(1);
}

static void jpg_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  (*cinfo->err->output_message)(cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

#ifndef WINDOWS
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
#endif

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

Fl_Image *preview_png(const char *fn, unsigned char *header, int len)
{
  if(png_sig_cmp(header, 0, 8) != 0)
    return 0;

  load_png(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data, Bitmap::preview->w, Bitmap::preview->h, 4, 0);

  return image;
}

Fl_Image *preview_jpg(const char *fn, unsigned char *header, int len)
{
  if(memcmp(header, (const unsigned char[2]){ 0xff, 0xd8 }, 2) != 0)
    return 0;

  load_jpg(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data, Bitmap::preview->w, Bitmap::preview->h, 4, 0);

  return image;
}

Fl_Image *preview_bmp(const char *fn, unsigned char *header, int len)
{
  if(memcmp(header, "BM", 2) != 0)
    return 0;

  load_bmp(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data, Bitmap::preview->w, Bitmap::preview->h, 4, 0);

  return image;
}

Fl_Image *preview_tga(const char *fn, unsigned char *header, int len)
{
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

  if(strcasecmp(ext, ".tga") != 0)
    return 0;

  load_tga(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data, Bitmap::preview->w, Bitmap::preview->h, 4, 0);

  return image;
}

void load_jpg(const char *fn, Bitmap *bitmap, int overscroll)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY linebuf;
  int row_stride;

  FILE *in = fl_fopen(fn, "rb");
  if(!in)
    return;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpg_exit;

  if(setjmp(jerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&cinfo);
    fclose(in);
    return;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, in);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  linebuf = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  int bytes = cinfo.out_color_components;

  int w = row_stride / bytes;
  int h = cinfo.output_height;
  int aw = w + overscroll * 2;
  int ah = h + overscroll * 2;

  delete bitmap;
  bitmap = new Bitmap(w, h, overscroll,
               makecol(255, 255, 255), makecol(128, 128, 128));
  int x;
  int *p = bitmap->row[overscroll] + overscroll;

  if(bytes == 3)
  {
    while(cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines(&cinfo, linebuf, 1);
      for(x = 0; x < row_stride; x += 3)
      {
        *p++ = makecol(linebuf[0][x] & 0xFF,
                       linebuf[0][x + 1] & 0xFF,
                       linebuf[0][x + 2] & 0xFF);
      }

      p += overscroll * 2;
    }
  }
  else
  {
    while(cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines(&cinfo, linebuf, 1);
      for(x = 0; x < row_stride; x += 1)
      {
        *p++ = makecol(linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF);
      }

      p += overscroll * 2;
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(in);
}

void load_bmp(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fl_fopen(fn, "rb");
  if(!in)
    return;

  BITMAPFILEHEADER bh;
  BITMAPINFOHEADER bm;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(BITMAPFILEHEADER), in) != (unsigned)sizeof(BITMAPFILEHEADER))
  {
    fclose(in);
    return;
  }

  unsigned char *p = buffer;
  bh.bfType = parse_uint16(p);
  bh.bfSize = parse_uint32(p);
  bh.bfReserved1 = parse_uint16(p);
  bh.bfReserved2 = parse_uint16(p);
  bh.bfOffBits = parse_uint32(p);

  if(fread(buffer, 1, sizeof(BITMAPINFOHEADER), in) != (unsigned)sizeof(BITMAPINFOHEADER))
  {
    fclose(in);
    return;
  }

  p = buffer;
  bm.biSize = parse_uint32(p);
  bm.biWidth = parse_uint32(p);
  bm.biHeight = parse_uint32(p);
  bm.biPlanes = parse_uint16(p);
  bm.biBitCount = parse_uint16(p);
  bm.biCompression = parse_uint32(p);
  bm.biSizeImage = parse_uint32(p);
  bm.biXPelsPerMeter = parse_uint32(p);
  bm.biYPelsPerMeter = parse_uint32(p);
  bm.biClrUsed = parse_uint32(p);
  bm.biClrImportant = parse_uint32(p);

  int w = bm.biWidth;
  int h = bm.biHeight;
  int bits = bm.biBitCount;

  if(bits != 24)
  {
    fclose(in);
    return;
  }

  // skip additional header info if it exists
  if(bm.biSize > 40)
    fseek(in, bm.biSize - 40, SEEK_CUR);

  //dpix = bm.biXPelsPerMeter / 39.370079 + .5;
  //dpiy = bm.biYPelsPerMeter / 39.370079 + .5;

  int mul = 3;
  int negx = 0, negy = 0;
  int pad = w % 4;

  if(w < 0)
    negx = 1;
  if(h >= 0)
    negy = 1;

  w = ABS(w);
  h = ABS(h);

  int aw = w + overscroll * 2;
  int ah = h + overscroll * 2;

  delete bitmap;
  bitmap = new Bitmap(w, h, overscroll,
                      makecol(255, 255, 255), makecol(128, 128, 128));

  unsigned char *linebuf = new unsigned char[w * mul + pad];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int y1 = negy ? h - 1 - y : y;
    y1 += overscroll;

    if(fread(linebuf, 1, w * mul + pad, in) != (unsigned)(w * mul + pad))
    {
      fclose(in);
      return;
    }
    else
    {
      int xx = 0;
      for(x = 0; x < w; x++)
      {
        int x1 = negx ? w - 1 - x : x;
        x1 += overscroll;
        *(bitmap->row[y1] + x1) = makecol(linebuf[xx + 2] & 0xFF,
                                          linebuf[xx + 1] & 0xFF,
                                          linebuf[xx + 0] & 0xFF);
        xx += mul;
      }
    }
  }

  delete[] linebuf;
  fclose(in);
}

void load_tga(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fl_fopen(fn, "rb");
  if(!in)
    return;

  TARGA_HEADER header;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(TARGA_HEADER), in) != (unsigned)sizeof(TARGA_HEADER))
  {
    fclose(in);
    return;
  }

  unsigned char *p = buffer;

  header.id_length = parse_uint8(p);
  header.color_map_type = parse_uint8(p);
  header.data_type = parse_uint8(p);
  header.color_map_origin = parse_uint16(p);
  header.color_map_length = parse_uint16(p);
  header.color_map_depth = parse_uint8(p);
  header.x = parse_uint16(p);
  header.y = parse_uint16(p);
  header.w = parse_uint16(p);
  header.h = parse_uint16(p);
  header.bpp = parse_uint8(p);
  header.descriptor = parse_uint8(p);

  if(header.data_type != 2)
  {
    fclose(in);
    return;
  }

  if(header.bpp != 24 && header.bpp != 32)
  {
    fclose(in);
    return;
  }

  int depth = header.bpp / 8;

  // skip additional header info if it exists
  if(header.id_length > 0)
    fseek(in, header.id_length, SEEK_CUR);
  if(header.color_map_type > 0)
    fseek(in, header.color_map_length, SEEK_CUR);

  int w = header.w;
  int h = header.h;

  int aw = w + overscroll * 2;
  int ah = h + overscroll * 2;

  delete bitmap;
  bitmap = new Bitmap(w, h, overscroll,
                      makecol(255, 255, 255), makecol(128, 128, 128));

  unsigned char *linebuf = new unsigned char[w * depth];

  int x, y;

  int negx = 1;
  int negy = 1;

  if(header.descriptor & (1 << 4))
    negx = 0;
  if(header.descriptor & (1 << 5))
    negy = 0;

  int xstart = 0;
  int xend = w - 1;
  int ystart = 0;
  int yend = h - 1;

  if(negx)
    SWAP(xstart, xend);
  if(negy)
    SWAP(ystart, yend);

  for(y = ystart; y != yend; y += negy ? -1 : 1)
  {
    if(fread(linebuf, 1, w * depth, in) != (unsigned)(w * depth))
    {
      fclose(in);
      return;
    }
    for(x = xstart; x != xend; x += negx ? -1 : 1)
    {
      *(bitmap->row[y + overscroll] + x + overscroll) =
                     makecol((linebuf[x * depth + 2] & 0xFF),
                             (linebuf[x * depth + 1] & 0xFF),
                             (linebuf[x * depth + 0] & 0xFF));
    }
  }

  delete[] linebuf;
  fclose(in);
}

void load_png(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fl_fopen(fn, "rb");
  if(!in)
    return;

  unsigned char header[64];

  if(fread(header, 1, 8, in) != 8)
  {
    fclose(in);
    return;
  }

  if(png_sig_cmp(header, 0, 8) != 0)
  {
    fclose(in);
    return;
  }

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
  {
    fclose(in);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    fclose(in);
    return;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    fclose(in);
    return;
  }

  png_init_io(png_ptr, in);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  int w = png_get_image_width(png_ptr, info_ptr);
  int h = png_get_image_height(png_ptr, info_ptr);

  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bpp = png_get_bit_depth(png_ptr, info_ptr);

  int passes = png_set_interlace_handling(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  int depth = rowbytes / w;

  png_bytep linebuf = new png_byte[rowbytes];

  int x, y;

  int aw = w + overscroll * 2;
  int ah = h + overscroll * 2;

  delete bitmap;
  bitmap = new Bitmap(w, h, overscroll,
                      makecol(255, 255, 255), makecol(128, 128, 128));

  int *p = bitmap->row[overscroll] + overscroll;

  for(y = 0; y < h; y++)
  {
    png_read_row(png_ptr, linebuf, (png_bytep)0); 
    int xx = 0;
    for(x = 0; x < w; x++)
    {
      *p++ = makecol(linebuf[xx + 0] & 0xFF,
                     linebuf[xx + 1] & 0xFF,
                     linebuf[xx + 2] & 0xFF);
      xx += depth;
    }
    p += overscroll * 2;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  delete[] linebuf;
  fclose(in);
}

