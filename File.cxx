/*
Copyright (c) 2014 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "File.h"
#include "Bitmap.h"
#include "Map.h"
#include "Palette.h"
#include "Gui.h"
#include "View.h"
#include "Undo.h"
#include "Dialog.h"
#include "Field.h"
#include "Widget.h"

#ifdef _WIN32
#define HAVE_BOOLEAN
#endif

#include <jpeglib.h>
#include <setjmp.h>

#ifndef _WIN32
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

void File::load(Fl_Widget *, void *)
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
  
  FILE *in = fopen(fn, "rb");
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
    File::loadPNG(fn, Bitmap::main, 64);
  else if(memcmp(header, (const unsigned char[2]){ 0xff, 0xd8 }, 2) == 0)
    File::loadJPG(fn, Bitmap::main, 64);
  else if(memcmp(header, "BM", 2) == 0)
    File::loadBMP(fn, Bitmap::main, 64);
  else if(strcasecmp(ext, ".tga") == 0)
    File::loadTGA(fn, Bitmap::main, 64);
  else
  {
    delete fc;
    return;
  }

  delete fc;

  delete Map::main;
  Map::main = new Map(Bitmap::main->w, Bitmap::main->h);

  Gui::view->zoom_fit(Gui::view->fit);
  Gui::view->draw_main(1);
  Undo::reset();
}

void File::loadJPG(const char *fn, Bitmap *bitmap, int overscroll)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY linebuf;
  int row_stride;

  FILE *in = fopen(fn, "rb");
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

void File::loadBMP(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return;

  BITMAPFILEHEADER bh;
  BITMAPINFOHEADER bm;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(BITMAPFILEHEADER), in) !=
     (unsigned)sizeof(BITMAPFILEHEADER))
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
      delete[] linebuf;
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

void File::loadTGA(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return;

  TARGA_HEADER header;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(TARGA_HEADER), in) !=
     (unsigned)sizeof(TARGA_HEADER))
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
      delete[] linebuf;
      return;
    }

    for(x = xstart; x != xend; x += negx ? -1 : 1)
    {
      if(depth == 3)
      {
        *(bitmap->row[y + overscroll] + x + overscroll) =
                       makecol((linebuf[x * depth + 2] & 0xFF),
                               (linebuf[x * depth + 1] & 0xFF),
                               (linebuf[x * depth + 0] & 0xFF));
      }
      else if(depth == 4)
      {
        *(bitmap->row[y + overscroll] + x + overscroll) =
                       makecola((linebuf[x * depth + 2] & 0xFF),
                                (linebuf[x * depth + 1] & 0xFF),
                                (linebuf[x * depth + 0] & 0xFF),
                                (linebuf[x * depth + 3] & 0xFF));
      }
    }
  }

  delete[] linebuf;
  fclose(in);
}

void File::loadPNG(const char *fn, Bitmap *bitmap, int overscroll)
{
  FILE *in = fopen(fn, "rb");
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
//FIXME cancel if not 24 or 32 bit

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
      if(depth == 3)
      {
        *p++ = makecol(linebuf[xx + 0] & 0xFF,
                       linebuf[xx + 1] & 0xFF,
                       linebuf[xx + 2] & 0xFF);
      }
      else if(depth == 4)
      {
        *p++ = makecola(linebuf[xx + 0] & 0xFF,
                        linebuf[xx + 1] & 0xFF,
                        linebuf[xx + 2] & 0xFF,
                        linebuf[xx + 3] & 0xFF);
      }

      xx += depth;
    }
    p += overscroll * 2;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  delete[] linebuf;
  fclose(in);
}

void File::save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Image");
  fc->filter("PNG Image\t*.png\nJPEG Image\t*.jpg\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
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
    File::savePNG(fn);
  else if(strcasecmp(ext, ".jpg") == 0)
    File::saveJPG(fn);
  else if(strcasecmp(ext, ".bmp") == 0)
    File::saveBMP(fn);
  else if(strcasecmp(ext, ".tga") == 0)
    File::saveTGA(fn);
  else
  {
    delete fc;
    return;
  }

  delete fc;
}

void File::saveBMP(const char *fn)
{
  FILE *out = fopen(fn, "wb");
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

    fwrite(&linebuf[0], 1, w * 3, out);
  }

  delete[] linebuf;
  fclose(out);
}

void File::saveTGA(const char *fn)
{
  FILE *out = fopen(fn, "wb");
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
  write_uint8(32, out);
  write_uint8(32, out);

  int *p = bmp->row[overscroll] + overscroll;
  unsigned char *linebuf = new unsigned char[w * 4];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int xx = 0;
    for(x = 0; x < w; x++)
    {
      linebuf[xx + 0] = (*p >> 16) & 0xff;
      linebuf[xx + 1] = (*p >> 8) & 0xff;
      linebuf[xx + 2] = *p & 0xff;
      linebuf[xx + 3] = (*p >> 24) & 0xff;
      p++;
      xx += 4;
    }
    p += overscroll * 2;

    fwrite(&linebuf[0], 1, w * 4, out);
  }

  delete[] linebuf;
  fclose(out);
}

void File::savePNG(const char *fn)
{
  FILE *out = fopen(fn, "wb");
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
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);

  png_bytep linebuf = new png_byte[w * 4];

  int x, y;

  for(y = 0; y < h; y++)
  {
    int *p = bmp->row[y + overscroll] + overscroll;
    // copy row
    for(x = 0; x < w * 4; x += 4)
    {
      linebuf[x + 0] = getr(*p); 
      linebuf[x + 1] = getg(*p); 
      linebuf[x + 2] = getb(*p); 
      linebuf[x + 3] = geta(*p); 
      p++;
    }

    png_write_row(png_ptr, &linebuf[0]);
  }

  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);
  delete[] linebuf;
  fclose(out);
}

void File::saveJPG(const char *fn)
{
  Dialog::showJpegQuality();

  int quality = atoi(Dialog::jpeg_quality_amount->value());
  if(quality < 1)
    quality = 1;
  if(quality > 100)
    quality = 100;

  FILE *out = fopen(fn, "wb");
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
  {
    fclose(out);
    return;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  jpeg_stdio_dest(&cinfo, out);

  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE);

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

Fl_Image *File::previewPNG(const char *fn, unsigned char *header, int len)
{
  if(png_sig_cmp(header, 0, 8) != 0)
    return 0;

  loadPNG(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data,
                                         Bitmap::preview->w, Bitmap::preview->h,
                                         4, 0);

  return image;
}

Fl_Image *File::previewJPG(const char *fn, unsigned char *header, int len)
{
  if(memcmp(header, (const unsigned char[2]){ 0xff, 0xd8 }, 2) != 0)
    return 0;

  loadJPG(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data,
                                         Bitmap::preview->w, Bitmap::preview->h,
                                         4, 0);

  return image;
}

Fl_Image *File::previewBMP(const char *fn, unsigned char *header, int len)
{
  if(memcmp(header, "BM", 2) != 0)
    return 0;

  loadBMP(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data,
                                         Bitmap::preview->w, Bitmap::preview->h,
                                         4, 0);

  return image;
}

Fl_Image *File::previewTGA(const char *fn, unsigned char *header, int len)
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

  loadTGA(fn, Bitmap::preview, 0);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)Bitmap::preview->data,
                                         Bitmap::preview->w, Bitmap::preview->h,
                                         4, 0);

  return image;
}

Fl_Image *File::previewGPL(const char *fn, unsigned char *header, int len)
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

  if(strcasecmp(ext, ".gpl") != 0)
    return 0;

  Palette *pal = new Palette();

  pal->load(fn);
  if(pal->max == 0)
    return 0;

  pal->draw(Gui::pal_preview);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)Gui::pal_preview->bitmap->data,
                     96, 96, 4, 0);

  delete pal;
  return image;
}

