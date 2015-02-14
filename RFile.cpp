/* The code is based on Haali Reader. */

#include "ptr.h"
#include "RFile.h"
#include <sys/stat.h>

int RFile::BSZ=16384;
int RFile::BMASK=~16383;



RFile::RFile(const std::string& filename) :
      m_fh(-1),
      m_fn(filename), 
      m_didreopen(false), 
      m_diderror(false),
      m_ptr(0) 
{
}

RFile::~RFile() 
{
  if (m_fh != -1)
    close(m_fh);
}

void  RFile::InitBufSize() {
  int rfbs=16384;
  int fbs=8192;
  while ((fbs<<1)<=rfbs)
    fbs<<=1;
  BSZ=fbs;
  BMASK=~(fbs-1);
}

bool  RFile::Reopen() {
  if (m_didreopen)
    return false;
  if (m_fh != -1) {
    close(m_fh);
    m_fh = -1;
  }

  int fh = open(m_fn.c_str(), O_RDONLY);

  if (fh == -1) 
  {
    m_didreopen = true;
    return false;
  }
  m_fh = fh;
  lseek(fh, 0, SEEK_SET);
  return true;
}

// to avoid dealing with exceptions we access the file handle directly
size_t RFile::size() 
{
  if (m_fh == -1)
    return 0;
  struct stat st;
  if (fstat(m_fh, &st)==-1)
    return 0;
  return st.st_size;
}



size_t RFile::read2(void *buf, size_t size) 
{
  ssize_t ret;
  ret = ::read(m_fh, buf, size);
  if (ret >= 0)
  {
    m_ptr += ret;
    return ret;
  }
  return 0;
}

size_t RFile::read(void *buf) { return read2(buf,BSZ); }

void  RFile::seek2(off_t  pos, int how) 
{
  off_t np = lseek(m_fh, pos, how);
  
  m_ptr = np;
}

void  RFile::seek(size_t pos) { seek2(pos, SEEK_SET); }

size_t RFile::pos() {
  return m_ptr;
}



