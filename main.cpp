#include "ptr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RFile.h"
#include "PDBFile.h"


int main(int argc, char **argv)
{
  if (argc != 2)
  {
      fprintf(stderr, "no input *.pdb file\n");
      return 1;
  }
  PDBFile pdb(argv[1]);
  size_t sz = pdb.size();
  if (sz == 0)
  {
      fprintf(stderr, "error while reading the file!\n");
      return 2;
  }
  std::string name;
  pdb.GetName(name);


  fflush(stdout);
  unsigned char buf[sz+1];
  size_t rd;
  int i;
  
  do
  {
    rd = pdb.read(buf);

    for (i=0;i<rd;i++)
        putchar(buf[i]);

  } while(rd);
  return 0;
}

