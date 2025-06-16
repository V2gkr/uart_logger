#ifndef __BSP_LOGGER_H
#define __BSP_LOGGER_H

#include "stm32g4xx_hal.h"

void BSP_LoggerTransmit(const uint8_t * buf,uint8_t size);

#endif