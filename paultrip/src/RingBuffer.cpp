//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.
  
  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file RingBuffer.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */


#include "RingBuffer.h"

#include <iostream>
#include <cstring>

using std::cout; using std::endl;


//*******************************************************************************
RingBuffer::RingBuffer(int SlotSize, int NumSlots) : 
  mSlotSize(SlotSize),
  mNumSlots(NumSlots),
  mTotalSize(mSlotSize*mNumSlots),
  mReadPosition(0),
  mWritePosition(0),
  mFullSlots(0),
  mRingBuffer(new int8_t[mTotalSize]),
  mLastReadSlot(new int8_t[mSlotSize])
{
  // Verify if there's enough space to for the buffers
  if ( (mRingBuffer == NULL) || (mLastReadSlot == NULL) ) {
    std::cerr << "ERROR: RingBuffer out of memory!" << endl;
    std::cerr << "Exiting program..." << endl;
    std::exit(1);
  }

  // Set the buffers to zeros
  for (int i=0; i<mTotalSize; i++) {
    mRingBuffer[i] = 0;    // Initialize all elements to zero.
  }
  for (int i=0; i<mSlotSize; i++) {
    mLastReadSlot[i] = 0;    // Initialize all elements to zero.
  }
  
  // Advance write position to half of the RingBuffer
  mWritePosition = ( (NumSlots/2) * SlotSize ) % mTotalSize;
  // Udpate Full Slots accordingly
  mFullSlots = (NumSlots/2);
}


//*******************************************************************************
RingBuffer::~RingBuffer()
{
  delete[] mRingBuffer; // Free memory
  mRingBuffer = NULL; // Clear to prevent using invalid memory reference
  delete[] mLastReadSlot;
  mLastReadSlot = NULL;
}


//*******************************************************************************
void RingBuffer::insertSlotBlocking(const int8_t* ptrToSlot)
{
  QMutexLocker locker(&mMutex); // lock the mutex
  // Check if there is space available to write a slot
  // If the Ringbuffer is full, it waits for the bufferIsNotFull condition
  while (mFullSlots == mNumSlots) {
    //std::cout << "OUPUT OVERFLOW BLOCKING before" << std::endl;
    //debugDump();
    mBufferIsNotFull.wait(&mMutex);
    //std::cout << "OUPUT OVERFLOW BLOCKING after" << std::endl;
    //debugDump();
  }

  // Copy mSlotSize bytes to mRingBuffer
  std::memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
  // Update write position
  mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
  mFullSlots++; //update full slots
  //cout << "WRITEBLOCK" << endl;
  //debugDump();
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotEmpty.wakeAll();
}


//*******************************************************************************
void RingBuffer::readSlotBlocking(int8_t* ptrToReadSlot)
{
  QMutexLocker locker(&mMutex); // lock the mutex

  // Check if there are slots available to read
  // If the Ringbuffer is empty, it waits for the bufferIsNotEmpty condition
  while (mFullSlots == 0) {
    //std::cerr << "READ UNDER-RUN BLOCKING before" << endl;
    //debugDump();
    mBufferIsNotEmpty.wait(&mMutex);
    //std::cerr << "READ UNDER-RUN BLOCKING after" << endl;
    //debugDump();
  }
  
  // Copy mSlotSize bytes to ReadSlot
  std::memcpy(ptrToReadSlot, mRingBuffer+mReadPosition, mSlotSize);
  // Update write position
  mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
  mFullSlots--; //update full slots
  //cout << "READBLOCK" << endl;
  //debugDump();
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotFull.wakeAll();
}


//*******************************************************************************
void RingBuffer::insertSlotNonBlocking(const int8_t* ptrToSlot)
{
  QMutexLocker locker(&mMutex); // lock the mutex

  // Check if there is space available to write a slot
  // If the Ringbuffer is full, it returns without writing anything
  // and resets the buffer
  /// \todo It may be better here to insert the slot anyways,
  /// instead of not writing anything
  if (mFullSlots == mNumSlots) {
    std::cout << "OUPUT OVERFLOW NON BLOCKING = " << mNumSlots << std::endl;
    overflowReset();
    return;
  }
  
  // Copy mSlotSize bytes to mRingBuffer
  std::memcpy(mRingBuffer+mWritePosition, ptrToSlot, mSlotSize);
  // Update write position
  mWritePosition = (mWritePosition+mSlotSize) % mTotalSize;
  mFullSlots++; //update full slots
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotEmpty.wakeAll();
}


//*******************************************************************************
void RingBuffer::readSlotNonBlocking(int8_t* ptrToReadSlot)
{
  QMutexLocker locker(&mMutex); // lock the mutex
  
 
  // Check if there are slots available to read
  // If the Ringbuffer is empty, it returns a buffer of zeros and rests the buffer
  if (mFullSlots == 0) {
    // Returns a buffer of zeros if there's nothing to read
    std::cerr << "READ UNDER-RUN NON BLOCKING = " << mNumSlots << endl;
    //std::memset(ptrToReadSlot, 0, mSlotSize);
    underrunReset();
    return;
  }
  
  // Copy mSlotSize bytes to ReadSlot
  std::memcpy(ptrToReadSlot, mRingBuffer+mReadPosition, mSlotSize);
  // Update write position
  mReadPosition = (mReadPosition+mSlotSize) % mTotalSize;
  mFullSlots--; //update full slots
  // Wake threads waitng for bufferIsNotFull condition
  mBufferIsNotFull.wakeAll();
}


//*******************************************************************************
// Under-run happens when there's nothing to read.
void RingBuffer::underrunReset()
{
  cout << "UNDERRRUNRESET BEFORE" << endl;
  debugDump();

  // Advance the write pointer 1/2 the ring buffer
  //mWritePosition = ( mReadPosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
  mWritePosition = ( mWritePosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
  mFullSlots += mNumSlots/2;
  // Set the entire buffer to 0
  std::memset(mRingBuffer, 0, mTotalSize);
  cout << "UNDERRRUNRESET AFTER" << endl;
  debugDump();
}


//*******************************************************************************
// Over-flow happens when there's no space to write more slots.
void RingBuffer::overflowReset()
{
  cout << "OVERFLOWRESET BEFORE" << endl;
  debugDump();

  // Advance the read pointer 1/2 the ring buffer
  //mReadPosition = ( mWritePosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
  mReadPosition = ( mReadPosition + ( (mNumSlots/2) * mSlotSize ) ) % mTotalSize;
  mFullSlots -= mNumSlots/2;

  cout << "OVERFLOWRESET AFTER" << endl;
  debugDump();

}


void RingBuffer::debugDump() const
{
  cout << "mTotalSize = " << mTotalSize << endl;
  cout << "mReadPosition = " << mReadPosition << endl;
  cout << "mWritePosition = " << mWritePosition << endl;
  cout <<  "mFullSlots = " << mFullSlots << endl;
}
