/*
Copyright (c) 2015 Joe Davisson.

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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

#include "../fltk-1.3.3/png/png.h"

#ifdef __WIN32
#define HAVE_BOOLEAN
// prevent jmorecfg.h from defining INT32
#ifndef XMD_H
#define XMD_H
#endif
#endif

#include "../fltk-1.3.3/jpeg/jpeglib.h"

// this must be included after pnglib
#include <setjmp.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Native_File_Chooser.H>

#include "Bitmap.H"
#include "Dialog.H"
#include "File.H"
#include "FileSP.H"
#include "Gui.H"
#include "Inline.H"
#include "Map.H"
#include "RenderaMath.H"
#include "Palette.H"
#include "Project.H"
#include "Stroke.H"
#include "Tool.H"
#include "Undo.H"
#include "View.H"
#include "Widget.H"

namespace
{
  #pragma pack(push)
  #pragma pack(1)

  // BMP file format structures
  struct bmp_file_header_type
  {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
  };

  struct bmp_info_header_type
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
  };

  // targa file format structure
  struct targa_header_type
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
  };

  #pragma pack(pop)

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

  // bitmap pointer used for FLTK's file preview
  Bitmap *preview_bmp = 0;

  // palette preview widget
  Widget *pal_preview = new Widget(new Fl_Group(0, 0, 96, 96),
                                   0, 0, 96, 96, "", 6, 6, 0);

  // file extensions in the order they appear in the file chooser dialog
  // (used to automatically append a file extension)
  const char *ext_string[] = { ".png", ".jpg", ".bmp", ".tga", ".java" };

  // store previous directory paths
  char load_dir[256];
  char save_dir[256];
  char pal_load_dir[256];
  char pal_save_dir[256];

  // show error dialog
  void errorMessage()
  {
    Dialog::message("File Error", "An error occured during the operation.");
  }

  // check if trying to overwrite existing file
  bool fileExists(const char *fn)
  {
    FILE *temp = fopen(fn, "rb");

    if(temp)
    {
      fclose(temp);
      return 1;
    }

    return 0;
  }

  // file header tests
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

  bool isTarga(const char *fn)
  {
    // targa has no real header, will have to trust the file extension
    const char *ext;
    ext = fl_filename_ext(fn);
    return (strcmp(ext, ".tga") == 0);
  }

  bool isGimpPalette(const unsigned char *header)
  {
    return (memcmp(header, "GIMP Palette", 12) == 0);
  }

  struct png_state
  {
    png_uint_32 pos;
    png_uint_32 size;
    const unsigned char *array;
  };

  // callback for reading PNG from byte array
  void pngReadFromArray(png_structp png_ptr, png_bytep dest, png_uint_32 length)
  {
    struct png_state *src = (struct png_state *)png_get_io_ptr(png_ptr);

    memcpy(dest, src->array + src->pos, length);
    src->pos += length;
  }

  // reset directories
  int *init()
  {
    strcpy(load_dir, ".");
    strcpy(save_dir, ".");
    strcpy(pal_load_dir, ".");
    strcpy(pal_save_dir, ".");
    return 0;
  }

  // force init to run when program starts
  static const int *temp = init(); 
}

// display file loading dialog
void File::load(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser fc;
  fc.title("Load Image");
  fc.filter("PNG Image\t*.png\n"
            "JPEG Image\t*.{jpg,jpeg}\n"
            "Bitmap Image\t*.bmp\n"
            "Targa Image\t*.tga\n");
  fc.options(Fl_Native_File_Chooser::PREVIEW);
  fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
  fc.directory(load_dir);

  switch(fc.show())
  {
    case -1:
    case 1:
      return;
    default:
      getDirectory(load_dir, fc.filename());
      break;
  }

  loadFile(fc.filename());
}

// load a file
int File::loadFile(const char *fn)
{
  FileSP in(fn, "rb");
  if(!in.get())
    return -1;

  unsigned char header[8];
  if(fread(&header, 1, 8, in.get()) != 8)
    return -1;

  // load to a temporary bitmap first
  int overscroll = Project::overscroll;
  Bitmap *temp = 0;

  if(isPng(header))
    temp = File::loadPng((const char *)fn, overscroll);
  else if(isJpeg(header))
    temp = File::loadJpeg((const char *)fn, overscroll);
  else if(isBmp(header))
    temp = File::loadBmp((const char *)fn, overscroll);
  else if(isTarga(fn))
    temp = File::loadTarga((const char *)fn, overscroll);

  if(!temp)
  {
    errorMessage();
    return -1;
  }

  // load was successful, set the main bitmap to use the temp pointer
  // and resize the brushstroke map to match the new image size
  delete Project::bmp;
  Project::bmp = temp;

  delete Project::map;
  Project::map = new Map(Project::bmp->w, Project::bmp->h);

  // redraw
  Project::stroke->clip();
  Gui::getView()->zoomFit(Gui::getView()->fit);
  Gui::getView()->drawMain(true);
  Undo::init();

  return 0;
}

Bitmap *File::loadJpeg(const char *fn, int overscroll)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;

  FileSP in(fn, "rb");
  if(!in.get())
    return 0;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = jpg_exit;

  if(setjmp(jerr.setjmp_buffer))
  {
    // jpeglib does a goto here if there is an error
    jpeg_destroy_decompress(&cinfo);
    return 0;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, in.get());
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  int row_stride = cinfo.output_width * cinfo.output_components;
  JSAMPARRAY linebuf = (*cinfo.mem->alloc_sarray)
              ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
  int bytes = cinfo.out_color_components;
  int w = row_stride / bytes;
  int h = cinfo.output_height;

// FIXME need to support dpi in load/save, here are the fields:
//printf("%d\n", cinfo.density_unit);
//printf("%d\n", cinfo.X_density);
//printf("%d\n", cinfo.Y_density);

  Bitmap *volatile temp = new Bitmap(w, h, overscroll);
  int *p = temp->row[overscroll] + overscroll;

  if(bytes == 3)
  {
    while(cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines(&cinfo, linebuf, 1);

      for(int x = 0; x < row_stride; x += 3)
      {
        *p++ = makeRgb(linebuf[0][x + 0] & 0xFF,
                       linebuf[0][x + 1] & 0xFF,
                       linebuf[0][x + 2] & 0xFF);
      }

      p += overscroll * 2;
    }
  }
  else if(bytes == 1)
  {
    while(cinfo.output_scanline < cinfo.output_height)
    {
      jpeg_read_scanlines(&cinfo, linebuf, 1);

      for(int x = 0; x < row_stride; x += 1)
      {
        *p++ = makeRgb(linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF,
                       linebuf[0][x] & 0xFF);
      }

      p += overscroll * 2;
    }
  }
  else
  {
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return temp;
}

Bitmap *File::loadBmp(const char *fn, int overscroll)
{
  FileSP in(fn, "rb");
  if(!in.get())
    return 0;

  bmp_info_header_type bm;
  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(bmp_file_header_type), in.get()) !=
     (unsigned)sizeof(bmp_file_header_type))
  {
    return 0;
  }

  unsigned char *p = buffer;

  /* bh.bfType = parseUint16(p); */
  /* bh.bfSize = parseUint32(p); */
  /* bh.bfReserved1 = parseUint16(p); */
  /* bh.bfReserved2 = parseUint16(p); */
  /* bh.bfOffBits = parseUint32(p); */

  if(fread(buffer, 1, sizeof(bmp_info_header_type), in.get())
     != (unsigned)sizeof(bmp_info_header_type))
  {
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
    return 0;

  // skip additional header info if it exists
  if(bm.biSize > 40)
    fseek(in.get(), bm.biSize - 40, SEEK_CUR);

  //dpix = bm.biXPelsPerMeter / 39.370079 + .5;
  //dpiy = bm.biYPelsPerMeter / 39.370079 + .5;

  int mul = 3;
  bool negx = false, negy = false;
  int pad = w % 4;

  if(w < 0)
    negx = true;
  if(h >= 0)
    negy = true;

  w = RenderaMath::abs(w);
  h = RenderaMath::abs(h);

  Bitmap *temp = new Bitmap(w, h, overscroll);
  std::vector<unsigned char> linebuf(w * mul + pad);

  for(int y = 0; y < h; y++)
  {
    int y1 = negy ? h - 1 - y : y;
    y1 += overscroll;

    if(fread(&linebuf[0], 1, w * mul + pad, in.get()) !=
       (unsigned)(w * mul + pad))
    {
      return 0;
    }
    else
    {
      int xx = 0;
      for(int x = 0; x < w; x++)
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

  return temp;
}

Bitmap *File::loadTarga(const char *fn, int overscroll)
{
  FileSP in(fn, "rb");
  if(!in.get())
    return 0;

  targa_header_type header;

  unsigned char buffer[64];

  if(fread(buffer, 1, sizeof(targa_header_type), in.get()) !=
     (unsigned)sizeof(targa_header_type))
  {
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
    return 0;

  if(header.bpp != 24 && header.bpp != 32)
    return 0;

  int depth = header.bpp / 8;

  // skip additional header info if it exists
  if(header.id_length > 0)
    fseek(in.get(), header.id_length, SEEK_CUR);
  if(header.color_map_type > 0)
    fseek(in.get(), header.color_map_length, SEEK_CUR);

  int w = header.w;
  int h = header.h;

  Bitmap *temp = new Bitmap(w, h, overscroll);
  std::vector<unsigned char> linebuf(w * depth);

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

  for(int y = ystart; y != yend; y += negy ? -1 : 1)
  {
    if(fread(&linebuf[0], 1, w * depth, in.get()) != (unsigned)(w * depth))
      return 0;

    for(int x = xstart; x != xend; x += negx ? -1 : 1)
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

  return temp;
}

Bitmap *File::loadPng(const char *fn, int overscroll)
{
  FileSP in(fn, "rb");
  if(!in.get())
    return 0;

  unsigned char header[64];

  if(fread(header, 1, 8, in.get()) != 8)
    return 0;

  if(!isPng(header))
    return 0;

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
    return 0;

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, 0, 0);
    return 0;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    // pnglib does a goto here if there is an error
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return 0;
  }

  png_uint_32 temp_w = 0;
  png_uint_32 temp_h = 0;
  int bits_per_channel = 0;
  int color_type = 0;
  int interlace_type = 0;
  int compression_type = 0;
  int filter_method = 0;

  png_init_io(png_ptr, in.get());
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &temp_w, &temp_h,
               &bits_per_channel, &color_type,
               &interlace_type, &compression_type, &filter_method);

  int w = temp_w;
  int h = temp_h;

  // check interlace mode
  bool interlace = png_set_interlace_handling(png_ptr) > 1 ? 1 : 0;

  // expand paletted images to RGB
  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(png_ptr);

  // expand low-color images to RGB
  if(color_type == PNG_COLOR_TYPE_GRAY && bits_per_channel < 8)
    png_set_expand(png_ptr);

  // check for alpha channel
  if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);

  // convert 16-bit images to 8
  if(bits_per_channel == 16)
    png_set_strip_16(png_ptr);

  // expand grayscale images to RGB
  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  // perform gamma correction if the file requires it
  // gonna ignore this for now
//  double gamma = 0;
//  if(png_get_gAMA(png_ptr, info_ptr, &gamma))
//    png_set_gamma(png_ptr, 2.2, gamma);

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  int channels = (int)png_get_channels(png_ptr, info_ptr);

  Bitmap *volatile temp = new Bitmap(w, h, overscroll);

  if(interlace)
  {
    // interlaced images require a buffer the size of the entire image
    std::vector<png_byte> data(rowbytes * h);
    std::vector<png_bytep> row_pointers(h);

    for(int y = 0; y < h; y++)
      row_pointers[y] = &data[y * rowbytes];

    // read image all at once
    png_read_image(png_ptr, &row_pointers[0]);

    // convert image
    for(int y = 0; y < h; y++)
    {
      int *p = temp->row[y + overscroll] + overscroll;
      int xx = 0;

      png_bytep row = row_pointers[y];

      for(int x = 0; x < w; x++)
      {
        if(channels == 3)
        {
          *p++ = makeRgb(row[xx + 0] & 0xFF,
                         row[xx + 1] & 0xFF,
                         row[xx + 2] & 0xFF);
        }
        else if(channels == 4)
        {
           *p++ = makeRgba(row[xx + 0] & 0xFF,
                           row[xx + 1] & 0xFF,
                           row[xx + 2] & 0xFF,
                           row[xx + 3] & 0xFF);
        }

        xx += channels;
      }
    }
  }
  else
  {
    // non-interlace images can be read line-by-line
    std::vector<png_byte> linebuf(rowbytes);

    for(int y = 0; y < h; y++)
    {
      // read line
      png_read_row(png_ptr, &linebuf[0], 0);

      int *p = temp->row[y + overscroll] + overscroll;
      int xx = 0;

      // convert line
      for(int x = 0; x < w; x++)
      {
        if(channels == 3)
        {
          *p++ = makeRgb(linebuf[xx + 0] & 0xFF,
                         linebuf[xx + 1] & 0xFF,
                         linebuf[xx + 2] & 0xFF);
        }
        else if(channels == 4)
        {
           *p++ = makeRgba(linebuf[xx + 0] & 0xFF,
                           linebuf[xx + 1] & 0xFF,
                           linebuf[xx + 2] & 0xFF,
                           linebuf[xx + 3] & 0xFF);
        }

        xx += channels;
      }
    }
  }

  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);

  return temp;
}

