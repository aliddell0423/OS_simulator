// Pre-compiler directive
#ifndef DISPLAYLOG_H
#define DISPLAYLOG_H

// header files
#include <stdio.h> // file operations
#include <stdlib.h> // dynamic memory operations
#include "metadataops.h"
#include "configops.h"
#include "MemoryManagement.h"
#include "simulator.h"

// codes for displaying
typedef enum {
   NONE,
   INIT,
   START,
   FINISH,
   PCB_NEW,
   PCB_READY,
   PCB_RUNNING,
   PCB_BLOCKED,
   PCB_EXIT,
   PCB_SELECT,
   CPU,
   DEV,
   MEM_START,
   MEM_SUCCESS,
   MEM_FAIL,
   ALL_EXIT,
   CPU_IDLE,
   QUANT_INT,
   BLOCKED_INT,
   FINISH_INT }DisplayCodes;
   
// struct for the file output buffer
typedef struct fileOutputBuffer
{
   //string for specified file line
   char lineString [ 100 ];

   // pointer to next line
   struct fileOutputBuffer *nextNode;
} fileOutputBuffer;

// function prototypes
//void displayAndLog( int loggingFormatFlag, PCB *pcb, 
//         fileOutputBuffer *fileOutputHead, DisplayCodes arg1, DisplayCodes arg2 );


#endif // ifndef DISPLAYLOG_H
