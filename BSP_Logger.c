#include "cmsis_os.h"
#include "BSP_Logger.h"

extern UART_HandleTypeDef huart2;
extern osSemaphoreId_t LoggerSemHandle;


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
  if(huart==&huart2){
    osSemaphoreRelease(LoggerSemHandle);
  }
}

void BSP_LoggerTransmit(const uint8_t * buf,uint8_t size){
  HAL_UART_Transmit_DMA(&huart2,buf,size);
}