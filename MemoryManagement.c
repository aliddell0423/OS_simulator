#include "metadataops.h"
#include "configops.h"
#include "simulator.h"
#include "MemoryManagement.h"

/*
Funtion name: addMemoryNode
Algorithm: adds MemoryBlock structure with data to a linked list
Precondition: linked list pointer assigned to null or to one MemoryBlock link,
              struct pointer assigned to MemoryBlock struct data
Postcondiditon: assigns new structure node to beginning of linked list
                or and of established linked list
Exceptions: none
Note: assumes memory access/availability
*/
MemoryBlock *addMemoryNode( MemoryBlock *localPtr, MemoryBlock *newNode )
{
   // check for local pointer assigned to null
   if( localPtr == NULL )
   {
      // access new memory for new link/node
         // function: malloc
      localPtr = (MemoryBlock *)malloc( sizeof( MemoryBlock ) );
         
      // assign all three values to newly created node
      // assign next pointer to null
      localPtr->upperBound = newNode->upperBound;
      localPtr->lowerBound = newNode->lowerBound;
      localPtr->designatedPCB = newNode->designatedPCB;
      
      localPtr->nextNode = NULL;
         
      // return current local pointer
      return localPtr;
   }  
   // assume end of list is not found yet
   // assign recursive function to current's next link
      // function: addMemoryNodeNode
   localPtr->nextNode = addMemoryNode( localPtr->nextNode, newNode );
      
   // return local pointer   
   return localPtr;
}

/*
Funtion name: allocateMem
Algorithm: runs through the linked list of bounds to make sure
           there is no overlapping of allocations
Precondition: there are correct values for the memory
              block node
Postcondiditon: return the state of whether a seg fault occured or not
Exceptions: none
Note: none
*/
Boolean allocateMem( MemoryBlock *newMemNode, MemoryBlock **memListHead, 
                                                ConfigDataType *configDataPtr )
{
   // initialize variables
   MemoryBlock *localPtr = NULL;
   Boolean segFault = False;
   
   // initialize local pointer to list head
   localPtr = *memListHead;
   
   // check if the upper bound exceeds the configured memory limits
   if( newMemNode->upperBound > configDataPtr->memAvailable
       || newMemNode->lowerBound < 0 )
   {
      // if so, that is a seg fault
      segFault = True;
   }
   
   // check if the local pointer is null
   else if( localPtr == NULL )
   {
      // set list head to the first memory block created
         // function: addMemoryNode
      *memListHead = addMemoryNode( localPtr, newMemNode );
   }
   
   // otherwise assume that there are already mem allocs
   else
   {
      // loop through all memory blocks
      while( localPtr != NULL && segFault == False )
      {
         // check if the bounds of the new node do not overlap local pointer
         //    also check if said pcb that allocated memory is in running state
         //    or blocked state
         if( !( ( newMemNode->upperBound > localPtr->upperBound && 
               newMemNode->lowerBound > localPtr->upperBound ) ||
             ( newMemNode->upperBound < localPtr->lowerBound && 
               newMemNode->lowerBound < localPtr->lowerBound ) ) &&
             ( compareString( localPtr->designatedPCB->processState, "EXIT") != STR_EQ ) )
         {
            // if so, that is a seg fault
            segFault = True;
         }
         
         // iterate to the next memory block
         localPtr = localPtr->nextNode;
      }
      // end loop
      
      // check if there was no seg fault detected
      if( segFault == False )
      {
         // if not, add the memory block to the linked list
            // function: addMemoryNode
         localPtr = addMemoryNode( *memListHead, newMemNode );
      }
   }
   
   return segFault;
}

/*
Funtion name: accessMem
Algorithm: runs through the linked list of bounds to make sure
           there is proper allocated memory
Precondition: there are correct values for the memory
              block node
Postcondiditon: return the state of whether a seg fault occured or not
Exceptions: none
Note: none
*/
Boolean accessMem( int lowerBound, int upperBound, MemoryBlock *memListHead,
                                ConfigDataType *configDataPtr, int pidOfAccess )
{
   // initialize variables
   Boolean segFault = False;
   MemoryBlock *localPtr = NULL;
   
   // set the local pointer to the list head
   localPtr = memListHead;
   
   // check if the allocation is beyond limits
   if( lowerBound < 0 || upperBound > configDataPtr->memAvailable )
   {
      // if so, that is a seg fault
      segFault = True;
   }
   
   // otherwise, assume possibility of allocated mem
   else
   {
      // unless specified otherwise, there is a seg fault
      segFault = True;
      
      // loop through memory blocks
      while( localPtr != NULL && segFault == True )
      {
         // check if the access is within allocation limits
         if( upperBound <= localPtr->upperBound &&
         lowerBound >= localPtr->lowerBound &&
         compareString( localPtr->designatedPCB->processState, "EXIT" ) != STR_EQ &&
         localPtr->designatedPCB->pid == pidOfAccess )
         {
            // if so, there available allocated memory
            segFault = False;
         }
         
         // iterate to the next memory block
         localPtr = localPtr->nextNode;
      }
   }
   
   return segFault;
}

/*
Funtion name: clearMemoryList
Algorithm: recursively iterates through memory linked list
           returns memory to OS from the bottom of the list upward
Precondition: clinked list, with or without data
Postcondiditon: all node memory, if any, is returned to OS,
                return pointer (head) is set to null
Exceptions: none
Note: none
*/
MemoryBlock *clearMemoryList( MemoryBlock *localPtr )
{
   // check for localPtr not set to null ( list isnt empty )
   if( localPtr != NULL )
   {
      // call for recursive function with next pointer
         // function: clearMetaDataList
      clearMemoryList( localPtr->nextNode );
      
      // after recursive call, release memory to OS
         // function: free
      free( localPtr );
      
      // set local pointer to null
      localPtr = NULL;
   }   
   // return null to calling function
   
   return NULL;
}
