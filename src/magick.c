#define _GNU_SOURCE
#include <wand/MagickWand.h>


static void
ThrowWandException(MagickWand *wand)                    \
{
  char *description;

  ExceptionType severity;

  description=MagickGetException(wand,&severity);
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description);
  description=(char *) MagickRelinquishMemory(description);
  exit(-1);
}

void
magick_dump (int rows, int cols, char *file, void *pixels)
{
  MagickBooleanType status;
  MagickWand *magick_wand;

  MagickWandGenesis();

  magick_wand = NewMagickWand();

  status = MagickConstituteImage(magick_wand,
                                 cols,       // const size_t columns,
                                 rows,          // const size_t rows,
                                 "BGRA",        // const char *map,
                                 CharPixel,     // const StorageType storage,
                                 pixels);       // void *pixels)
  if (status == MagickFalse)
    ThrowWandException(magick_wand);

  status = MagickWriteImages(magick_wand, file, MagickTrue);

  if (status == MagickFalse)
    ThrowWandException(magick_wand);

  magick_wand = DestroyMagickWand(magick_wand);
  
  MagickWandTerminus();
}
