#include "MemoryManager.h"
//#include <iostream>
//#include <string.h>

#define ADD_HEADER 0xC0
#define REMOVE_HEADER 0x3F
#define MEMORY_16K 16*1024

namespace MemoryManager
{
  // IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
  //
  // This is the only static memory that you may use, no other global variables may be
  // created, if you need to save data make it fit in MM_pool
  //
  // IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT


  const int MM_POOL_SIZE = 65536;
  char MM_pool[MM_POOL_SIZE];
  
  //forward declarations
  void getSizeInt(char* firstHeader, char* secondHeader, int *size );
  void memcpy(void *dest, void *src, const unsigned int n);
  
  // Initialize set up any data needed to manage the memory pool
  void initializeMemoryManager(void)
  {
    for(int i=0; i< MM_POOL_SIZE; i++)
      MM_pool[i]=0x0;
  }

  // return a pointer inside the memory pool
  // If no chunk can accommodate aSize call onOutOfMemory()
  /*
  Implementation 
  ---------------
  This solution byte-aligns the requested space in bytes and includes a 2 byte header
  The header contains a signature denoting an occupied block in 2 bits and the rest of 
  14 bits represent the size of the allocated memory block in bytes.
  The scanner jumps uses the block size as a cue to jump to the next block. If the next block
  is a free block, its size is compared with the requested size, if a match then it is allocated.
  
  A typical example :
  1) Request is 7 bytes of memory.
  2) The scanner starts with the byte. It compares the first byte to see if it a free block.
  3) If yes, then  it is checked whether it can accomodate 7 bytes of memory and 2 bytes of header.
  4) After that, its first two bits are made as the signature ADD_HEADER (given as macro) and
      rest of 14 bits are set as 7.
  5) The pointer to the third free byte (as the first two bytes are HEADER and size)is returned back. 
  6) Had the block not been free, the scanner would have figured out the next block to jump to by adding size of
    allocated block to the current pointer position.
  7) If a free block is not found, or a free block with exact size is not found, an out of memory error is generated.

  Deallocation traversal is similar to that of allcation. Only in deallocation, the scanner finds out the size of the 
  allocated block from the first 2 bytes (using REMOVE_HEADER) and makes the size+Headers bytes to 0x0.

  Results :
  Free memory = 65536
  Free memory = 65269

  Scope of optimization : 
  A freed block can store the offset address of the next free block. Furthermore, a dedicated 2 bytes in the beginning
  of the memory pool can be used to keep offset of the very first free block and should be updated accordingly. This will
  allow the scanner to skip traversal and directly jump between free blocks. This implementation will require more space but
  will make allocation faster.

  Citations :
  Used memcpy template from geeksforgeeks.org as using additional header files is not allowed.
  Discussion with roommate led to the addition of a size as part of header. My previous idea was to have a start header
  and an end header and keep offset of addresses of free blocks as mentioned in the previous section. Removed it as it was
  wasting a lot of space and I decided to priotise space over speedup.

  */
  void* allocate(int aSize)
  {
    
    if(aSize > MEMORY_16K){
      onOutOfMemory();
    }
    char *p = MM_pool;
    char firstHeader = 0x0;
    char secondHeader = 0x0;

    char *temp = (char *)&aSize;   //int is of 4 bytes, char is of 1
    firstHeader = ADD_HEADER | temp[1];   //char pointer points to LSB of int ie. last byte
    secondHeader = temp[0];

    //find right free memory block
    char header_one, header_two;
    int iter_count = 0;
    while(iter_count < MM_POOL_SIZE){
      
      if(*p == 0x0){  // empty block is found
        if(*(p+(aSize-1 + 2)) == 0x0){ //empty block size is enough to store
          break;
        }
        else{  //not enough free block, search for another
          p++;
          iter_count++;
          continue;
        }
      }
      header_one = p[0];
      header_two = p[1];
      header_one = REMOVE_HEADER & header_one;   //remove 11 which is a signature
      int size = 0;
      getSizeInt(&header_one, &header_two, &size);
      p = p+size+2;
      iter_count = iter_count + size+2;
      
    }

    //if there is no free memory
    if(iter_count == MM_POOL_SIZE){
      onOutOfMemory();
    }
    
    if(iter_count+(aSize-1 + 2) >= MM_POOL_SIZE){
      onOutOfMemory();
    }

    p[0] = firstHeader;
    p[1] = secondHeader;
    return p+2;

    //return ((void*) 0);
  }

