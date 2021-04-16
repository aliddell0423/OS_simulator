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
Funtion name: displayAndLog
Algorithm: displays and logs message to linked list depending on the codes specified
Precondition: none
Postcondiditon: none
Exceptions: none
Note: none
*/
void displayAndLog( int loggingFormatFlag, PCB *pcb, 
         fileOutputBuffer *fileOutputHead, DisplayCodes arg1, DisplayCodes arg2 )
{
   // initialize variables
   char timeString[ STD_STR_LEN ];
   char stringBuffer[ STD_STR_LEN ];
   fileOutputBuffer *localPtr = fileOutputHead;
   
   // allocate mem for the new node
   fileOutputBuffer *newNode = (fileOutputBuffer *)malloc( sizeof( fileOutputBuffer ) );

   // check if the first arg is the initializer
   if( arg1 == INIT )
   {
      // init the timer
      accessTimer( 0, timeString );
      
      // record starting msges
      sprintf( stringBuffer, "Running Simulator\n-----------------\n\n%s, OS: Start\n", 
                            timeString );
   }
   // otherwise assume this is a timer access
   else
   {
      accessTimer( 1, timeString );
   }
   
   // start switch statement for the displays
   switch( arg1 )
   {
      case PCB_NEW:
   
         sprintf( stringBuffer, "%s, OS: Process %d set to READY state from "
                        "NEW state\n", timeString, pcb->pid );
   
         break;
         
      case PCB_READY:
   
         sprintf( stringBuffer,
                  "%s, OS: Process %d set from READY to RUNNING\n",
                  timeString, pcb->pid );
      
         break;
      
      case PCB_RUNNING:
      
         sprintf( stringBuffer, "%s, OS: Process %d, set from RUNNING to BLOCKED\n",  
         timeString, pcb->pid );
   
         break;
      
      case PCB_BLOCKED:
      
         sprintf( stringBuffer, "%s, OS: Process %d set from BLOCKED to READY\n", 
         timeString, pcb->pid );
         
         break;
      
      case PCB_EXIT:
         
         sprintf( stringBuffer, "%s, OS: Process %d, ended and set to "
         "EXIT state\n", timeString, pcb->pid );
         
         break;
      
      case PCB_SELECT:
      
         sprintf( stringBuffer, "%s, OS: Process %d selected with %dms remaining\n",  
                  timeString, pcb->pid, pcb->msLeft);
         
         break;
      
      case DEV:
      
         if( arg2 == START)
         {
            sprintf( stringBuffer, "\n%s, OS: Process %d, %s "
            "%sput operation start\n\n", 
            timeString, pcb->pid, 
            pcb->currentOpCode->strArg1,
            pcb->currentOpCode->inOutArg );
         }
         else
         {
            sprintf( stringBuffer, "\n%s, OS: Process %d, %s "
            "%sput operation end\n\n", 
            timeString, pcb->pid, 
            pcb->currentOpCode->strArg1,
            pcb->currentOpCode->inOutArg );
         }
         
         break;
      
      case CPU:
         
         if( arg2 == START )
         {
            sprintf( stringBuffer, "\n%s, OS: Process %d, "
            "cpu process operation start\n", 
            timeString, pcb->pid );
         }
         else
         {
            sprintf( stringBuffer, "%s, OS: Process %d, "
            "cpu process operation end\n", 
            timeString, pcb->pid );
         }
         
         break;
      
      case MEM_START:
      
         sprintf( stringBuffer, "\n%s, OS: Process %d, attempting "
         "mem %s request\n\n", 
         timeString, pcb->pid, pcb->currentOpCode->strArg1 );
         
         break;
      
      case MEM_SUCCESS:
         
         sprintf( stringBuffer, "%s, OS: successful mem %s "
         "request\n", timeString, pcb->currentOpCode->
         strArg1);
         
         break;
      
      case MEM_FAIL:
         
         sprintf( stringBuffer, "%s, OS: Process %d, failed mem %s "
         "request; segmentation fault\n", timeString, pcb->pid,
         pcb->currentOpCode->strArg1 );
         
         break;
   
      case ALL_EXIT:

         sprintf( stringBuffer, "%s, OS: All processes in EXIT state"
          "-  scheduling complete\n", timeString );
          
          break;
      
      case CPU_IDLE:
         
         if( arg2 == START )
         {
            sprintf( stringBuffer, "%s, OS: CPU idle, all active processes blocked\n", 
                     timeString );
         }
         else
         {
            sprintf( stringBuffer, "%s, OS: CPU interrupt, end idle\n",
                                                                           timeString );
         }
         
         break;
      
      case QUANT_INT:
      
         sprintf( stringBuffer, "%s, OS: Process %d, "
         "quantum time out, cpu process operation end\n", 
         timeString, pcb->pid );
         
         break;
      
      case BLOCKED_INT:
      
         sprintf( stringBuffer, "%s, OS: Process %d, "
         "blocked for %sput operation\n",
         timeString, pcb->pid, pcb->currentOpCode->inOutArg );
         
         break;
      
      case FINISH_INT:
      
         sprintf( stringBuffer, "%s, OS: Interrupted by Process %d %s "
         "%sput operation\n", 
         timeString, pcb->pid, pcb->currentOpCode->strArg1, 
         pcb->currentOpCode->inOutArg );
         
         break;
         
      case START:
         printf("Display Error; request not handled: CASE: START\n");
         break;
      case FINISH:
         printf("Display Error; request not handled: CASE: FINISH\n");
         break;
      case NONE:
         break;
      case INIT:
         break;

   }
   // check if we want to print to monitor, and if so, print
   if( loggingFormatFlag == LOGTO_MONITOR_CODE ||
         loggingFormatFlag == LOGTO_BOTH_CODE )
   {
      printf( stringBuffer );
   }
   
   if( stringBuffer != NULL )
   {
      // set the new node's string
      copyString( newNode->lineString, stringBuffer );
      
      // add this node to the linked list of strings for file output
      addListNode( localPtr, newNode );
   }
   
   // free new node memory
   free( newNode );
   
}

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
fileOutputBuffer *addListNode( fileOutputBuffer *localPtr, 
                                    fileOutputBuffer *newNode )
{
   // check for local pointer assigned to null
   if( localPtr == NULL )
   {
      // access new memory for new link/node
         // function: malloc
      localPtr = (fileOutputBuffer *)malloc( sizeof( fileOutputBuffer ) );
         
      // assign all strings to node
      // assign next pointer to null
         // function: copyString
      copyString( localPtr->lineString, newNode->lineString );
      
      localPtr->nextNode = NULL;
         
      // return current local pointer
      return localPtr;
   }  
   // assume end of list is not found yet
   // assign recursive function to current's next link
      // function: addNode
   localPtr->nextNode = addListNode( localPtr->nextNode, newNode );
      
   // return local pointer   
   return localPtr;
}
