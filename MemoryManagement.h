// Pre-compiler directive
#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

// header files
#include <stdio.h> // file operations
#include <stdlib.h> // dynamic memory operations
#include "metadataops.h"
#include "configops.h"
#include "simulator.h"

// PCB data structure
typedef struct MemoryBlock
{
   // upper bound of memory
   int upperBound;
   
   // lower bound of memory
   int lowerBound;
   
   // pointer to which pcb allocated memory
   struct PCB *designatedPCB;
   
   // pointer to the next PCB process
   struct MemoryBlock *nextNode;
} MemoryBlock;

// function prototypes
Boolean allocateMem( MemoryBlock *newMemNode, MemoryBlock **memListHead, 
                                                ConfigDataType *configDataPtr );
Boolean accessMem( int lowerBound, int upperBound, MemoryBlock *memListHead,
                                 ConfigDataType *configDataPtr, int pidOfAccess );
MemoryBlock *clearMemoryList( MemoryBlock *localPtr );

#endif   // MEMORY_MANAGEMENT_H