Bitmap *File::loadPngFromArray(const unsigned char *array, int overscroll)
{
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
    return 0;

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, 0, 0);
    return 0;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    // pnglib does a goto here if there is an error
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return 0;
  }

  struct png_state state;
  state.array = array;
  state.pos = 0;

  png_uint_32 temp_w = 0;
  png_uint_32 temp_h = 0;
  int bits_per_channel = 0;
  int color_type = 0;
  int interlace_type = 0;
  int compression_type = 0;
  int filter_method = 0;

  png_set_read_fn(png_ptr, &state, (png_rw_ptr)pngReadFromArray);

//  png_init_io(png_ptr, in.get());
//  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &temp_w, &temp_h,
               &bits_per_channel, &color_type,
               &interlace_type, &compression_type, &filter_method);

  int w = temp_w;
  int h = temp_h;

  // check interlace mode
  bool interlace = png_set_interlace_handling(png_ptr) > 1 ? 1 : 0;

  // expand paletted images to RGB
  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_expand(png_ptr);

  // expand low-color images to RGB
  if(color_type == PNG_COLOR_TYPE_GRAY && bits_per_channel < 8)
    png_set_expand(png_ptr);

  // check for alpha channel
  if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_expand(png_ptr);

  // convert 16-bit images to 8
  if(bits_per_channel == 16)
    png_set_strip_16(png_ptr);

  // expand grayscale images to RGB
  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  // perform gamma correction if the file requires it
  // gonna ignore this for now
