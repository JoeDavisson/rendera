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

#include "File.H"
#include "Bitmap.H"
#include "Map.H"
#include "Palette.H"
#include "Gui.H"
#include "View.H"
#include "Undo.H"
#include "Dialog.H"
#include "Tool.H"
#include "Stroke.H"
#include "Widget.H"
#include "Project.H"

#ifdef _WIN32
#define HAVE_BOOLEAN
#endif

#include <png.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <cstring>

Bitmap *File::preview_bmp = 0;

namespace
{
  #pragma pack(1)
  typedef struct
  {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
  }
  BMP_FILE_HEADER;

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
  BMP_INFO_HEADER;

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

  // jpeg structures
  struct my_error_mgr
  {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
  };

  typedef struct my_error_mgr *my_error_ptr;

  void jpg_exit(j_common_ptr cinfo)
  {
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
  }

  // palette preview widget
  Widget *pal_preview = new Widget(new Fl_Group(0, 0, 96, 96),
                                   0, 0, 96, 96, "", 6, 6, 0);

  const char *ext_string[] = { ".png", ".jpg", ".bmp", ".tga" };

  // store previous directory paths
  char load_dir[256];
  char save_dir[256];
  char pal_dir[256];

  void errorMessage()
  {
    fl_message_title("File Error");
    fl_message("An error occured during the operation.");
  }

  bool fileExists(const char *s)
  {
    FILE *temp = fopen(s, "r");

    if(temp)
    {
      fclose(temp);
      return 1;
    }

    return 0;
  }

  bool isPng(const unsigned char *header)
  {
    return (png_sig_cmp((png_bytep)header, 0, 8) == 0);
  }

  bool isJpeg(const unsigned char *header)
  {
    const unsigned char id[2] = { 0xFF, 0xD8 };
    return (memcmp(header, id, 2) == 0);
  }

  bool isBmp(const unsigned char *header)
  {
    return (memcmp(header, "BM", 2) == 0);
  }

  // targa has no real header, will have to trust the file extension
  bool isTarga(const char *fn)
  {
    const char *ext;
    ext = fl_filename_ext(fn);
    return (strcmp(ext, ".tga") == 0);
  }

  bool isGimpPalette(const unsigned char *header)
  {
    return (memcmp(header, "GIMP Palette", 12) == 0);
  }
}

void File::init()
{
  strcpy(load_dir, ".");
  strcpy(save_dir, ".");
  strcpy(pal_dir, ".");
}

void File::load(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Load Image");
  fc->filter("PNG Image\t*.png\nJPEG Image\t*.{jpg,jpeg}\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
  fc->directory(load_dir);

  switch(fc->show())
  {
    case -1:
    case 1:
      delete fc;
      return;
    default:
      getDirectory(load_dir, fc->filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc->filename());
  delete fc;

  FILE *in = fopen(fn, "rb");
  if(!in)
    return;

  unsigned char header[8];
  if(fread(&header, 1, 8, in) != 8)
  {
    fclose(in);
    errorMessage();
    return;
  }

  fclose(in);

  int overscroll = Project::overscroll;

  if(isPng(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadPNG((const char *)fn, overscroll)))
    {
      errorMessage();
      return;
    }
  }
  else if(isJpeg(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadJPG((const char *)fn, overscroll)))
    {
      errorMessage();
      return;
    }
  }
  else if(isBmp(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadBMP((const char *)fn, overscroll)))
    {
      errorMessage();
      return;
    }
  }
  else if(isTarga(fn))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadTGA((const char *)fn, overscroll)))
    {
      errorMessage();
      return;
    }
  }
  else
  {
    errorMessage();
    return;
  }

  delete Project::map;
  Project::map = new Map(Project::bmp->w, Project::bmp->h);

  Gui::getView()->tool->stroke->clip();
  Gui::getView()->zoomFit(Gui::getView()->fit);
  Gui::getView()->drawMain(true);
  Undo::reset();
}

