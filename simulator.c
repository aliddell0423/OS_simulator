#include "simulator.h"
#include "configops.h"
#include "metadataops.h"
#include "simtimer.h"
#include "MemoryManagement.h"
#include "CpuManagement.h"
#include "DisplayLog.h"
#include <pthread.h>
#include <signal.h>

/*
Funtion name: addPcbNode
Algorithm: adds PCB structure with data to a linked list
Precondition: linked list pointer assigned to null or to one PCB link,
              struct pointer assigned to PCB struct data
Postcondiditon: assigns new structure node to beginning of linked list
                or and of established linked list
Exceptions: none
Note: assumes memory access/availability
*/
PCB *addPcbNode( PCB *localPtr, PCB *newNode )
{
   // check for local pointer assigned to null
   if( localPtr == NULL )
   {
      // access new memory for new link/node
         // function: malloc
      localPtr = (PCB *)malloc( sizeof( PCB ) );
      
      // assign all three values to newly created node
      // assign next pointer to null
         // function: copyString
      localPtr->pid = newNode->pid;
      copyString( localPtr->processState, newNode->processState );
      localPtr->msLeft = newNode->msLeft;
      localPtr->thread = newNode->thread;
      localPtr->waitTime = newNode->waitTime;
      localPtr->currentOpCode = newNode->currentOpCode;
      
      localPtr->nextNode = NULL;
         
      // return current local pointer
      return localPtr;
   }  
   // assume end of list is not found yet
   // assign recursive function to current's next link
      // function: addNode
   localPtr->nextNode = addPcbNode( localPtr->nextNode, newNode );
      
   // return local pointer   
   return localPtr;
}

/*
Funtion name: runIOThread
Algorithm: Runs the timer for the number of cycles specified
           for the IO op code
Precondition: Given op code pointer cycles
Postcondiditon: none
Exceptions: none
Note: none
*/
void *runThread( void *milliseconds )
{
   // convert the void pointer into an int
   int millisecondsToWait = *(int*) milliseconds;
   
   // pause the program for the specified cycles
            // function: runTimer
   runTimer( millisecondsToWait );
   
   return NULL;
}

