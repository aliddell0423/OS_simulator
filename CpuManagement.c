#include "simulator.h"
#include "configops.h"
#include "metadataops.h"
#include "simtimer.h"
#include "MemoryManagement.h"
#include "CpuManagement.h"
#include "DisplayLog.h"
#include <pthread.h>
#include <signal.h>
#include <math.h>

// global constants
const int MAX_THREADS = 500;
const int absoluteMin = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Boolean second_mutex = True;


/*
Funtion name: selectPCB
Algorithm: will select a PCB node depending on the scheduling format specified
Precondition: The scheduling format is preemptive
Postcondiditon: return the PCB selected due to above conditions
Exceptions: none
Note: none
*/
PCB *selectPCB( PCB *pcbListHead, int schedFormat, int loggingFormatFlag,
            fileOutputBuffer *localList, ConfigDataType *configPtr, 
            PCB *previousProcess )
{
   // initialize variables
   PCB *nodeSelector = NULL;
   PCB *localPtr = pcbListHead;
   Boolean readyPCBFound = False;
   int lowestRuntime = 2147483647;
   int highestWaitTime = -1;
   
   // branch to specified cpu sched code
   switch( schedFormat )
   {
      case CPU_SCHED_FCFS_P_CODE:
      case CPU_SCHED_FCFS_N_CODE:
      
         // loop through the pcb linked list until null or found a ready
         while( localPtr != NULL && readyPCBFound == False )
         {
            // check if pcb is ready
            if( compareString( localPtr->processState, "READY" ) == STR_EQ )
            {
               // set boolean to true
               readyPCBFound = True;
               
               // set node selector to found PCB
               nodeSelector = localPtr;
            }
            
            // iterate to next pointer
            localPtr = localPtr->nextNode;
         }
         // end loop
         break;
         
      case CPU_SCHED_SRTF_P_CODE:
      case CPU_SCHED_SJF_N_CODE:
         
         // loop through pcb linked list until null
         while( localPtr != NULL )
         {
            // check if lowest runtime is 0 or if it is greater current PCB's ms
            if( lowestRuntime > localPtr->msLeft  &&
                   compareString( localPtr->processState, "READY" ) == STR_EQ )
            {
               // if so, reset lowest runtime
               lowestRuntime = localPtr->msLeft;
               
               // set node selector to current node
               nodeSelector = localPtr;
            }
            // iterate to next pointer
            localPtr = localPtr->nextNode;
         }
         // end loop
         break;

      case CPU_SCHED_RR_P_CODE:
         
         // loop through pcb linked list until null
         while( localPtr != NULL )
         {
            // check if highest wait time is less than current PCB's
            if( highestWaitTime < localPtr->waitTime &&
                  compareString( localPtr->processState, "READY" ) == STR_EQ )
            {
               // if so, reset highest wait time
               highestWaitTime = localPtr->waitTime;
               
               // set node selector to current nodeSelector
               nodeSelector = localPtr;
            }
            
            // iterate to next pointer
            localPtr = localPtr->nextNode;
         }
         // end loop
         break;
   }
   
   // check if the node selector is not null
   if( nodeSelector != NULL )
   {
   
      if( nodeSelector->pid != previousProcess->pid )
      {
         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, 
                                                localList, PCB_SELECT, NONE );

         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, 
                                          localList, PCB_READY, PCB_RUNNING );

      }
      
      // increase wait times of all PCB's
         // function: increaseWaitTimes
      increaseWaitTimes( pcbListHead );
      
      // set the selected node's wait time to zero
      nodeSelector->waitTime = 0;
      
      if( compareString(nodeSelector->processState, "BLOCKED") != STR_EQ )
      {
         // set the process to running
         copyString( nodeSelector->processState, "RUNNING");
      }
      
      // reset the previous process variable
      previousProcess->pid = nodeSelector->pid;
      
   }
   
   return nodeSelector; // return the selected PCB
}

/*
Funtion name: increaseWaitTimes
Algorithm: increases all wait times by 1
Precondition: none
Postcondiditon: none
Exceptions: none
Note: none
*/
void increaseWaitTimes( PCB *pcbListHead )
{
   // initialize variables
   PCB *localPtr = pcbListHead;
   
   // loop until the end of the list
   while( localPtr != NULL )
   {
      // check if the process state is in Exit state
         // function: compareString
      if( compareString( localPtr->processState, "EXIT" ) == STR_EQ )
      {
         localPtr->waitTime = 0;
      }
      // otherwise increase the wait time
      else
      {
         // increase wait time
         localPtr->waitTime++;
      }
      
      // iterate to next PCB
      localPtr = localPtr->nextNode;
   }
}

