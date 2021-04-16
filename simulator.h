// Pre-compiler directive
#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include <stdio.h> // file operations
#include <stdlib.h> // dynamic memory operations
#include "metadataops.h"
#include "configops.h"
#include "MemoryManagement.h"
#include "simulator.h"
#include "DisplayLog.h"

// create global constants - across files
extern const int MAX_MILLISECONDS;
extern char *StateTypes[];
extern char *SchedTypes[];

// process block states
typedef enum { PCB_NEW_STATE,
               PCB_READY_STATE,
               PCB_RUNNING_STATE,
               PCB_EXIT_STATE } pcbStateCodes;
               
// PCB data structure
typedef struct PCB
{
   // This specifies the process state i.e. new, ready, running, waiting or terminated.
   char processState [ 15 ];
   
   // This shows the number of the particular process.
   int pid;
   
   // time left for this process
   int msLeft;
   
   // time left for current op code
   int opMSLeft;
   
   // wait time for pcb
   int waitTime;
   
   // op code location
   OpCodeType *currentOpCode;
   
   // pointer to the next PCB process
   struct PCB *nextNode;
   
   // thread to linked list
   struct OpCodeType *thread;
} PCB;

// function prototypes
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );
void showProgramFormat();
PCB *addPcbNode( PCB *localPtr, PCB *newNode );
fileOutputBuffer *addListNode( fileOutputBuffer *localPtr, 
                                                   fileOutputBuffer *newNode );
void *runThread( void *milliseconds );
fileOutputBuffer *clearLogList( fileOutputBuffer *localPtr );
PCB *clearPCBList( PCB *localPtr );
void displayAndLog( int loggingFormatFlag, PCB *pcb, 
         fileOutputBuffer *fileOutputHead, DisplayCodes arg1, DisplayCodes arg2 );
DisplayCodes findDisplayCode( char *command );

#endif   // SIMULATOR_H

