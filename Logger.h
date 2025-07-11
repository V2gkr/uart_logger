#ifndef __LOGGER_H
#define __LOGGER_H

/** @brief: driver for printing debug messages by USART
    multithread access supported
*/
#include <stdint.h>

#define QUEUE_MAX_SIZE 20

typedef enum __attribute__((packed)){
  LOG_ERROR=0,
  LOG_STATUS=1,
  LOG_INFO=2,
  LOG_DATA=3,
  LOG_UNKNOWN=4
}LogType;


typedef struct{
  LogType type;
  uint8_t size;
  uint8_t payload[QUEUE_MAX_SIZE];
}LogDesctiptor;

/** @brief: function copies data to local buffer , transmission is delayed */
uint8_t LoggerSend(const char *str, uint8_t len,LogType type);

/** @brief: function that sends data from local buffer  */
void LoggerTransmit(void);

#endif /* __LOGGER_H */