/*
Funtion name: cpuIdle
Algorithm: will idle the CPU until the process with the lowest number of cycles
           left in its op code are finished. It will then report that said 
           process has interrupted the idle.
Precondition: all processes are in the BLOCKED state
Postcondiditon: will return an interrupt code depending on outcome of changing 
                cycles left.
Exceptions: none
Note: none
*/
cpuInterruptCodes cpuIdle( PCB *pcbListHead, int loggingFormatFlag, 
                        fileOutputBuffer *localList, ConfigDataType *configPtr )
{
   // initialize variables
   int minimumMSLeft;
   
   // log event
   displayAndLog( loggingFormatFlag, NULL, localList, CPU_IDLE, START );

   // find minimum cycles left to take from each PCB
      // function getMinimumCycles
   minimumMSLeft = getMinimumMS( pcbListHead );
   
   // subtract minimum cycles from each blocked pcb
      // function: updateRemainingCycles
   updateRemainingMS( pcbListHead, minimumMSLeft );
   
   // run timer for specified amount of cycles
   runTimer( minimumMSLeft );
   
   // log event
   displayAndLog( loggingFormatFlag, NULL, localList, CPU_IDLE, FINISH );
   
   return INTERRUPT_OUT_OF_CYCLES; // stub
}

/*
Funtion name: cpuRun
Algorithm: will run a cpu op code for a designated pcb. It will interrupt
           if the op code runs out of cycles or quantum time has been reached
Precondition: The selected PCB node's current op code is a cpu task
Postcondiditon: will return an interrupt code depending on outcome of changing 
                cycles left.
Exceptions: none
Note: none
*/
cpuInterruptCodes cpuRun( PCB *pcbListHead, PCB *nodeSelector, 
                  int loggingFormatFlag, fileOutputBuffer *localList,
                  ConfigDataType *configPtr )
{
   // initialize variables
   int maxQuantumMS;
   int ioMinMS;
   int procMSLeft;
   int trueMin;
   int pid = nodeSelector->pid;
   pthread_t ioThread[MAX_THREADS];
   
   while( compareString( nodeSelector->currentOpCode->command, "mem" ) != STR_EQ )
      {
         if( compareString( nodeSelector->currentOpCode->command, "cpu" ) == STR_EQ )
         {
            maxQuantumMS = configPtr->quantumCycles * configPtr->procCycleRate;
            ioMinMS = getMinimumMS( pcbListHead );
            procMSLeft = nodeSelector->opMSLeft;
            trueMin = findMin( ioMinMS, maxQuantumMS, procMSLeft );
            
            // log event
            displayAndLog( loggingFormatFlag, nodeSelector, localList, CPU, START );
            
            // update the remaining ms
            updateRemainingMS( pcbListHead, trueMin );
            
            // run timer for designated minimum MS
            runTimer( trueMin );
            
            // check if minimum is a blocked process
            if( trueMin == ioMinMS )
            {
               // update finished IO
               updateIO( pcbListHead, loggingFormatFlag, localList, configPtr );
            }
            // check if minimum is the quantum timer
            else if( trueMin == maxQuantumMS )
            {
               // set interrupt to the quantum time
               return INTERRUPT_FROM_QUANTUM;
            }
            // otherwise, assume it is the process time left that is the minimumMSLeft
            else
            {
               // log event
               displayAndLog( loggingFormatFlag, nodeSelector, localList, CPU, FINISH );
               
               updatePcbOpCode( nodeSelector, configPtr );
            }
         }
         // assume the op code is dev
         else
         {
            // log event
            displayAndLog( loggingFormatFlag, nodeSelector, localList, DEV, START );
            
            pthread_create(&ioThread[pid], NULL, runPThread, NULL);
            
            return INTERRUPT_FROM_IO_BLOCK;
         }

      }
   
   return NO_INTERRUPT; // stub
}