//  double gamma = 0;
//  if(png_get_gAMA(png_ptr, info_ptr, &gamma))
//    png_set_gamma(png_ptr, 2.2, gamma);

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  int channels = (int)png_get_channels(png_ptr, info_ptr);

  Bitmap *volatile temp = new Bitmap(w, h, overscroll);

  if(interlace)
  {
    // interlaced images require a buffer the size of the entire image
    std::vector<png_byte> data(rowbytes * h);
    std::vector<png_bytep> row_pointers(h);

    for(int y = 0; y < h; y++)
      row_pointers[y] = &data[y * rowbytes];

    // read image all at once
    png_read_image(png_ptr, &row_pointers[0]);

    // convert image
    for(int y = 0; y < h; y++)
    {
      int *p = temp->row[y + overscroll] + overscroll;
      int xx = 0;

      png_bytep row = row_pointers[y];

      for(int x = 0; x < w; x++)
      {
        if(channels == 3)
        {
          *p++ = makeRgb(row[xx + 0] & 0xFF,
                         row[xx + 1] & 0xFF,
                         row[xx + 2] & 0xFF);
        }
        else if(channels == 4)
        {
           *p++ = makeRgba(row[xx + 0] & 0xFF,
                           row[xx + 1] & 0xFF,
                           row[xx + 2] & 0xFF,
                           row[xx + 3] & 0xFF);
        }

        xx += channels;
      }
    }
  }
  else
  {
    // non-interlace images can be read line-by-line
    std::vector<png_byte> linebuf(rowbytes);

    for(int y = 0; y < h; y++)
    {
      // read line
      png_read_row(png_ptr, &linebuf[0], 0);

      int *p = temp->row[y + overscroll] + overscroll;
      int xx = 0;

      // convert line
      for(int x = 0; x < w; x++)
      {
        if(channels == 3)
        {
          *p++ = makeRgb(linebuf[xx + 0] & 0xFF,
                         linebuf[xx + 1] & 0xFF,
                         linebuf[xx + 2] & 0xFF);
        }
        else if(channels == 4)
        {
           *p++ = makeRgba(linebuf[xx + 0] & 0xFF,
                           linebuf[xx + 1] & 0xFF,
                           linebuf[xx + 2] & 0xFF,
                           linebuf[xx + 3] & 0xFF);
        }

        xx += channels;
      }
    }
  }

  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);

  return temp;
}

