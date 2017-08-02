#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <string.h>
#include "MemoryManager.h"

int main(void)
{
  using namespace MemoryManager;

  initializeMemoryManager();

  long* int_pointer;
  char* string_pointer;
  char* string_pointer2;
  char* string_pointer3;

  std::cout << "Free memory = " << freeRemaining() << std::endl;

  int_pointer = (long *) allocate(sizeof(long));
  string_pointer = (char*) allocate(255);
  *int_pointer = 0xDEADBEEF;
  strcpy(string_pointer,"It was the best of times, it was the worst of times");

  std::cout << "Free memory = " << freeRemaining() << std::endl;

  deallocate(int_pointer);
  deallocate(string_pointer);
}

namespace MemoryManager
{
  void onOutOfMemory(void)
  {
    std::cerr << "Memory pool out of memory" << std::endl;
    exit( 1 );
  }

  // call for any other error condition, providing meaningful error messages are appreciated
  void onIllegalOperation(const char* fmt,...)
  {
    if ( fmt == NULL )
    {
      std::cerr << "Unknown illegal operation" << std::endl;
      exit( 1 );
    }
    else
    {
      char	buf[8192];

      va_list argptr;
      va_start (argptr,fmt);
      vsprintf (buf,fmt,argptr);
      va_end (argptr);

      std::cerr << "Illegal operation: \"" << buf << "\"" << std::endl;
      exit( 1 );
    }
  }
}

