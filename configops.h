// Pre-compiler directive
#ifndef CONFIGOPS_H
#define CONFIGOPS_H

// header files
#include <stdio.h> // file operations
#include <stdlib.h> // dynamic memory operations
#include "StringUtils.h"

// create global constants - across files
typedef enum { CFG_FILE_ACCESS_ERR = 3,
               CFG_CORRUPT_DESCRIPTION_ERR,
               CFG_DATA_OUT_OF_RANGE_ERR,
               CFG_CORRUPT_PROMPT_ERR,
               CFG_VERSION_CODE,
               CFG_MD_FILE_NAME_CODE,
               CFG_CPU_SCHED_CODE,
               CFG_QUANT_CYCLES_CODE,
               CFG_MEM_AVAILABLE_CODE,
               CFG_PROC_CYCLES_CODE,
               CFG_IO_CYCLES_CODE,
               CFG_LOG_TO_CODE,
               CFG_LOG_FILE_NAME_CODE } ConfigCodeMessages;
               
typedef enum { CPU_SCHED_SJF_N_CODE,
               CPU_SCHED_SRTF_P_CODE,
               CPU_SCHED_FCFS_P_CODE,
               CPU_SCHED_RR_P_CODE,
               CPU_SCHED_FCFS_N_CODE } ConfigCpuDataCodes;
               
typedef enum { LOGTO_MONITOR_CODE,
               LOGTO_FILE_CODE,
               LOGTO_BOTH_CODE } ConfigLogDataCodes;

// config data structure
typedef struct
{
   double version;
   char metaDataFileName[ 100 ];
   int cpuSchedCode;
   int quantumCycles;
   int memAvailable;
   int procCycleRate;
   int ioCycleRate;
   int logToCode;
   char logToFileName[ 100 ];
} ConfigDataType;

// function prototypes
ConfigDataType *clearConfigData( ConfigDataType *configData );
void configCodeToString( int code, char *outString );
void displayConfigData( ConfigDataType *configData );
Boolean getConfigData(char *fileName,
                               ConfigDataType **configData, char *endStateMsg );
ConfigCpuDataCodes getCpuSchedCode( char *codeStr );
int getDataLineCode( char *dataBuffer );
ConfigLogDataCodes getLogToCode( char *logToStr );
Boolean valueInRange( int lineCode, int intVal,
                                       double doubleVal, char *stringVal );


#endif   // CONFIGOPS_H