void File::save(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser fc;
  fc.title("Save Image");
  fc.filter("PNG Image\t*.png\n"
            "JPEG Image\t*.jpg\n"
            "Bitmap Image\t*.bmp\n"
            "Targa Image\t*.tga\n"
            "Java Array\t*.java\n");
  fc.options(Fl_Native_File_Chooser::PREVIEW);
  fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc.directory(save_dir);

  switch(fc.show())
  {
    case -1:
    case 1:
      return;
    default:
      getDirectory(save_dir, fc.filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc.filename());
  int ext = fc.filter_value();
  fl_filename_setext(fn, sizeof(fn), ext_string[ext]);

  if(fileExists(fn))
  {
    if(!Dialog::choice("Replace File?",
                      "Do you want to overwrite this file?"))
    {
      return;
    }
  }

  int ret = 0;
  
  switch(ext)
  {
    case 0:
      ret = File::savePng(fn);
      break;
    case 1:
      ret = File::saveJpeg(fn);
      break;
    case 2:
      ret = File::saveBmp(fn);
      break;
    case 3:
      ret = File::saveTarga(fn);
      break;
    case 4:
      ret = File::saveJava(fn);
      break;
    default:
      ret = -1;
  }

  if(ret < 0)
    errorMessage();
}

int File::saveBmp(const char *fn)
{
  FileSP out(fn, "wb");
  FILE *outp = out.get();
  if(!outp)
    return -1;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;
  int pad = w % 4;

  // BMP_FILE_HEADER
  writeUint8('B', outp);
  writeUint8('M', outp);
  writeUint32(14 + 40 + ((w * 3 + pad) * h), outp);
  writeUint16(0, outp);
  writeUint16(0, outp);
  writeUint32(14 + 40, outp);

  // BMP_INFO_HEADER
  writeUint32(40, outp);
  writeUint32(w, outp);
  writeUint32(-h, outp);
  writeUint16(1, outp);
  writeUint16(24, outp);
  writeUint32(0, outp);
  writeUint32(0, outp);
// FIXME eventually the program will set the dpi for use here
//       defaulting to 300 for now
  writeUint32(300 * 39.370079 + .5, outp);
  writeUint32(300 * 39.370079 + .5, outp);
  writeUint32(0, outp);
  writeUint32(0, outp);

  int *p = bmp->row[overscroll] + overscroll;
  std::vector<unsigned char> linebuf(w * 3 + pad);

  for(int y = 0; y < h; y++)
  {
    int xx = 0;
    for(int x = 0; x < w; x++)
    {
      linebuf[xx + 0] = (*p >> 16) & 0xff;
      linebuf[xx + 1] = (*p >> 8) & 0xff;
      linebuf[xx + 2] = *p & 0xff;
      p++;
      xx += 3;
    }

    for(int x = 0; x < pad; x++)
      linebuf[xx++] = 0;

    p += overscroll * 2;

    if(fwrite(&linebuf[0], 1, w * 3 + pad, outp) != (unsigned)(w * 3 + pad))
      return -1;
  }

  return 0;
}

int File::saveTarga(const char *fn)
{
  FileSP out(fn, "wb");
  FILE *outp = out.get();
  if(!outp)
    return -1;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  writeUint8(0, outp);
  writeUint8(0, outp);
  writeUint8(2, outp);
  writeUint16(0, outp);
  writeUint16(0, outp);
  writeUint8(0, outp);
  writeUint16(0, outp);
  writeUint16(0, outp);
  writeUint16(w, outp);
  writeUint16(h, outp);
  writeUint8(32, outp);
  writeUint8(32, outp);

  int *p = bmp->row[overscroll] + overscroll;
  std::vector<unsigned char> linebuf(w * 4);

  for(int y = 0; y < h; y++)
  {
    int xx = 0;

    for(int x = 0; x < w; x++)
    {
      linebuf[xx + 0] = (*p >> 16) & 0xff;
      linebuf[xx + 1] = (*p >> 8) & 0xff;
      linebuf[xx + 2] = *p & 0xff;
      linebuf[xx + 3] = (*p >> 24) & 0xff;
      p++;
      xx += 4;
    }

    p += overscroll * 2;

    if(fwrite(&linebuf[0], 1, w * 4, outp) != (unsigned)(w * 4))
      return -1;
  }

  return 0;
}

int File::savePng(const char *fn)
{
  Dialog::pngOptions();
  bool use_palette = Dialog::pngUsePalette();
  bool use_alpha = Dialog::pngUseAlpha();
  int alpha_levels = Dialog::pngAlphaLevels();
  float alpha_step = 255.0 / (alpha_levels - 1);
  Palette *pal = Project::palette.get();

  if(use_palette && use_alpha && pal->max * alpha_levels > 256)
  {
    Dialog::message("PNG Error",
                    "Not enough palette entries left for this "
                    "many alpha channel levels.");
    return 0;
  }

  std::vector<png_color> palette(256);
  std::vector<png_byte> trans(256);

  FileSP out(fn, "wb");
  if(!out.get())
    return -1;

  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if(!png_ptr)
    return -1;

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, 0);
    return -1;
  }

  if(setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return -1;
  }

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  png_init_io(png_ptr, out.get());

  if(use_palette)
  {
    if(use_alpha)
    {
      int index = 0;

      for(int j = 0; j < alpha_levels; j++)
      {
        int value = 255 - (int)(j * alpha_step);

        for(int i = 0; i < pal->max; i++)
        {
          rgba_type rgba = getRgba(pal->data[i]);

          palette[index].red = rgba.r;
          palette[index].green = rgba.g;
          palette[index].blue = rgba.b;
          trans[index] = value;
          index++;
        }
      }
    }
    else
    {
      for(int i = 0; i < pal->max; i++)
      {
        rgba_type rgba = getRgba(pal->data[i]);

        palette[i].red = rgba.r;
        palette[i].green = rgba.g;
        palette[i].blue = rgba.b;
      }
    }

    png_set_IHDR(png_ptr, info_ptr, w, h, 8,
                 PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_PLTE(png_ptr, info_ptr, &palette[0],
                 use_alpha ? pal->max * alpha_levels : pal->max);

    if(use_palette && use_alpha)
      png_set_tRNS(png_ptr, info_ptr, &trans[0], pal->max * alpha_levels, 0);
  }
  else
  {
    png_set_IHDR(png_ptr, info_ptr, w, h, 8,
                 use_alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
  }

  png_write_info(png_ptr, info_ptr);

  int bytes = 3;
  if(use_alpha)
    bytes = 4;
  if(use_palette)
    bytes = 1;

  std::vector<png_byte> linebuf(w * bytes);

  for(int y = 0; y < h; y++)
  {
    int *p = bmp->row[y + overscroll] + overscroll;

    for(int x = 0; x < w * bytes; x += bytes)
    {
      if(use_palette)
      {
        if(use_alpha)
          linebuf[x] = pal->lookup(*p) +
                       (int)(pal->max * ((255 - geta(*p)) / (int)alpha_step));
        else
          linebuf[x] = pal->lookup(*p);
      }
      else
      {
        linebuf[x + 0] = getr(*p); 
        linebuf[x + 1] = getg(*p); 
        linebuf[x + 2] = getb(*p); 
        if(use_alpha)
          linebuf[x + 3] = geta(*p); 
      }

      p++;
    }

    png_write_row(png_ptr, &linebuf[0]);
  }

  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  return 0;
}

int File::saveJpeg(const char *fn)
{
  FileSP out(fn, "wb");
  if(!out.get())
    return -1;

  // show quality dialog
  Dialog::jpegQuality();
  int quality = Dialog::jpegQualityValue();

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  std::vector<JSAMPLE> linebuf(w * 3);
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, out.get());

  cinfo.image_width = w;
  cinfo.image_height = h;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  int *p = bmp->row[overscroll] + overscroll;

  while(cinfo.next_scanline < cinfo.image_height)
  {
    for(int x = 0; x < w * 3; x += 3)
    {
      linebuf[x + 0] = getr(*p); 
      linebuf[x + 1] = getg(*p); 
      linebuf[x + 2] = getb(*p); 
      p++;
    }

    JSAMPROW row_pointer = &linebuf[0];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    p += overscroll * 2;
  }

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  return 0;
}

