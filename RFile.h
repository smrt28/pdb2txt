#ifndef __rfile_h_
#define __rfile_h_

#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


class RFile  
{
public:
  static int  BSZ,BMASK; // block size, must be a power of two

  RFile(const std::string& filename);

  virtual ~RFile() ;

  // generic file operations
  virtual size_t	  size();
  virtual size_t	  read(void *buf);
  virtual void	  seek(size_t pos);

  // compression
  virtual std::string CompressionInfo() { return "None"; }

  static void	  InitBufSize();

  // RFile helpers
  bool		  Reopen();
  void		  ShowError();
  size_t		  read2(void *buf, size_t size);

protected:
  int	 m_fh;
  std::string	  m_fn;
  size_t m_ptr;

  bool		  m_didreopen;
  bool		  m_diderror;
  void seek2(off_t  pos, int how);
  size_t		  pos();
};


#endif 