int File::loadFile(const char *fn)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return -1;

  unsigned char header[8];
  if(fread(&header, 1, 8, in) != 8)
  {
    fclose(in);
    return -1;
  }

  fclose(in);

  int overscroll = Project::overscroll;

  if(isPng(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadPNG((const char *)fn, overscroll)))
    {
      return -1;
    }
  }
  else if(isJpeg(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadJPG((const char *)fn, overscroll)))
    {
      return -1;
    }
  }
  else if(isBmp(header))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadBMP((const char *)fn, overscroll)))
    {
      return -1;
    }
  }
  else if(isTarga(fn))
  {
    delete Project::bmp;
    if(!(Project::bmp = File::loadTGA((const char *)fn, overscroll)))
    {
      return -1;
    }
  }
  else
  {
    return -1;
  }

  delete Project::map;
  Project::map = new Map(Project::bmp->w, Project::bmp->h);

  Gui::getView()->tool->stroke->clip();
  Gui::getView()->zoomFit(Gui::getView()->fit);
  Gui::getView()->drawMain(true);
  Undo::reset();

  return 0;
}

Bitmap *File::loadJPG(const char *fn, int overscroll)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY linebuf;
  int row_stride;

  FILE *in = fopen(fn, "rb");
  if(!in)
    return 0;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpg_exit;

  if(setjmp(jerr.setjmp_buffer))
  {
    jpeg_destroy_decompress(&cinfo);
    fclose(in);
    return 0;
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

  Bitmap *temp = new Bitmap(w, h, overscroll);

  int x;
  int *p = temp->row[overscroll] + overscroll;

  if(bytes == 3)
  {
    while(cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines(&cinfo, linebuf, 1);

      for(x = 0; x < row_stride; x += 3)
      {
        *p++ = makeRgb(linebuf[0][x] & 0xFF,
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
        *p++ = makeRgb(linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF);
      }

      p += overscroll * 2;
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(in);

  return temp;
}

Bitmap *File::loadBMP(const char *fn, int overscroll)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return 0;

  BMP_INFO_HEADER bm;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(BMP_FILE_HEADER), in) !=
     (unsigned)sizeof(BMP_FILE_HEADER))
  {
    fclose(in);
    return 0;
  }

  unsigned char *p = buffer;

  /* bh.bfType = parseUint16(p); */
  /* bh.bfSize = parseUint32(p); */
  /* bh.bfReserved1 = parseUint16(p); */
  /* bh.bfReserved2 = parseUint16(p); */
  /* bh.bfOffBits = parseUint32(p); */

  if(fread(buffer, 1, sizeof(BMP_INFO_HEADER), in)
     != (unsigned)sizeof(BMP_INFO_HEADER))
  {
    fclose(in);
    return 0;
  }

  p = buffer;
  bm.biSize = parseUint32(p);
  bm.biWidth = parseUint32(p);
  bm.biHeight = parseUint32(p);
  bm.biPlanes = parseUint16(p);
  bm.biBitCount = parseUint16(p);
  bm.biCompression = parseUint32(p);
  bm.biSizeImage = parseUint32(p);
  bm.biXPelsPerMeter = parseUint32(p);
  bm.biYPelsPerMeter = parseUint32(p);
  bm.biClrUsed = parseUint32(p);
  bm.biClrImportant = parseUint32(p);

  int w = bm.biWidth;
  int h = bm.biHeight;
  int bits = bm.biBitCount;

  if(bits != 24)
  {
    fclose(in);
    return 0;
  }

  // skip additional header info if it exists
  if(bm.biSize > 40)
    fseek(in, bm.biSize - 40, SEEK_CUR);

  //dpix = bm.biXPelsPerMeter / 39.370079 + .5;
  //dpiy = bm.biYPelsPerMeter / 39.370079 + .5;

  int mul = 3;
  bool negx = false, negy = false;
  int pad = w % 4;

  if(w < 0)
    negx = true;
  if(h >= 0)
    negy = true;

  w = std::abs(w);
  h = std::abs(h);

  Bitmap *temp = new Bitmap(w, h, overscroll);

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
      return 0;
    }
    else
    {
      int xx = 0;
      for(x = 0; x < w; x++)
      {
        int x1 = negx ? w - 1 - x : x;
        x1 += overscroll;
        *(temp->row[y1] + x1) = makeRgb(linebuf[xx + 2] & 0xFF,
                                        linebuf[xx + 1] & 0xFF,
                                        linebuf[xx + 0] & 0xFF);
        xx += mul;
      }
    }
  }

  delete[] linebuf;
  fclose(in);

  return temp;
}

