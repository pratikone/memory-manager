# memory-manager
A simple memory manager written in C++ (using C semantics) for Insomniac games interview.
### How it works
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

  For a free block, first two bits of header will be `00b` and for a occupied block, it would be `11b` 
```
  Results :
  Free memory = 65536
  Free memory = 65269
```
  #### Scope of optimization : 
  A freed block can store the offset address of the next free block. Furthermore, a dedicated 2 bytes in the beginning
  of the memory pool can be used to keep offset of the very first free block and should be updated accordingly. This will
  allow the scanner to skip traversal and directly jump between free blocks. This implementation will require more space but
  will make allocation faster.

  #### Citations :
  Used memcpy template from geeksforgeeks.org as using additional header files is not allowed.
  Discussion with roommate led to the addition of a size as part of header. My previous idea was to have a start header
  and an end header and keep offset of addresses of free blocks as mentioned in the previous section. Removed it as it was
  wasting a lot of space and I decided to priotise space over speedup.
