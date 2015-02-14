/* The code is based on Haali Reader. */

#include "ptr.h"
#include "PDBFile.h"

#include <string.h>
#include "types.h"

// pdb header
struct PDBHdr {
  char	  name[32];
  u16	  attr;
  u16	  ver;
  u32	  ctime;
  u32	  mtime;
  u32	  btime;
  u32	  mnum;
  u32	  appinfoid;
  u32	  aortinfoid;
  char	  type[4];
  char	  creator[4];
  u32	  idseed;
  u32	  nextreclist;
  u16	  numrec;
};
#define	PDBHDRSIZE  78

// record 0
struct PDBRec0 {
  u16	  ver;
  u16	  res1;
  u32	  usize;
  u16	  nrec;
  u16	  rsize;
  u32	  res2;
};
#define	PDBR0SIZE 16

static size_t  dword(const size_t& dw) { // convert from BE
  u8	*b=(u8*)&dw;
  return ((size_t)b[0]<<24)|((size_t)b[1]<<16)|((size_t)b[2]<<8)|b[3];
}

static u16   word(const u16& w) { // convert from BE
  u8	*b=(u8*)&w;
  return ((u16)b[0]<<8)|b[1];
}

bool  PDBFile::CheckPDB(RFile *fp,PDBHdr& hdr,PDBRec0& r0) {
  // we want to access original RFile methods, not the overridden
  // ones
  fp->RFile::seek(0);
  if (fp->RFile::read2(&hdr,PDBHDRSIZE)!=PDBHDRSIZE)
    return false;
  if (memcmp(hdr.type,"TEXt",4) || memcmp(hdr.creator,"REAd",4))
    return false;
  size_t	  off0;
  if (fp->RFile::read2(&off0,4)!=4)
    return false;
  fp->RFile::seek(dword(off0));
  if (fp->RFile::read2(&r0,PDBR0SIZE)!=PDBR0SIZE)
    return false;
    /*
  if (word(r0.ver)!=1 && word(r0.ver)!=2 && word(r0.ver)!=257 && word(r0.nrec)>0)
    return false;
    */
  return true;
}

static size_t	scan_size(u8 *src,size_t len) {
  size_t	  ulen=0;

  while (len--) {
    if (*src>=1 && *src<=8) {
      size_t k=*src++;
      while (k-- && len--)
	src++,ulen++;
    } else if (*src<=0x7f)
      src++,ulen++;
    else if (*src>=0xc0)
      src++,ulen+=2;
    else if (len) {
      size_t k=*src++;
      k<<=8; k|=*src++;
      --len;
      ulen+=(k&7)+3;
    }
  }
  return ulen;
}

PDBFile::PDBFile(const std::string& fn) :
  RFile(fn), m_length(0), m_ptr(0)
{
  PDBHdr    hdr;
  PDBRec0   r0;
  if (Reopen() && CheckPDB(this,hdr,r0)) {
    m_name = hdr.name;
    
    m_comp = 1;
    if (word(r0.ver)==1)
        m_comp = 0;
//    m_comp=word(r0.ver)==2;

    m_rsz=word(r0.rsize);
    size_t nr=word(r0.nrec);
    m_blocks=Buffer<Rec>(nr);
    // fill in records table
    RFile::seek(PDBHDRSIZE+8);
    for (size_t j=0;j<nr;++j) {
      size_t   off[2];
      if (RFile::read2(off,8)!=8)
	goto fail;
      m_blocks[j].off=dword(off[0]);
      if (j>0) {
	m_blocks[j-1].csize=m_blocks[j].off-m_blocks[j-1].off;
	if (m_blocks[j-1].csize>m_rsz)
	  goto fail;
      }
    }
    if (nr+1<word(hdr.numrec)) { // minus rec0
      size_t   off[2];
      if (RFile::read2(off,8)!=8)
	goto fail;
      m_blocks[nr-1].csize=dword(off[0])-m_blocks[nr-1].off;
    } else
      m_blocks[nr-1].csize=(size_t)RFile::size()-m_blocks[nr-1].off;
    if (m_blocks[nr-1].csize>m_rsz)
      goto fail;
    if (m_comp) { // compressed
      Buffer<u8>  tmp(m_rsz);
      size_t	    uoff=0;
      for (int i=0;i<m_blocks.size();++i) {
	RFile::seek(m_blocks[i].off);
	if (RFile::read2(tmp,m_blocks[i].csize)!=m_blocks[i].csize)
	  goto fail;
	m_blocks[i].usize=scan_size(tmp,m_blocks[i].csize);
	if (m_blocks[i].usize>m_rsz)
	  goto fail;
	m_blocks[i].uoff=uoff;
	uoff+=m_blocks[i].usize;
      }
      m_length=uoff;
    } else { // uncompressed
      for (int i=0;i<m_blocks.size();++i) {
	m_blocks[i].usize=m_blocks[i].csize;
	m_blocks[i].uoff=m_length;
	m_length+=m_blocks[i].usize;
      }
    }
    return;
  }
fail:
  return;
}