Bitmap *File::loadTGA(const char *fn, int overscroll)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return 0;

  TARGA_HEADER header;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(TARGA_HEADER), in) !=
     (unsigned)sizeof(TARGA_HEADER))
  {
    fclose(in);
    return 0;
  }

  unsigned char *p = buffer;

  header.id_length = parseUint8(p);
  header.color_map_type = parseUint8(p);
  header.data_type = parseUint8(p);
  header.color_map_origin = parseUint16(p);
  header.color_map_length = parseUint16(p);
  header.color_map_depth = parseUint8(p);
  header.x = parseUint16(p);
  header.y = parseUint16(p);
  header.w = parseUint16(p);
  header.h = parseUint16(p);
  header.bpp = parseUint8(p);
  header.descriptor = parseUint8(p);

  if(header.data_type != 2)
  {
    fclose(in);
    return 0;
  }

  if(header.bpp != 24 && header.bpp != 32)
  {
    fclose(in);
    return 0;
  }

  int depth = header.bpp / 8;

  // skip additional header info if it exists
  if(header.id_length > 0)
    fseek(in, header.id_length, SEEK_CUR);
  if(header.color_map_type > 0)
    fseek(in, header.color_map_length, SEEK_CUR);

  int w = header.w;
  int h = header.h;

  Bitmap *temp = new Bitmap(w, h, overscroll);

  unsigned char *linebuf = new unsigned char[w * depth];

  int x, y;

  bool negx = true;
  bool negy = true;

  if(header.descriptor & (1 << 4))
    negx = false;
  if(header.descriptor & (1 << 5))
    negy = false;

  int xstart = 0;
  int xend = w;
  int ystart = 0;
  int yend = h;

  if(negx)
  {
    xstart = w - 1;
    xend = -1;
  }

  if(negy)
  {
    ystart = h - 1;
    yend = -1;
  }

  for(y = ystart; y != yend; y += negy ? -1 : 1)
  {
    if(fread(linebuf, 1, w * depth, in) != (unsigned)(w * depth))
    {
      fclose(in);
      delete[] linebuf;
      return 0;
    }

    for(x = xstart; x != xend; x += negx ? -1 : 1)
    {
      if(depth == 3)
      {
        *(temp->row[y + overscroll] + x + overscroll) =
                       makeRgb((linebuf[x * depth + 2] & 0xFF),
                                (linebuf[x * depth + 1] & 0xFF),
                                (linebuf[x * depth + 0] & 0xFF));
      }
      else if(depth == 4)
      {
        *(temp->row[y + overscroll] + x + overscroll) =
                       makeRgba((linebuf[x * depth + 2] & 0xFF),
                                 (linebuf[x * depth + 1] & 0xFF),
                                 (linebuf[x * depth + 0] & 0xFF),
                                 (linebuf[x * depth + 3] & 0xFF));
      }
    }
  }

  delete[] linebuf;
  fclose(in);

  return temp;
}