/*
Funtion name: interruptHandler
Algorithm: will perform a set of tasks depending on the interrupt code it
           receives. It will typically change the next op code of a PCB and
           switch their process state.
Precondition: The interrupt code assigned is correct
Postcondiditon: return PCB 
Exceptions: none
Note: none
*/
void interruptHandler( PCB *pcbListHead, cpuInterruptCodes currentInterrupt, 
                       int loggingFormatFlag, fileOutputBuffer *localList,
                       ConfigDataType *configPtr, PCB *nodeSelector )
{
   // start switch case for each interrupt code 
   switch( currentInterrupt )
   {
      case INTERRUPT_FROM_QUANTUM:

         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, localList, QUANT_INT, NONE );

         // set the PCB's state to READY
         copyString( nodeSelector->processState, "READY" );
         
         // update all io processes that have reached 0 ms
            // function: updateIO
         updateIO( pcbListHead, loggingFormatFlag, localList, configPtr );

         break;

      case INTERRUPT_FROM_IO_BLOCK:

         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, localList, BLOCKED_INT, NONE );
         
         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, 
                                          localList, PCB_RUNNING, PCB_BLOCKED );

         // change the pcb state from running to blocked
            // function: copyString
         copyString( nodeSelector->processState, "BLOCKED" );

         break;

      case INTERRUPT_OUT_OF_CYCLES:

         // update all io processes that have reached 0 ms
            // function: updateIO
         updateIO( pcbListHead, loggingFormatFlag, localList, configPtr );

         break;

      case INTERRUPT_SEG_FAULT:

         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, localList, MEM_FAIL, NONE );

         // change the pcb state from running to exit
            // function: copyString
         copyString( nodeSelector->processState, "EXIT" );
         
         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, localList, PCB_EXIT, NONE );

         break;

      case SUCCESSFUL_MEM_OP:
      
         // log event
         displayAndLog( loggingFormatFlag, nodeSelector, localList, MEM_SUCCESS, NONE );

         // set the next op code for pcb
            // function: updatePcbOpCode
         updatePcbOpCode( nodeSelector, configPtr );
         
         break;

      case NO_INTERRUPT:
         break;
   }
   
   // check to see if any pcb's have finished all of their ops
      // function: checkListForFinish
   checkListForFinishAndReady( pcbListHead, loggingFormatFlag, localList );
}

/*
Funtion name: getMinimumCycles
Algorithm: go through the pcb list and return the lowest amount of cycles left 
           that were found
Precondition: none of the pcb's have 0 cycles left
Postcondiditon: will return an integer for the number of cycles found
Exceptions: none
Note: none
*/
int getMinimumMS( PCB *pcbListHead )
{
   // initialize vaiables
   int MAX_MS = 2147483647;
   PCB *localPtr = pcbListHead;
   int minimumMS = MAX_MS;
   
   // loop through PCB list
   while( localPtr != NULL )
   {
      //printf( "GET_MINIMUM_MS: PCB %d State: %s OP MS Left: %d \n", localPtr->pid, localPtr->processState, localPtr->opMSLeft );
      // check if cycles left if greater than local pointer'switch
      if( ( minimumMS > localPtr->opMSLeft ) &&
             compareString( localPtr->processState, "BLOCKED" ) == STR_EQ )
      {
         // set new minimum
         minimumMS = localPtr->opMSLeft;
      }
      // iterate forward
      localPtr = localPtr->nextNode;
   }
   
   return minimumMS; // stub
}

/*
Funtion name: updateRemainingCycles
Algorithm: go through the pcb list and subtract designated cycles
Precondition: none of the pcb's have 0 cycles left
Postcondiditon: none
Exceptions: none
Note: none
*/
void updateRemainingMS( PCB *pcbListHead, int msToSubtract )
{
   // initialize vaiables
   PCB *localPtr = pcbListHead;
   
   // loop through PCB list
   while( localPtr != NULL )
   {
      // check if process is either running or blocked
      if( compareString( localPtr->processState, "BLOCKED" ) == STR_EQ ||
            compareString( localPtr->processState, "RUNNING" ) == STR_EQ )
      {
         // subtract all number from cycles left
         localPtr->msLeft = localPtr->msLeft - msToSubtract;
         
         // subtract all number from cycles left
         localPtr->opMSLeft = localPtr->opMSLeft - msToSubtract;
      }
      
      // iterate forward
      localPtr = localPtr->nextNode;
   }
}

/*
Funtion name: findMin
Algorithm: find minimum of 3 integers
Precondition: none
Postcondiditon: none
Exceptions: none
Note: none
*/
int findMin( int first, int second, int third )
{
   // initialize counter
   int counter = 0;
   
   //loop until one of the numbers gets to zero
   while( first && second && third )
   {
      first--;
      second--;
      third--;
      counter++;
   }
   // return counter
   return counter;
}

