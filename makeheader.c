#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// reads files from a directory and creates a header file
// used for embedding PNG images
// example: makeheader src/Images.H images/*.png

int main(int argc, char **argv)
{
  FILE *in, *out;

  int i = 1;
  char s[4096];

  out = fopen(argv[1], "w");

  while(i++ < argc - 1)
  {
    strcpy(s, argv[i]);

    for(int j = 0; j < strlen(s); j++)
      if(s[j] == '/' || s[j] == '.')
        s[j] = '_';

    fprintf(out, "unsigned char const %s [] = \n", s);
    fprintf(out, "{");

    in = fopen(argv[i], "r");

    int pos = 0;
    int c = fgetc(in);

    if(c == EOF)
    {
      fclose(in);
      fclose(out);
      printf("Error.\n");

      return 0;
    }

    while(1)
    {
      if((pos % 8) == 0)
        fprintf(out, "\n  ");

      pos++;
      fprintf(out, "0x%02x", c); 
      c = fgetc(in);

      if(c == EOF)
        break;
      else
        fprintf(out, ", ");
    }

    fprintf(out, "\n};\n\n");
    fclose(in);
  }

  fclose(out);

  return 0;
}

