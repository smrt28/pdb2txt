#include "ptr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char * unbreakable[] =  {
        "v", "k", "s", "a", "u", "i", "o", "z",
        "na", "ve", "do", "za", "se", "po", "ke", "zpoza", "pro", "nad", "pod", "pred",
        NULL
    };



int is_unbreakable(char *buf, int ofs)
{
    if (buf[ofs] != ' ')
        return 0;

    ofs = (ofs + 15) % 16;

     int l, o, i;
     const char *c;
     const char **u;

    for (u=unbreakable;*u;u++)
    {
        c = *u;
        l = strlen(c) - 1;
        o = ofs;

        for (;l>=0;)
        {
            if (!isalpha(buf[o]))
                break;

            if (tolower(buf[o]) != c[l])
                break;
           
            o--; l--;
            if (o < 0)
                o += 16;
        }
        if (l >= 0)
            continue;

        if (buf[o] == ' ')
            return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
  int i;
  int lb_offs = 0;
  char lb[16];
  memset(lb, 0, sizeof(lb));

  for(;;)
  {
    int rd = fgetc(stdin);
    if (rd == EOF)
        return 0;

    lb_offs++;
    lb_offs %= 16;
    lb[lb_offs] = rd;
    switch(rd)
    {
        case '"':
            printf("&quot;");
            break;
        case '&':
            printf("&amp;");
            break;
        case '>':
            printf("&gt;");
            break;
        case '<':
            printf("&lt;");
            break;
        default:
            {
                if (is_unbreakable(lb, lb_offs))
                    printf("&nbsp;");
                else
                    putchar(rd);
            }
            break;
    }
  } 
  return 0;
}