static size_t	pdb_decompress(u8 *source,size_t srclen,u8 *dest,size_t destlen)
{
  u8	    *se=source+srclen;
  u8	    *de=dest+destlen;
  u8	    *dd=dest;

  while (source<se && dest<de) {
    size_t c=*source++;
    if (c>=1 && c<=8) { // copy
      while (c-- && source<se && dest<de)
	*dest++=*source++;
    } else if (c<=0x7f) // this char
      *dest++=(u8)c;
    else if (c>=0xc0) { // space + c&0x7f
      *dest++=' ';
      if (dest<de)
	*dest++=(u8)c&0x7f;
    } else if (source<se) { // copy from decoded buf
      c=(c<<8)|*source++;
      int k=(c&0x3fff)>>3;
      c=3+(c&7);
      if (dest-k<dd || dest+c>de) // invalid buffer
	break;
      while (c-- && dest<de) {
	*dest=dest[-k];
	++dest;
      }
    }
  }
  return dest-dd;
}

void  PDBFile::seek(size_t pos) {
  if (pos>=m_length)
    m_ptr=m_length;
  else
    m_ptr=pos&BMASK;
}

size_t PDBFile::read(void *buf) {
  if (m_ptr>=m_length)
    return 0;
  // ok, figure what block is needed
  int i;
  for (i=0;i<m_blocks.size();++i)
    if (m_ptr>=m_blocks[i].uoff && m_ptr<m_blocks[i].uoff+m_blocks[i].usize)
      goto found;
  return 0;
found:
  Buffer<u8>	  tmp(m_rsz),tmp2(m_rsz);
  u8		  *p=(u8*)buf;
  size_t		  n=BSZ;
  while (i<m_blocks.size() && n>0) {
    RFile::seek(m_blocks[i].off);
    if (m_comp) {
      if (RFile::read2(tmp2,m_blocks[i].csize)!=m_blocks[i].csize)
	return 0;
      if (pdb_decompress(tmp2,m_blocks[i].csize,tmp,m_rsz)!=m_blocks[i].usize)
	return 0;
    } else {
      if (RFile::read2(tmp,m_blocks[i].csize)!=m_blocks[i].csize)
	return 0;
    }
    // now we have a decompressed block in tmp
    size_t   hb=m_blocks[i].uoff+m_blocks[i].usize-m_ptr;
    if (hb>n)
      hb=n;
    memcpy(p,tmp+m_ptr-m_blocks[i].uoff,hb);
    p+=hb;
    m_ptr+=hb;
    n-=hb;
    ++i;
  }
  return BSZ-n;
}

bool  PDBFile::IsPDB(RFile *fp) {
  PDBHdr    hdr;
  PDBRec0   r0;
  bool ret=CheckPDB(fp,hdr,r0);
  fp->seek(0);
  return ret;
}
