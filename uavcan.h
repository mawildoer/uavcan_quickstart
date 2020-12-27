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

#include "canard.h"
#include "o1heap.h"
#include "stm32g4xx_hal_fdcan.h"

typedef void (*UavcanTx)(const CanardFrame* ins);

int32_t UavcanLengthToDlc(size_t length);
size_t UavcanDlcToLength(uint32_t st_dlc);

typedef struct UavcanInstance {
  const O1HeapInstance* heap;
  const CanardInstance* canard;

  UavcanTx canTxFunc;

  uint8_t message_transfer_id;
} UavcanInstance;

UavcanInstance uavcanInit(const size_t heap_size);

void uavcanSetNodeID(UavcanInstance* ins, uint16_t id);

#ifdef __cplusplus
}
#endif

#endif /* __UAVCAN_H */
