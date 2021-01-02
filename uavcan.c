/*
 * uavcan.c
 *
 *  Created on: Dec 27, 2020
 *      Author: mawildoer
 */

#include "uavcan.h"

static O1HeapInstance* uavcan_heap;
CanardInstance uavcan_canard;
static uint8_t uavcan_heap_base[UAVCAN_HEAP_SIZE] __attribute__ ((aligned (O1HEAP_ALIGNMENT)));
static FDCAN_HandleTypeDef* can_ins;

static int32_t uavcanLengthToDlc(size_t length) {
  return CanardCANLengthToDLC[length] << 16;
}

static size_t uavcanDlcToLength(uint32_t st_dlc) {
  return CanardCANDLCToLength[st_dlc >> 16];
}

static void* uavcanMemAllocate(CanardInstance* const ins, const size_t amount) {
  (void) ins;
  return o1heapAllocate(uavcan_heap, amount);
}

static void uavcanMemFree(CanardInstance* const ins, void* const pointer) {
  (void) ins;
  o1heapFree(uavcan_heap, pointer);
}

void uavcanInit(FDCAN_HandleTypeDef* lcan_ins) {
  uavcan_canard = canardInit(&uavcanMemAllocate, &uavcanMemFree);
  uavcan_heap = o1heapInit(uavcan_heap_base, UAVCAN_HEAP_SIZE, NULL, NULL);
  can_ins = lcan_ins;
}

void uavcanSetNodeID(CanardNodeID id) {
  uavcan_canard.node_id = id;
}

static bool canTx(const CanardFrame* txf) {
  const FDCAN_TxHeaderTypeDef TxHeader = {
      .Identifier = txf->extended_can_id,
      .IdType = FDCAN_EXTENDED_ID,
      .TxFrameType = FDCAN_DATA_FRAME,
      .DataLength = uavcanLengthToDlc(txf->payload_size),
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_ON,
      .FDFormat = FDCAN_FD_CAN,
      .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
      .MessageMarker = 0
  };

  return (HAL_FDCAN_AddMessageToTxFifoQ(can_ins, &TxHeader, txf->payload) == HAL_OK);
}

void uavcanCanardProcess(void) {
  for (const CanardFrame *txf = NULL; (txf = canardTxPeek(&uavcan_canard)) != NULL;) { // Look at the top of the TX queue.
      if (txf->timestamp_usec > HAL_GetTick() * 1000 || txf->timestamp_usec == 0) { // Ensure TX deadline not expired.
        if (!(canTx(txf))) { // Send the frame. Redundant interfaces may be used here.
          break;                // If the driver is busy, break and retry later.
        }
      }
      canardTxPop(&uavcan_canard); // Remove the frame from the queue after it's transmitted.
      uavcan_canard.memory_free(&uavcan_canard, (CanardFrame*) txf); // Deallocate the dynamic memory afterwards.
    }
}
