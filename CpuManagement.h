// Pre-compiler directive
#ifndef CPUMANAGEMENT_H
#define CPUMANAGEMENT_H

// codes for interrupt messages
typedef enum { INTERRUPT_FROM_QUANTUM,
               INTERRUPT_FROM_IO_BLOCK,
               INTERRUPT_OUT_OF_CYCLES,
               INTERRUPT_SEG_FAULT,
               SUCCESSFUL_MEM_OP,
               NO_INTERRUPT } cpuInterruptCodes;
               
// function prototypes
PCB *selectPCB( PCB *pcbListHead, int schedFormat, int loggingFormatFlag,
            fileOutputBuffer *localList, ConfigDataType *configPtr, 
            PCB *previousProcess );
cpuInterruptCodes cpuIdle( PCB *pcbListHead, int loggingFormatFlag, 
                        fileOutputBuffer *localList, ConfigDataType *configPtr );
cpuInterruptCodes cpuRun( PCB *pcbListHead, PCB *nodeSelector, 
                  int loggingFormatFlag, fileOutputBuffer *localList,
                  ConfigDataType *configPtr );
void interruptHandler( PCB *pcbListHead, cpuInterruptCodes currentInterrupt, 
                       int loggingFormatFlag, fileOutputBuffer *localList,
                       ConfigDataType *configPtr, PCB *nodeSelector );
int getMinimumMS( PCB *pcbListHead );
void updateRemainingMS( PCB *pcbListHead, int msToSubtract );
int findMin( int first, int second, int third );
void updatePcbOpCode( PCB *nodeSelector, ConfigDataType *configPtr );
void checkListForFinishAndReady( PCB *pcbListHead, int loggingFormatFlag,
                                                   fileOutputBuffer *localList );
void updateIO( PCB *pcbListHead, int loggingFormatFlag,
                        fileOutputBuffer *localList, ConfigDataType *configPtr );
void increaseWaitTimes( PCB *pcbListHead );
void *runPThread();

#endif   // CPUMANAGEMENT_H
