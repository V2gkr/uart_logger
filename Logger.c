#include "Logger.h"
#include "BSP_Logger.h"
#include "cmsis_os.h"
#include "string.h"


#define PREFIX_SIZE 9   /* size of a message descriptor*/


/* transmission buffer , non circular*/
uint8_t TransmitBuf[40];

extern osMutexId_t LoggerMutexHandle;
extern osMessageQueueId_t LoggerQueueHandle;
extern osSemaphoreId_t LoggerSemHandle;

/* message descriptors */
const char * const logPrefix[]={"[error ]:",
                                "[status]:",
                                "[ info ]:",
                                "[ data ]:",
                                "unknown: "};

/** @brief: function to place a message in a send queue
 *  @param: str - pointer to a data to place in a queue
 *  @param: len - lenght of data to place in a queue
 *  @param: type - type of descriptor to add to a message
 *  @retval: 0 - data is placed in a queue , 1 - data is not placed
*/
uint8_t LoggerSend(const char *str, uint8_t len,LogType type) {
  LogDesctiptor msg;
  if(str==NULL || len == 0)
    return 1; // Error: null string or zero length
  if(type>LOG_DATA)
    type=LOG_UNKNOWN;
  msg.type=type;
  if(osMutexAcquire(LoggerMutexHandle,0)!=osOK)
    return 1;
  if(len<=QUEUE_MAX_SIZE)
    msg.size=len;
  else
    msg.size=QUEUE_MAX_SIZE;
  memcpy(&msg.payload,str,msg.size);
  if(osMessageQueuePut(LoggerQueueHandle,&msg,0,0)!=osOK)
    return 1;
  osMutexRelease(LoggerMutexHandle);
  return 0; 

}

/** @brief: function to transmit a single message */
void LoggerTransmit(void){
  if(osSemaphoreAcquire(LoggerSemHandle,0)==osOK){
    if(osMessageQueueGetCount(LoggerQueueHandle)){
      if(osMutexAcquire(LoggerMutexHandle,0)==osOK){
        LogDesctiptor msg;
        osMessageQueueGet(LoggerQueueHandle,&msg,NULL,0);
        memcpy(TransmitBuf,logPrefix[msg.type],PREFIX_SIZE);
        memcpy((TransmitBuf+PREFIX_SIZE),&msg.payload,msg.size);
        BSP_LoggerTransmit(TransmitBuf,PREFIX_SIZE+msg.size);
        osSemaphoreRelease(LoggerSemHandle);
        osMutexRelease(LoggerMutexHandle);
      }
      else
        osSemaphoreRelease(LoggerSemHandle);
    }
    else
      osSemaphoreRelease(LoggerSemHandle);
  }
}

