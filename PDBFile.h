/* The code is based on Haali Reader. */
#ifndef __pdbfile_h_
#define __pdbfile_h_

#include "RFile.h"

class PDBFile : public RFile
{
public:
  PDBFile(const std::string &fn);

  // generic file operations
  virtual size_t size() { return m_length; }
  virtual size_t read(void *buf);
  virtual void seek(size_t pos);

  // check if this is a pdb file
  static bool	  IsPDB(RFile *fp);
protected:
  size_t		  m_length;
  size_t		  m_ptr;
  size_t		  m_rsz;
  bool		  m_comp;
  
  struct Rec {
    size_t   usize;
    size_t   uoff;
    size_t   off;
    size_t   csize;
  };
  
  Buffer<Rec>	  m_blocks;

  static bool CheckPDB(RFile *fp, struct PDBHdr &, struct PDBRec0 &);
  int	findblock(size_t uoff);

  std::string m_name;
public:
   void GetName(std::string &n) { n = m_name; }
};

#endif