// FIXME this doesn't work yet
int File::saveJava(const char *fn)
{
  FileSP out(fn, "w");
  FILE *outp = out.get();
  if(!outp)
    return -1;

  Dialog::javaExport();
  int option = Dialog::javaExportOption();

  Bitmap *bmp = Project::bmp;
  int overscroll = Project::overscroll;
  int w = bmp->cw;
  int h = bmp->ch;

  fprintf(outp, "  static byte array[] = \n");
  fprintf(outp, "  {\n");

  int count = 0;

  if(option == 0)
  {
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x += 2)
      {
        int c1 = Project::palette->lookup(bmp->getpixel(x, y)) & 15;
        int c2 = Project::palette->lookup(bmp->getpixel(x + 1, y)) & 15;

        if(count == 0)
          fprintf(outp, "    ");

        fprintf(outp, "%d, ", (char)((c1 << 4) | c2));
        count++;

        if(count >= 10)
        {
          fprintf(outp, "\n");
          count = 0;
        }
      }
    }

    fprintf(outp, "\n  };\n");
  }
  else if(option == 1)
  {
    for(int y = bmp->ct; y <= bmp->cb; y++)
    {
      for(int x = bmp->cl; x <= bmp->cr; x++)
      {
        int c = Project::palette->lookup(bmp->getpixel(x, y));

        if(count == 0)
          fprintf(outp, "    ");

        fprintf(outp, "%d, ", (char)c);
        count++;

        if(count >= 10)
        {
          fprintf(outp, "\n");
          count = 0;
        }
      }
    }

    fprintf(outp, "\n  };\n");
  }

  return 0;
}
// callbacks for FLTK's file preview
Fl_Image *File::previewPng(const char *fn, unsigned char *header, int)
{
  if(!isPng(header))
    return 0;

  Bitmap *temp = 0;
  temp = loadPng(fn, 0);

  if(!temp)
    return 0;

  if(preview_bmp)
    delete preview_bmp;

  preview_bmp = temp;

  return new Fl_RGB_Image((unsigned char *)preview_bmp->data,
                          preview_bmp->w, preview_bmp->h, 4, 0);
}

