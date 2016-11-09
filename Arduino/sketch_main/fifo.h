#ifndef _H_FIFO
#define _H_FIFO

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Buffer data type
typedef char fifo_data_t;

struct fifo_t {
  fifo_data_t *startPtr; // Points on buffer
  fifo_data_t *endPtr;   // Points on last element in buffer
  fifo_data_t *readPtr;  // Points on next byte to read
  fifo_data_t *writePtr; // Points on next byte to write
  size_t sz;      // Buffer size
};


// FWD declaration
struct fifo_t fifo_init(size_t sz);
void fifo_release(struct fifo_t *fifo);
size_t fifo_get_used_bytes(struct fifo_t *fifo);
size_t fifo_get_free_bytes(struct fifo_t *fifo);
size_t fifo_put_data(struct fifo_t *fifo, fifo_data_t *data, size_t sz);
size_t fifo_get_data(struct fifo_t *fifo, fifo_data_t *buff, size_t sz);
size_t fifo_get_bytes_until_end(struct fifo_t *fifo, fifo_data_t *ptr);
size_t fifo_get_until(struct fifo_t *fifo, fifo_data_t* buff, fifo_data_t delimit, size_t maxCnt);


struct fifo_t fifo_init(size_t sz)
{
  struct fifo_t fifo;
  fifo.startPtr = (fifo_data_t*)malloc(sz * sizeof(fifo_data_t));
  fifo.endPtr = fifo.startPtr + sz - 1;
  fifo.readPtr = fifo.startPtr;
  fifo.writePtr = fifo.startPtr;
  fifo.sz = sz;

  return fifo;
}

void fifo_release(struct fifo_t *fifo) {
  free(fifo->startPtr);
  fifo->startPtr = 0;
  fifo->endPtr = 0;
  fifo->readPtr = 0;
  fifo->writePtr = 0;
  fifo->sz = 0;
}

size_t fifo_get_used_bytes(struct fifo_t *fifo) {
  size_t tmp = fifo->writePtr - fifo->readPtr;

  if (fifo->writePtr < fifo->readPtr)
    tmp += fifo->sz;

  return tmp;
}
size_t fifo_get_free_bytes(struct fifo_t *fifo) {
  return fifo->sz - fifo_get_used_bytes(fifo) - 1;
}

size_t fifo_put_data(struct fifo_t *fifo, fifo_data_t *data, size_t sz) {
  size_t free_bytes = fifo_get_free_bytes(fifo);
  // Only insert maximum possible byte count
  if (free_bytes < sz)
    sz = free_bytes;

  size_t bytes_until_end = fifo_get_bytes_until_end(fifo, fifo->writePtr);
  // Take minimum of all bytes to copy and available bytes
  size_t first_copy_size = bytes_until_end > sz ? sz : bytes_until_end;
  memcpy(fifo->writePtr, data, first_copy_size);

  // Still bytes missing?
  if (first_copy_size < sz)
  {
    memcpy(fifo->startPtr, data + first_copy_size, sz - first_copy_size);
    fifo->writePtr = fifo->startPtr + sz - first_copy_size;
  } else {
    // Point to next write position
    fifo->writePtr += sz;
  }
  return sz;
}
size_t fifo_get_data(struct fifo_t *fifo, fifo_data_t *buff, size_t sz) {
  size_t used_bytes = fifo_get_used_bytes(fifo);
  if (used_bytes < sz)
    sz = used_bytes;

  size_t bytes_until_end = fifo_get_bytes_until_end(fifo, fifo->readPtr);
  // Take minimum of all bytes to copy and available bytes
  size_t first_copy_size = bytes_until_end > sz ? sz : bytes_until_end;
  memcpy(buff, fifo->readPtr, first_copy_size);

  // Still bytes missing?
  if (first_copy_size < sz)
  {
    memcpy(buff, fifo->startPtr + first_copy_size, sz - first_copy_size);
    fifo->readPtr = fifo->startPtr + sz - first_copy_size;
  } else {
    // Point to next write position
    fifo->readPtr += sz;
  }
  return sz;
}

size_t fifo_get_until(struct fifo_t *fifo, fifo_data_t* buff, fifo_data_t delimit, size_t maxCnt) {
  size_t used_bytes = fifo_get_used_bytes(fifo);
  int idx = 0;
  fifo_data_t* tmpReadPtr = fifo->readPtr;

  while (idx < maxCnt && idx < used_bytes) {
    buff[idx] = *tmpReadPtr++;

    if (tmpReadPtr > fifo->endPtr)
      tmpReadPtr = fifo->startPtr;

    if (buff[idx] == delimit) {
      fifo->readPtr = tmpReadPtr;
      return idx + 1; // Return actual read bytes
    }

    idx++;
  }

  // Return 0 if delimiter not found
  return 0;
}

size_t fifo_get_bytes_until_end(struct fifo_t *fifo, fifo_data_t *ptr) {
  return fifo->endPtr - ptr + 1;
}


#endif

