/*
 * uavcan.h
 *
 *  Created on: Dec 27, 2020
 *      Author: mawildoer
 */


#ifndef __UAVCAN_H
#define __UAVCAN_H

#ifdef __cplusplus
extern "C" {
#endif

#define UAVCAN_HEAP_SIZE 4096

#include "canard.h"
#include "o1heap.h"
#include "stm32g4xx_hal.h"

extern CanardInstance uavcan_canard;

void uavcanInit(FDCAN_HandleTypeDef* lcan_ins);

void uavcanSetNodeID(CanardNodeID id);
void uavcanCanardProcess(void);

#ifdef __cplusplus
}
#endif

#endif /* __UAVCAN_H */
