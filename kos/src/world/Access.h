/******************************************************************************
    Copyright © 2012-2015 Martin Karsten

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#ifndef _Access_h_
#define _Access_h_

#include "runtime/SynchronizedArray.h"
#include "kernel/Output.h"
#include "devices/Keyboard.h"

#include <map>
#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h> // SEEK_SET, SEEK_CUR, SEEK_END

class Access : public SynchronizedElement {
public:
/*A3*/
  virtual ~Access() {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o) { return -EBADF; }
  virtual ssize_t pwrite(off_t o, size_t nbyte, void *buf) { return -EBADF; }
  virtual ssize_t read(void *buf, size_t nbyte) { return -EBADF; }
  virtual ssize_t write(const void *buf, size_t nbyte) { return -EBADF; }
  virtual off_t lseek(off_t o, int whence) { return -EBADF; }
};
/*A3*/

struct RamFile {
  vaddr vma;
  paddr pma;
  size_t size;
  RamFile(vaddr v, paddr p, size_t s) : vma(v), pma(p), size(s) {}
};

struct MyRamFile {
  vaddr vma; //Laura: this needs to be changed to the virtual address
  paddr pma;
  size_t size;
  MyRamFile(vaddr v, paddr p, size_t s) : vma(v), pma(p), size(s) {}
};
//Laura: this is storing the name of the file and the info on how to access it.

extern map<string,RamFile> kernelFS; 
//Laura gonna keep this and create my own copy. order: virtual memory acess, physical memory adress, size.
//if you are doing a link list need the head in here to
//if doing an index there needs to be an array in here.


extern map<string,MyRamFile> myKernelFS;
extern char* savedMemory;
//initalize savedMemory here so that it is accessible where ever myKernelFS is accessable

class FileAccess : public Access {
  SpinLock olock;
  off_t offset;
  const RamFile &rf;
public:

/*A3*/
  FileAccess(const RamFile& rf) : offset(0), rf(rf) {}
  virtual ssize_t pread(void *buf, size_t nbyte, off_t o);
  virtual ssize_t pwrite(off_t o, size_t nbyte, void *buf);
  virtual ssize_t read(void *buf, size_t nbyte);
  virtual ssize_t write( void *buf, size_t nbyte);
  virtual off_t lseek(off_t o, int whence);
};
/*A3*/
//Laura: everytime i ask it to read n characters the offset is increased by n 
//for a new instance the offset is 0.
//this needs the ram file to go in. so if you have an inode it needs to get passed in here

class KernelOutput;
class OutputAccess : public Access {
  KernelOutput& ko;
public:
  OutputAccess(KernelOutput& ko) : ko(ko) {}
  virtual ssize_t write(const void *buf, size_t nbyte) {
    return ko.write(buf, nbyte);
  }
};

extern Keyboard keyboard;
class InputAccess : public Access {
public:
  InputAccess() {}
  virtual ssize_t read(void *buf, size_t nbyte) {
    if (nbyte == 0) return 0;
    Keyboard::KeyCode k = keyboard.read();
    char* s = (char*)buf;
    for (size_t r = 0; r < nbyte; r += 1) {
      s[r] = k;
      if (!keyboard.tryRead(k)) return r+1;
    }
    return nbyte;
  }
};

#endif /* _Access_h_ */
