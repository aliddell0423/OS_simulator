// Pre-compiler directive
#ifndef METADATAOPS_H
#define METADATAOPS_H

// header files
#include <stdio.h> // file operations
#include <stdlib.h> // dynamic memory operations
#include "StringUtils.h"

// op code messages (starts at index 3)
typedef enum { MD_FILE_ACCESS_ERROR = 3,
               MD_CORRUPT_DESCRIPTOR_ERR,
               OPCMD_ACCESS_ERR,
               CORRUPT_OPCMD_ERR,
               CORRUPT_OPCMD_ARG_ERR,
               UNBALANCED_START_END_ERR,
               COMPLETE_OPCMD_FOUND_MSG,
               LAST_OPCMD_FOUND_MSG } OpCodeMessages;

// create global constants - across files
extern const int BAD_ARG_VAL;

// OpCodeType struct
typedef struct OpCodeType
{
   int pid;
   char command [ 5 ];
   char inOutArg [ 5 ];
   char strArg1 [ 15 ];
   int intArg2;
   int intArg3;

   double opEndTime;
   struct OpCodeType *nextNode;
} OpCodeType;

// function prototypes
OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode );
OpCodeType *clearMetaDataList( OpCodeType *localPtr );
void displayMetaData( OpCodeType *localPtr );
int getCommand( char *cmd, char *inputStr, int index );
Boolean getMetaData(char *fileName,
                               OpCodeType **opCodeDataHead, char *endStateMsg );
int getNumberArg( int *number, char *inputStr, int index );
int getOpCommand( FILE *filePtr, OpCodeType *inData );
int getStringArg( char *strArg, char *inputStr, int index );
Boolean isDigit( char testChar );
int updateStartCount( int count, char *opString );
int updateEndCount( int count, char *opString );
Boolean verifyFirstStringArg( char *strArg );
Boolean verifyValidCommand( char *testCmd );

#endif   // METADATAOPS_H