/*
Funtion name: runSim
Algorithm: master driver for simulator operations;
           conducts OS simulation with varying scheduling strategies
           and varying numbers of processes
Precondition: given head pointer to config data and meta data
Postcondiditon: simulation is provided, file output is provided as configured
Exceptions: none
Note: none
*/
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
{  
   // initialize variables
   int loggingFormatFlag;
   int cpuSchedulingFlag;
   int runMilliseconds;
   PCB *localPtr = NULL;
   PCB *pcbListHead = NULL;
   PCB *newNode = NULL;
   PCB *nodeSelector = NULL;
   static PCB *previousProcess = NULL;
   OpCodeType *localMetaDataPtr = metaDataMstrPtr;
   int pidCounter = 0;
   int queueCounter;
   fileOutputBuffer *localList = NULL;
   static fileOutputBuffer *listHead = NULL;
   MemoryBlock *newMemNode = NULL;
   static MemoryBlock *memListHead = NULL;
   FILE* logFile;
   cpuInterruptCodes currentInterrupt;
   Boolean allProcessesExited = False;
   Boolean allProcessesBlocked;
   Boolean segFault;
   DisplayCodes whichCommand;
   pthread_t Thread;
   
   // reference the logging option in the config file (pointer) and apply it to a 
   //  string flag
   loggingFormatFlag = configPtr->logToCode;
      
   // reference the cpu scheduling format and place it into another flag
   cpuSchedulingFlag = configPtr->cpuSchedCode;
   
   // allocate memory for the new node
      // function: malloc
   newNode = (PCB *)malloc( sizeof( PCB ) );
   
   // allocate memory for the new memory block node
   newMemNode = (MemoryBlock *)malloc( sizeof( MemoryBlock ) );
   
   // allocate memory for the new file output block node
   listHead = (fileOutputBuffer *)malloc( sizeof( fileOutputBuffer ) );
   listHead->nextNode = NULL;

   // loop through op code list until local pointer is at sys end
   while( localMetaDataPtr->nextNode != NULL )
   {
      // check for if command is of "app"
         // function: compareString
      if( compareString(localMetaDataPtr->command, "app" ) == STR_EQ )
      {
         // check for start arg in current line
            // function: compareString
         if( compareString(localMetaDataPtr->strArg1, "start" ) == STR_EQ )
         {            
            // state for PCB will be set to NEW
               // function: copyString
            copyString( newNode->processState, "NEW" );
            
            // set current pid counter to PCB pid
            newNode->pid = pidCounter;
            
            // make PCB thread pointer to meta data config pointer
            newNode->thread = localMetaDataPtr;
            
            newNode->currentOpCode = localMetaDataPtr;
            
            // set wait tiime to zero
            newNode->waitTime = 0;
         }
         // check for end arg in current line
            // function: compareString
         if( compareString( localMetaDataPtr->strArg1, "end" ) == STR_EQ )
         {
            // add node to list
            localPtr = addPcbNode( localPtr, newNode );
            
            // check if this is the first node
            if( pidCounter == 0 )
            {
               pcbListHead = localPtr;
            }
            
            // reset ms left on the newNode
            newNode->msLeft = 0;
                     
            // increment the PCB pid 1 more than the previous
            pidCounter++;
         }
      }
      // check for if the command is a cpu bound process
         // function: compareString
      if( compareString( localMetaDataPtr->command, "cpu" ) == STR_EQ )
      {
         // increment the PCB node's time left by calculating the cycles x ms/cycle
         newNode->msLeft += localMetaDataPtr->intArg2 * configPtr->procCycleRate;
      }
      // check for if the command is an IO bound process
         // function: compareString
      if( compareString( localMetaDataPtr->command, "dev" ) == STR_EQ )
      {
         // increment the PCB node's time left by calculating the cycles x ms/cycle
         newNode->msLeft += localMetaDataPtr->intArg2 * configPtr->ioCycleRate;
      }
      
      // iterate to next op code
      localMetaDataPtr = localMetaDataPtr->nextNode;
   }
   // end loop
   
   // set node pointer to beginning of PCB list
   localPtr = pcbListHead;
   
   // initialize queueCount
   queueCounter = pidCounter;
   
   // initialize logging process
   displayAndLog( loggingFormatFlag, NULL, listHead, INIT, NONE );
   
   // loop through PCB linked list to set all in ready state
   while( localPtr != NULL )
   {    
      // change state within PCB to READY
         // copyString
      copyString( localPtr->processState, "READY" );
      
      // set the op code MS left
      localPtr->opMSLeft = 0;
      
      // set default wait time for the PCB
      localPtr->waitTime = queueCounter;
      
      // log event
      displayAndLog( loggingFormatFlag, localPtr, listHead, PCB_NEW, PCB_READY );

      // set up PCB for running
      updatePcbOpCode( localPtr, configPtr );
      
      // decrement the queue counter by 1
      queueCounter--;
      
      // iterate to next process
      localPtr = localPtr->nextNode;
   }
   // end loop
   
   // check if the log spec is to file
   if( loggingFormatFlag == LOGTO_FILE_CODE )
   {
      printf("Completeing logging process, please wait...\n");
   }
   
   // set node pointer back to head of PCB linked list
   localPtr = pcbListHead;
   
   // set the new node's pid to mark previous processes
   newNode->pid = -1;
   
   // set the first previous process pointer
   previousProcess = newNode;
////////////////////////////////////////////////////////////////////////////////

   // create a case for each sched flag
   switch( cpuSchedulingFlag )
   {
      case CPU_SCHED_SJF_N_CODE:
      case CPU_SCHED_FCFS_N_CODE:
         
         // loop untl a node selector has not found a suitable pcb
         do
            {
               // assume there is no seg fault
               segFault = False;
               
               // get a node
                  // function: nodeSelector
               nodeSelector = selectPCB( pcbListHead, cpuSchedulingFlag,
                                         loggingFormatFlag,
                                         listHead, configPtr,
                                         previousProcess );
            
            // only do this branch if node found
            if( nodeSelector != NULL )
            {
               // while the current op code is not on an app end (and no seg fault)
               while( compareString( nodeSelector->currentOpCode->strArg1, 
                                                            "end") != STR_EQ &&
                                                segFault == False )
               {
                  // check if its a mem op
                  if( compareString( nodeSelector->currentOpCode->command, 
                                                            "mem") == STR_EQ )
                  {
                     // check if its an allocation
                     if( compareString( nodeSelector->currentOpCode->strArg1, 
                                                               "allocate") == STR_EQ )
                     {
                        // create new mem node
                        newMemNode->lowerBound = nodeSelector->currentOpCode->intArg2;
                        newMemNode->upperBound = ( nodeSelector->currentOpCode->
                                     intArg3 + nodeSelector->currentOpCode->intArg2 );
                        newMemNode->designatedPCB = nodeSelector;
                        
                        // if so, allocate mem and set to boolean
                           // function: allocateMem
                        segFault = allocateMem( newMemNode, &memListHead, configPtr );
                     }
                     // otherwise, assume its an access
                     else
                     {
                        // access mem and set to boolean
                           // function: accessMem
                        segFault = accessMem( newMemNode->lowerBound, 
                           newMemNode->upperBound, memListHead, 
                           configPtr, nodeSelector->pid );
                     }
                     
                     // log event
                     displayAndLog( loggingFormatFlag, nodeSelector, 
                                                      listHead, MEM_START, NONE );
                     
                     // check if seg fault occured
                     if( segFault == True )
                     {
                        // log event
                        displayAndLog( loggingFormatFlag, nodeSelector, 
                                                         listHead, MEM_FAIL, NONE );
                     }
                     // otherise, report successful
                     else
                     {
                        // log event
                        displayAndLog( loggingFormatFlag, nodeSelector, 
                                                         listHead, MEM_SUCCESS, NONE );
                     }
                  }
                  // otherwise, assume dev or cpu
                  else
                  {
                     // find appropriate display code
                     whichCommand = findDisplayCode( 
                                          nodeSelector->currentOpCode->command );
                     // log event
                     displayAndLog( loggingFormatFlag, nodeSelector, 
                                                      listHead, whichCommand, START );
                     
                     // set up time for thread use
                     runMilliseconds = nodeSelector->opMSLeft;
                     
                     // run IO Thread
                        // function: pthread_create, pthread_join
                     pthread_create( &Thread, NULL, runThread, (void*)&runMilliseconds );
                     pthread_join( Thread, NULL );
                     
                     // log event
                     displayAndLog( loggingFormatFlag, nodeSelector, 
                                                      listHead, whichCommand, FINISH );
                  }
                  // update the op code for specific pcb
                  updatePcbOpCode( nodeSelector, configPtr );
               }
               // log exit event
               displayAndLog( loggingFormatFlag, nodeSelector, 
                                                listHead, PCB_EXIT, NONE );
               // set pcb to exit
               copyString( nodeSelector->processState, "EXIT" );
            }
            } while( nodeSelector != NULL );
         
         break;

      // check if CPU scheduling flag is preemptive
      case CPU_SCHED_FCFS_P_CODE:
      case CPU_SCHED_SRTF_P_CODE:
      case CPU_SCHED_RR_P_CODE:

         // loop until all processes are in exit state
         while( allProcessesExited == False )
         {
            // set boolean saying all PCB's in exit state to true
            allProcessesExited = True;
            
            // set boolean saying all PCB's are in blocked state
            allProcessesBlocked = True;
            
            // set local pointer to list head
            localPtr = pcbListHead;
            
            // loop through list of PCB's 
            while( localPtr != NULL )
            {
               //printf( "PCB %d State: %s Total ms left: %d \n", localPtr->pid, localPtr->processState, localPtr->msLeft );
               // check if they are not in exit state
                // function: compareString
               if( compareString( localPtr->processState, "EXIT" ) != STR_EQ )
               {
                  // if so, switch the exit state flag to false
                  allProcessesExited = False;
               }
               // check if PCB is not in blocked state or exit state
               if( compareString( localPtr->processState,
                                                      "READY" ) == STR_EQ )
               {
                  // if so, switch the designated flag to false
                  allProcessesBlocked = False;
               }
               
               // iterate to next node
               localPtr = localPtr->nextNode;
            }
            // end loop
            
            // set the focus PCB with respective scheduler
               // function: selectPCB
            nodeSelector = selectPCB( pcbListHead, cpuSchedulingFlag, 
                                      loggingFormatFlag, listHead, configPtr, 
                                      previousProcess );

            // check if all processes have been exited
            if( allProcessesExited == True )
            {
               // log event
               displayAndLog( loggingFormatFlag, localPtr, listHead, ALL_EXIT, NONE );

               // set interrupt code to null
               currentInterrupt = NO_INTERRUPT;
            }
            // check if all blocked flag
            else if( allProcessesBlocked == True )
            {
               // if so, run the cpu idle case and set its interrupt code
                  // function: cpuIdle
               currentInterrupt = cpuIdle( pcbListHead, loggingFormatFlag,
                                                         listHead, configPtr );
            }
            // check if current op code is cpu
            else if( compareString( nodeSelector->currentOpCode->command, 
                                                            "mem" ) != STR_EQ )
            {
               // run cpu until cycles are gone or quantum out and set
               //   interrupt code
               // function: cpuRun
               currentInterrupt = cpuRun( pcbListHead, nodeSelector,
                                 loggingFormatFlag, listHead, configPtr );
            }
            // otherwise, op is mem
            else
            {
               // create new mem node
               newMemNode->lowerBound = nodeSelector->currentOpCode->intArg2;
               newMemNode->upperBound = ( nodeSelector->currentOpCode->
                            intArg3 + nodeSelector->currentOpCode->intArg2 );
               newMemNode->designatedPCB = nodeSelector;
               
               // check if it specifies allocation
               if( compareString( nodeSelector->currentOpCode->strArg1, 
                                                        "allocate" ) == STR_EQ )
               {
                  // log event
                  displayAndLog( loggingFormatFlag, nodeSelector, 
                                                   listHead, MEM_START, NONE );
               
                  // if so, allocate mem and set to boolean
                     // function: allocateMem
                  segFault = allocateMem( newMemNode, &memListHead, configPtr );
                     
               }
               // otherwise, it is an access request
               else
               {
                  // log event
                  displayAndLog( loggingFormatFlag, nodeSelector, 
                                                   listHead, MEM_START, NONE );
               
                  // access mem and set to boolean
                     // function: accessMem
                  segFault = accessMem( newMemNode->lowerBound, 
                     newMemNode->upperBound, memListHead, 
                     configPtr, nodeSelector->pid );
               }
               
               // check if seg fault occured
               if( segFault == True )
               {
                  // if yes, set interrupt code to seg fault
                  currentInterrupt = INTERRUPT_SEG_FAULT;
               }
               // otherise, report successful
               else
               {
                  currentInterrupt = SUCCESSFUL_MEM_OP;
               }
            }
            // handle the interrupt case
               // function: interruptHandler
            interruptHandler( pcbListHead, currentInterrupt,loggingFormatFlag, 
                              listHead, configPtr, nodeSelector );
         }
         // end loop
         break;
   }
   
////////////////////////////////////////////////////////////////////////////////
   
   // check if the logging option is file log or both
   if( loggingFormatFlag == LOGTO_FILE_CODE
       || loggingFormatFlag == LOGTO_BOTH_CODE )
   {     
      // put output list pointer back to the head
      localList = listHead;
   
      // open blank log file
         // function: fopen
      logFile = fopen( configPtr->logToFileName, "w" );
      
      // loop through output string list
      while( localList != NULL )
      {
         // write string to log file
            // function: fputs
         fputs( localList->lineString, logFile );
         
         // iterate to next list node
         localList = localList->nextNode;
      }
      
      // close log file
         // function: fclose
      fclose( logFile );
   }
   
   listHead = clearLogList( listHead );
   
   pcbListHead = clearPCBList( pcbListHead );
   
   memListHead = clearMemoryList( memListHead );
   
   // free allocated memory for PCB and output list
      // function: free
   free( newNode );
   free( newMemNode );
      
   // end program (and display so)
      // function: printf
   printf("-----------------\n\n");
   printf("Simulator End\n");
      
}

