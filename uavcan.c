/*
 * uavcan.c
 *
 *  Created on: Dec 27, 2020
 *      Author: mawildoer
 */

#include "uavcan.h"


int32_t UavcanLengthToDlc(size_t length) {
  return CanardCANLengthToDLC[length] << 16;
}

size_t UavcanDlcToLength(uint32_t st_dlc) {
  return CanardCANDLCToLength[st_dlc >> 16];
}

void* uavcanMemAllocate(CanardInstance* const ins, const size_t amount)
{
    return o1heapAllocate(((UavcanInstance*)(ins->user_reference))->heap, amount);
}

void uavcanMemFree(CanardInstance* const ins, void* const pointer)
{
    o1heapFree(((UavcanInstance*)(ins->user_reference))->heap, pointer);
}

UavcanInstance uavcanInit(const size_t heap_size, size_t mtu_bytes) {
  uint8_t heap_base[heap_size] __attribute__ ((aligned (O1HEAP_ALIGNMENT)));
  const CanardInstance canard = canardInit(&uavcanMemAllocate, &uavcanMemFree);

  UavcanInstance ins = {
      .heap = o1heapInit(heap_base, heap_size, NULL, NULL),
      .canard = &canard,
      .message_transfer_id = 0
  };

  ins.canard->user_reference = (void*)&ins;
  ins.canard->mtu_bytes = mtu_bytes;
}

void uavcanSetNodeID(UavcanInstance* ins, CanardNodeID id) {
  ins->canard->node_id = id;
}

void uavcanCanardProcess(const UavcanInstance* ins) {
  for (const CanardFrame *txf = NULL; (txf = canardTxPeek(ins->canard)) != NULL;) { // Look at the top of the TX queue.
      if (txf->timestamp_usec > getCurrentMicroseconds() || txf->timestamp_usec == 0) { // Ensure TX deadline not expired.
        if (!(ins->canTxFunc(txf))) { // Send the frame. Redundant interfaces may be used here.
          break;                // If the driver is busy, break and retry later.
        }
      }
      canardTxPop(ins->canard); // Remove the frame from the queue after it's transmitted.
      uavcan_ins.memory_free(&uavcan_ins, (CanardFrame*) txf); // Deallocate the dynamic memory afterwards.
    }
}