  // Free up a chunk previously allocated
  void deallocate(void* aPointer)
  {
    char *p = (char *)aPointer;
    if (p == 0x0){
      onIllegalOperation("Error on trying to deallocate a free memory/null pointer");
    }


    char *firstHeader = p - 2;
    char *secondHeader = p - 1;
    *firstHeader = REMOVE_HEADER & *firstHeader;   //remove 11 which is a signature
    int size=0;
    getSizeInt( firstHeader, secondHeader, &size );
    char *temp;
    for( int i=0; i< (size+2); i=i+1 ){
      temp = &MM_pool[i];
      *temp = 0x0;
    }


  }

  //---
  //--- support routines
  //--- 

  //return a size integer from parts of header in format of char
  void getSizeInt(char* firstHeader, char* secondHeader, int *size ){
    char tempSize[4];
    tempSize[3] = 0x0;
    tempSize[2] = 0x0;
    tempSize[1] = *firstHeader;
    tempSize[0] = *secondHeader;
    memcpy(size, tempSize, 4);
  }
  // Providing own implementation of memcpy as adding additional header was not allowed
  void memcpy(void *dest, void *src, const unsigned int n){ 
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
   for (int i=0; i<n; i++)
       cdest[i] = csrc[i];
  }

  // Will scan the memory pool and return the total free space remaining
  int freeRemaining(void)
  {
    char *p = MM_pool;
    int count = 0;
    int iter_count = 0;
    char header_one = 0;
    char header_two = 0;

    while(iter_count < MM_POOL_SIZE){
      if(*p == (char)0x0){
        count++;
        p++;
        iter_count++;
        continue;
      }else{
      }
      header_one = p[0];
      header_two = p[1];
      header_one = REMOVE_HEADER & header_one;   //remove 11 which is a signature
      int size = 0;
      getSizeInt(&header_one, &header_two, &size);
      p = p+size+2;
      iter_count = iter_count + size+2;
    }

    return count;
  }

  // Will scan the memory pool and return the largest free space remaining
  int largestFree(void)
  {
    char *p = MM_pool;
    int largest_count = 0;
    int temp_count = 0;
    int iter_count = 0;
    char header_one = 0;
    char header_two = 0;

    while(iter_count < MM_POOL_SIZE){
      
      if(*p == 0x0){
        temp_count++;
        p++;
        iter_count++;
        continue;
      }
      if(temp_count > largest_count){
        largest_count = temp_count;
        temp_count = 0;
      }

      header_one = p[0];
      header_two = p[1];
      header_one = REMOVE_HEADER & header_one;   //remove 11 which is a signature
      int size = 0;
      getSizeInt(&header_one, &header_two, &size);
      p = p+size;
      iter_count = iter_count + size+2;
    }

    if(temp_count > largest_count){
        largest_count = temp_count;
        temp_count = 0;
      }

    return largest_count;
  }

  // will scan the memory pool and return the smallest free space remaining
  int smallestFree(void)
  {
    char *p = MM_pool;
    int smallest_count = 0;
    int temp_count = 0;
    int iter_count = 0;
    char header_one = 0;
    char header_two = 0;
    while(iter_count < MM_POOL_SIZE){
      
      if(*p == 0x0){
        temp_count++;
        p++;
        iter_count++;
        continue;
      }
      if(temp_count < smallest_count){
        smallest_count = temp_count;
        temp_count = 0;
      }

      header_one = p[0];
      header_two = p[1];
      header_one = REMOVE_HEADER & header_one;   //remove 11 which is a signature
      int size = 0;
      getSizeInt(&header_one, &header_two, &size);
      p = p+size;
      iter_count = iter_count + size+2;
    }

    if(temp_count < smallest_count){
        smallest_count = temp_count;
        temp_count = 0;
      }

    return smallest_count;

  }
 }