/*
Funtion name: updatePcbOpCode
Algorithm: reset the wait time and op code of selected pcb
Precondition: the wait time in ms is at 0 already
Postcondiditon: the pcb is back to being able to start again (ready state)
Exceptions: none
Note: none
*/
void updatePcbOpCode( PCB *nodeSelector, ConfigDataType *configPtr )
{
   // initialize variables
   int procCycleRate = configPtr->procCycleRate;
   int ioCycleRate = configPtr->ioCycleRate;

   // set the next op code for pcb
   nodeSelector->currentOpCode = nodeSelector->currentOpCode->nextNode;
   
   // check for if op code in not mem
   if( compareString( nodeSelector->currentOpCode->command, "mem" ) == STR_EQ )
   {
      // set the new wait time for current op code
      nodeSelector->opMSLeft = 0;
   }
   // check for if op code in not mem
   else if( compareString( nodeSelector->currentOpCode->command, "cpu" ) == STR_EQ )
   {
      // set the new wait time for current op code
      nodeSelector->opMSLeft = nodeSelector->currentOpCode->intArg2*
                                                            procCycleRate;
   }
   // check for if op code in not mem
   else
   {
      // set the new wait time for current op code
      nodeSelector->opMSLeft = nodeSelector->currentOpCode->intArg2*
                                                            ioCycleRate;
   }
   
   // set process state to ready
   copyString( nodeSelector->processState, "READY" );
}

/*
Funtion name: checkListForFinish
Algorithm: checks for if any current op codes are at app end, and will update 
           the wait time of any processes that have remained in READY
Precondition: none
Postcondiditon: changes pcb to exit state and reports change
Exceptions: none
Note: none
*/
void checkListForFinishAndReady( PCB *pcbListHead, int loggingFormatFlag,
                                                   fileOutputBuffer *localList )
{
   // initialize variables
   PCB *localPtr;
   
   // set local to list head
   localPtr = pcbListHead;
   
   // loop until at end of list
   while( localPtr != NULL )
   {
      if( compareString( localPtr->currentOpCode->command, "app" ) == STR_EQ &&
         compareString( localPtr->currentOpCode->strArg1, "end" ) == STR_EQ &&
         compareString( localPtr->processState, "EXIT" ) != STR_EQ )
         {
            // log event
            displayAndLog( loggingFormatFlag, localPtr, localList, PCB_EXIT, NONE );

            // set pcb state to exit
            copyString( localPtr->processState, "EXIT" );
         }

      // iterate to next PCB
      localPtr = localPtr->nextNode;

   // end loop
   }
}

/*
Funtion name: updateIO
Algorithm: updates all blocked processes that have finished their dev code
Precondition: none
Postcondiditon: changes pcb to exit state and reports change
Exceptions: none
Note: none
*/
void updateIO( PCB *pcbListHead, int loggingFormatFlag,
                        fileOutputBuffer *localList, ConfigDataType *configPtr )
{
   // initialize variables
   PCB *localPtr;
   
   // set local to list head
   localPtr = pcbListHead;
   
   // loop until at end of list
   while( localPtr != NULL )
   {
      // check if the process state is running
      if( compareString( localPtr->processState, "RUNNING" ) == STR_EQ )
      {
         // change the state back to ready
            // function: copyString
         copyString( localPtr->processState, "READY" );
      }
      // check if the process is blocked and their ms left is zero
      if( compareString( localPtr->processState, "BLOCKED" ) == STR_EQ &&
            localPtr->opMSLeft == 0 )
         {
            // unlock mutex
            second_mutex = False;
            pthread_mutex_unlock(&mutex);
            
            // log event
            displayAndLog( loggingFormatFlag, localPtr, localList, FINISH_INT, NONE );
   
            // log event
            displayAndLog( loggingFormatFlag, localPtr, localList, PCB_BLOCKED, NONE );
            
            // update the specific PCB
            updatePcbOpCode( localPtr, configPtr );
         }

      // iterate to next PCB
      localPtr = localPtr->nextNode;

   // end loop
   }
}

/*
Funtion name: runPThread
Algorithm: Runs the timer for the number of cycles specified
           for the IO op code
Precondition: Given op code pointer cycles
Postcondiditon: none
Exceptions: none
Note: none
*/
void *runPThread()
{
   while(second_mutex == True);
   second_mutex = True;
   pthread_mutex_lock(&mutex);
   
   // pause the program for the specified cycles
            // function: runTimer
   runTimer( absoluteMin );
   
   //printf("Thread complete\n");
   
   return NULL;
}