Bitmap *File::loadPNG(const char *fn, int overscroll)
{
  FILE *in = fopen(fn, "rb");
  if(!in)
    return 0;

  unsigned char header[64];

  if(fread(header, 1, 8, in) != 8)
  {
    fclose(in);
    return 0;
  }

  if(!isPng(header))
  {
    fclose(in);
    return 0;
  }

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

  if(!png_ptr)
  {
    fclose(in);
    return 0;
  }

  info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr)
  {
    fclose(in);
    return 0;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    fclose(in);
    return 0;
  }

  png_init_io(png_ptr, in);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  int w = png_get_image_width(png_ptr, info_ptr);
  int h = png_get_image_height(png_ptr, info_ptr);

  /* png_byte color_type = png_get_color_type(png_ptr, info_ptr); */
  /* png_byte bpp = png_get_bit_depth(png_ptr, info_ptr); */
  /* int passes = png_set_interlace_handling(png_ptr); */

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  int depth = rowbytes / w;
//FIXME cancel if not 24 or 32 bit

  png_bytep linebuf = new png_byte[rowbytes];

  int x, y;

  Bitmap *temp = new Bitmap(w, h, overscroll);

  int *p = temp->row[overscroll] + overscroll;

  for(y = 0; y < h; y++)
  {
    png_read_row(png_ptr, linebuf, (png_bytep)0); 

    int xx = 0;

    for(x = 0; x < w; x++)
    {
      if(depth == 3)
      {
        *p++ = makeRgb(linebuf[xx + 0] & 0xFF,
                       linebuf[xx + 1] & 0xFF,
                       linebuf[xx + 2] & 0xFF);
      }
      else if(depth == 4)
      {
        *p++ = makeRgba(linebuf[xx + 0] & 0xFF,
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

  return temp;
}

void File::save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Image");
  fc->filter("PNG Image\t*.png\nJPEG Image\t*.jpg\nBitmap Image\t*.bmp\nTarga Image\t*.tga\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc->directory(save_dir);

  switch(fc->show())
  {
    case -1:
    case 1:
      delete fc;
      return;
    default:
      getDirectory(save_dir, fc->filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc->filename());
  int ext = fc->filter_value();
  fl_filename_setext(fn, sizeof(fn), ext_string[ext]);

  if(fileExists(fn))
  {
    fl_message_title("Replace File?");
    if(fl_choice("Replace File?", "No", "Yes", NULL) == 0)
      return;
  }
  
  switch(ext)
  {
    case 0:
      if(File::savePNG(fn) < 0)
      {
        errorMessage();
        return;
      }
      break;
    case 1:
      if(File::saveJPG(fn) < 0)
      {
        errorMessage();
        return;
      }
      break;
    case 2:
      if(File::saveBMP(fn) < 0)
      {
        errorMessage();
        return;
      }
      break;
    case 3:
      if(File::saveTGA(fn) < 0)
      {
        errorMessage();
        return;
      }
      break;
  }
}

int File::saveBMP(const char *fn)
{
  FILE *out = fopen(fn, "wb");
  if(!out)
    return -1;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;
  int pad = w % 4;

  // BMP_FILE_HEADER
  writeUint8('B', out);
  writeUint8('M', out);
  writeUint32(14 + 40 + ((w + pad) * h) * 3, out);
  writeUint16(0, out);
  writeUint16(0, out);
  writeUint32(14 + 40, out);

  // BMP_INFO_HEADER
  writeUint32(40, out);
  writeUint32(w, out);
  writeUint32(-h, out);
  writeUint16(1, out);
  writeUint16(24, out);
  writeUint32(0, out);
  writeUint32(0, out);
// FIXME eventually the program will set the dpi for use here
//       defaulting to 300 for now
  writeUint32(300 * 39.370079 + .5, out);
  writeUint32(300 * 39.370079 + .5, out);
  writeUint32(0, out);
  writeUint32(0, out);

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

    if(fwrite(&linebuf[0], 1, w * 3, out) != w * 3)
    {
      delete[] linebuf;
      fclose(out);
      return -1;
    }
  }

  delete[] linebuf;
  fclose(out);

  return 0;
}

int File::saveTGA(const char *fn)
{
  FILE *out = fopen(fn, "wb");
  if(!out)
    return -1;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  writeUint8(0, out);
  writeUint8(0, out);
  writeUint8(2, out);
  writeUint16(0, out);
  writeUint16(0, out);
  writeUint8(0, out);
  writeUint16(0, out);
  writeUint16(0, out);
  writeUint16(w, out);
  writeUint16(h, out);
  writeUint8(32, out);
  writeUint8(32, out);

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

    if(fwrite(&linebuf[0], 1, w * 4, out) != w * 4)
    {
      delete[] linebuf;
      fclose(out);
      return -1;
    }
  }

  delete[] linebuf;
  fclose(out);

  return 0;
}

int File::savePNG(const char *fn)
{
  FILE *out = fopen(fn, "wb");
  if(!out)
    return -1;

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
  {
    fclose(out);
    return -1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, 0);
    fclose(out);
    return -1;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(out);
    return -1;
  }

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

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

  return 0;
}

int File::saveJPG(const char *fn)
{
  Dialog::jpegQuality();

  int quality = Dialog::jpegQualityValue();

  FILE *out = fopen(fn, "wb");
  if(!out)
    return -1;

  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  JSAMPLE *linebuf;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  linebuf = new JSAMPLE[w * 3];
  if(!linebuf)
  {
    fclose(out);
    return -1;
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

  int x;

  int *p = bmp->row[overscroll] + overscroll;

  while(cinfo.next_scanline < cinfo.image_height)
  {
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

  return 0;
}

Fl_Image *File::previewPNG(const char *fn, unsigned char *header, int)
{
  if(!isPng(header))
    return 0;

  if(File::preview_bmp)
    delete File::preview_bmp;
  File::preview_bmp = loadPNG(fn, 0);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)File::preview_bmp->data,
                     File::preview_bmp->w, File::preview_bmp->h,
                     4, 0);

  return image;
}

Fl_Image *File::previewJPG(const char *fn, unsigned char *header, int)
{
  if(!isJpeg(header))
    return 0;

  if(File::preview_bmp)
    delete File::preview_bmp;
  File::preview_bmp = loadJPG(fn, 0);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)File::preview_bmp->data,
                     File::preview_bmp->w, File::preview_bmp->h,
                     4, 0);

  return image;
}