/*
Funtion name: clearLogList
Algorithm: recursively iterates through log linked list
           returns memory to OS from the bottom of the list upward
Precondition: clinked list, with or without data
Postcondiditon: all node memory, if any, is returned to OS,
                return pointer (head) is set to null
Exceptions: none
Note: none
*/
fileOutputBuffer *clearLogList( fileOutputBuffer *localPtr )
{
   // check for localPtr not set to null ( list isnt empty )
   if( localPtr != NULL )
   {
      // call for recursive function with next pointer
         // function: clearMetaDataList
      clearLogList( localPtr->nextNode );
      
      // after recursive call, release memory to OS
         // function: free
      free( localPtr );
      
      // set local pointer to null
      localPtr = NULL;
   }   
   // return null to calling function
   
   return NULL;
}

/*
Funtion name: clearPCBList
Algorithm: recursively iterates through PCB linked list
           returns memory to OS from the bottom of the list upward
Precondition: clinked list, with or without data
Postcondiditon: all node memory, if any, is returned to OS,
                return pointer (head) is set to null
Exceptions: none
Note: none
*/
PCB *clearPCBList( PCB *localPtr )
{
   // check for localPtr not set to null ( list isnt empty )
   if( localPtr != NULL )
   {
      // call for recursive function with next pointer
         // function: clearMetaDataList
      clearPCBList( localPtr->nextNode );
      
      // after recursive call, release memory to OS
         // function: free
      free( localPtr );
      
      // set local pointer to null
      localPtr = NULL;
   }   
   // return null to calling function
   
   return NULL;
}

/*
Funtion name: findDisplayCode
Algorithm: decides which display code to return depending on the provided string
Postcondiditon: none
Exceptions: none
Note: none
*/
DisplayCodes findDisplayCode( char *command )
{
   if( compareString( command, "dev" ) == STR_EQ )
   {
      return DEV;
   }
   else
   {
      return CPU;
   }
}


/*
name: showProgramFormat
process: displays command line argument requirements for this program
method input/parameters: none
method output/parameters: none
method output/returned: none
device input/keyboard: none
device output/monitor: instructions provided as specified
dependencies: none
*/
void showProgramFormat()
{
   printf( "Program Format:\n" );
   printf( "      sim_0x [-dc] [rs] config file name\n" );
   printf( "      -dc [optional] displays configuration data\n" );
   printf( "      -dm [optional] displays meta data\n" );
   printf( "      -rs [optional] runs simulator\n" );
   printf( "      config file name is required to run\n" );
}
