#include "rendera.h"

extern Gui *gui;

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

Fl_Image *preview_pal(const char *fn, unsigned char *header, int len)
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

  pal->draw(gui->pal_preview);

  Fl_RGB_Image *image = new Fl_RGB_Image((unsigned char *)gui->pal_preview->bitmap->data, 96, 96, 4, 0);

  delete pal;
  return image;
}