Fl_Image *File::previewJpeg(const char *fn, unsigned char *header, int)
{
  if(!isJpeg(header))
    return 0;

  Bitmap *temp = 0;
  temp = loadJpeg(fn, 0);

  if(!temp)
    return 0;

  if(preview_bmp)
    delete preview_bmp;

  preview_bmp = temp;

  return new Fl_RGB_Image((unsigned char *)preview_bmp->data,
                          preview_bmp->w, preview_bmp->h, 4, 0);
}

Fl_Image *File::previewBmp(const char *fn, unsigned char *header, int)
{
  if(!isBmp(header))
    return 0;

  Bitmap *temp = 0;
  temp = loadBmp(fn, 0);

  if(!temp)
    return 0;

  if(preview_bmp)
    delete preview_bmp;

  preview_bmp = temp;

  return new Fl_RGB_Image((unsigned char *)preview_bmp->data,
                          preview_bmp->w, preview_bmp->h, 4, 0);
}

Fl_Image *File::previewTarga(const char *fn, unsigned char *, int)
{
  if(!isTarga(fn))
    return 0;

  Bitmap *temp = 0;
  temp = loadTarga(fn, 0);

  if(!temp)
    return 0;

  if(preview_bmp)
    delete preview_bmp;

  preview_bmp = temp;

  return new Fl_RGB_Image((unsigned char *)preview_bmp->data,
                          preview_bmp->w, preview_bmp->h, 4, 0);
}

