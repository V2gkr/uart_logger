#include "Logger.h"
#include "BSP_Logger.h"
//#include "FreeRTOS.h"
//#include "task.h"
#include "cmsis_os.h"
#include "string.h"


#define BUF_SIZE    256 /* Size of the circular buffer */ 
#define PREFIX_SIZE 9   /* size of a message descriptor*/

uint8_t CircBuf[BUF_SIZE]={0}; /* Circular buffer for USART debug messages */ 
uint32_t CircBufHead = 0; /* Head index for the circular buffer */
uint32_t CircBufTail = 0; /* Tail index for the circular buffer */ 

/* transmission buffer , non circular*/
uint8_t TransmitBuf[40];

volatile uint8_t task1msgcount=0;
volatile uint8_t task2msgcount=0;

extern osMutexId_t LoggerMutexHandle;
extern osMessageQueueId_t LoggerQueueHandle;
extern osSemaphoreId_t LoggerSemHandle;

/* message descriptors */
const char * const logPrefix[]={"[error ]:",
                                "[status]:",
                                "[ info ]:",
                                "[ data ]:",
                                "unknown: "};
const char * const task2comp={"[status]:task2blabla"};
const char * const task1comp={"[ data ]:task1belbel"};
/** @brief: function to get a num of bytes in queue data buffer 
 *  @retval: num of bytes 
*/
static uint8_t LoggerGetBufSize(void){
  if(CircBufHead>=CircBufTail)
    return CircBufHead-CircBufTail;
  else
    return BUF_SIZE-CircBufTail+CircBufHead;
}

/** @brief: function to place a message in a send queue
 *  @param: str - pointer to a data to place in a queue
 *  @param: len - lenght of data to place in a queue
 *  @param: type - type of descriptor to add to a message
 *  @retval: 0 - data is placed in a queue , 1 - data is not placed
*/
uint8_t LoggerSend(const char *str, uint8_t len,LogType type) {
  if(str==NULL || len == 0) {
    return 1; // Error: null string or zero length
  }
  if(type>LOG_DATA)
    type=LOG_UNKNOWN;
  LogDesctiptor msg={.type=type};
  uint8_t OccupiedBufSize=LoggerGetBufSize();
  if((BUF_SIZE-OccupiedBufSize-1)<len)
    return 1;
  if(osMutexAcquire(LoggerMutexHandle,0)==osOK){
    msg.size=len;
    if(osMessageQueuePut(LoggerQueueHandle,&msg,0,0)==osOK){
      while(len>0){
        CircBuf[CircBufHead]=*str++;
        CircBufHead++;
        if(CircBufHead==BUF_SIZE)
          CircBufHead=0;
        len--;
      }
    }
    osMutexRelease(LoggerMutexHandle);
    return 0; 
  }
  return 1;
}

/** @brief: function to transmit a single message */
void LoggerTransmit(void){
  LogDesctiptor msg;
  if(osSemaphoreAcquire(LoggerSemHandle,0)==osOK){
    if(osMessageQueueGetCount(LoggerQueueHandle)){
      if(osMutexAcquire(LoggerMutexHandle,0)==osOK){
        osMessageQueueGet(LoggerQueueHandle,&msg,NULL,0);
        memcpy(TransmitBuf,logPrefix[msg.type],PREFIX_SIZE);
        for(uint8_t i=0;i<msg.size;i++){
          TransmitBuf[PREFIX_SIZE+i]=CircBuf[CircBufTail++];
          if(CircBufTail==BUF_SIZE)
            CircBufTail=0;
        }
        if(memcmp(TransmitBuf,task1comp,20)!=0 && msg.type==LOG_DATA){
          task1msgcount++;
        }
        if(memcmp(TransmitBuf,task2comp,20)!=0 && msg.type==LOG_STATUS){
          task2msgcount++;
        }
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