Fl_Image *File::previewBMP(const char *fn, unsigned char *header, int)
{
  if(!isBmp(header))
    return 0;

  if(File::preview_bmp)
    delete File::preview_bmp;
  File::preview_bmp = loadBMP(fn, 0);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)File::preview_bmp->data,
                     File::preview_bmp->w, File::preview_bmp->h,
                     4, 0);

  return image;
}

Fl_Image *File::previewTGA(const char *fn, unsigned char *, int)
{
  if(!isTarga(fn))
    return 0;

  if(File::preview_bmp)
    delete File::preview_bmp;
  File::preview_bmp = loadTGA(fn, 0);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)File::preview_bmp->data,
                     File::preview_bmp->w, File::preview_bmp->h,
                     4, 0);

  return image;
}

Fl_Image *File::previewGPL(const char *fn, unsigned char *header, int)
{
  if(!isGimpPalette(header))
    return 0;

  Palette *temp_pal = new Palette();

  temp_pal->load(fn);
  if(temp_pal->max == 0)
    return 0;

  temp_pal->draw(pal_preview);

  Fl_RGB_Image *image =
    new Fl_RGB_Image((unsigned char *)pal_preview->bitmap->data,
                     96, 96, 4, 0);

  delete temp_pal;
  return image;
}

void File::loadPalette()
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Load Palette");
  fc->filter("GIMP Palette\t*.gpl\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_FILE);
  fc->directory(pal_dir);

  switch(fc->show())
  {
    case -1:
    case 1:
      delete fc;
      return;
    default:
      getDirectory(pal_dir, fc->filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc->filename());
  delete fc;

  FILE *in = fopen(fn, "r");
  if(!in)
    return;

  unsigned char header[12];
  if(fread(&header, 1, 12, in) != 12)
  {
    fclose(in);
    errorMessage();
    return;
  }

  fclose(in);

  if(isGimpPalette(header))
  {
    if(Project::palette->load((const char*)fn) < 0)
    {
      errorMessage();
      return;
    }
    Gui::drawPalette();
  }
}

void File::savePalette()
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser();
  fc->title("Save Palette");
  fc->filter("GIMP Palette\t*.gpl\n");
  fc->options(Fl_Native_File_Chooser::PREVIEW);
  fc->type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc->directory(pal_dir);

  switch(fc->show())
  {
    case -1:
    case 1:
      delete fc;
      return;
    default:
      getDirectory(pal_dir, fc->filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc->filename());
  fl_filename_setext(fn, sizeof(fn), ".gpl");

  delete fc;

  if(fileExists(fn))
  {
    fl_message_title("Replace File?");
    if(fl_choice("Replace File?", "No", "Yes", NULL) == 0)
      return;
  }
  
  if(Project::palette->save(fn) < 0)
  {
    errorMessage();
    return;
  }
}

void File::decodeURI(char *s)
{
  int i, j;
  unsigned int c;
  int len = strlen(s);

  for(i = 0; i < len - 2; i++)
  {
    if(s[i] == '%')
    {
      if(sscanf(&s[i + 1], "%2X", &c) != 1)
        break;

      s[i] = c;

      for(j = 0; j < len - (i + 2); j++)
        s[i + 1 + j] = s[i + 3 + j];

      len -= 2;
    }
  }
}

void File::getDirectory(char *dest, const char *src)
{
  strcpy(dest, src);

  int i;
  int len = strlen(dest);
  if(len < 2)
    return;

  for(i = len - 1; i > 0; i--)
  {
    if(dest[i - 1] == '/')
    {
      dest[i] = '\0';
      break;
    }
  }
}