Fl_Image *File::previewGimpPalette(const char *fn, unsigned char *header, int)
{
  if(!isGimpPalette(header))
    return 0;

  SP<Palette> temp_pal = new Palette();

  temp_pal->load(fn);
  if(temp_pal->max == 0)
    return 0;

  temp_pal->draw(pal_preview);

  return new Fl_RGB_Image((unsigned char *)pal_preview->bitmap->data,
                          96, 96, 4, 0);
}

// load a palette using the file chooser
void File::loadPalette()
{
  Fl_Native_File_Chooser fc;
  fc.title("Load Palette");
  fc.filter("GIMP Palette\t*.gpl\n");
  fc.options(Fl_Native_File_Chooser::PREVIEW);
  fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
  fc.directory(pal_load_dir);

  switch(fc.show())
  {
    case -1:
    case 1:
      return;
    default:
      getDirectory(pal_load_dir, fc.filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc.filename());

  FileSP in(fn, "r");
  if(!in.get())
    return;

  unsigned char header[12];
  if(fread(&header, 1, 12, in.get()) != 12)
  {
    errorMessage();
    return;
  }

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

// save a palette using the file chooser
void File::savePalette()
{
  Fl_Native_File_Chooser fc;
  fc.title("Save Palette");
  fc.filter("GIMP Palette\t*.gpl\n");
  fc.options(Fl_Native_File_Chooser::PREVIEW);
  fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
  fc.directory(pal_save_dir);

  switch(fc.show())
  {
    case -1:
    case 1:
      return;
    default:
      getDirectory(pal_save_dir, fc.filename());
      break;
  }

  char fn[256];
  strcpy(fn, fc.filename());
  fl_filename_setext(fn, sizeof(fn), ".gpl");

  if(fileExists(fn))
  {
    if(!Dialog::choice("Replace File?",
                      "Do you want to overwrite this file?"))
    {
      return;
    }
  }
  
  if(Project::palette->save(fn) < 0)
  {
    errorMessage();
    return;
  }
}

// convert special characters from drag n' drop path/filename string
void File::decodeURI(char *s)
{
  unsigned int c;
  int len = strlen(s);

  for(int i = 0; i < len - 2; i++)
  {
    if(s[i] == '%')
    {
      if(sscanf(&s[i + 1], "%2X", &c) != 1)
        break;

      s[i] = c;

      for(int j = 0; j < len - (i + 2); j++)
        s[i + 1 + j] = s[i + 3 + j];

      len -= 2;
    }
  }
}

// extract directory from a path/filename string
void File::getDirectory(char *dest, const char *src)
{
  strcpy(dest, src);

  int len = strlen(dest);
  if(len < 2)
    return;

  for(int i = len - 1; i > 0; i--)
  {
    if(dest[i - 1] == '/')
    {
      dest[i] = '\0';
      break;
    }
  }
}

