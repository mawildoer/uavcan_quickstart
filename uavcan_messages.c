/*
 * uavcan_messages.c
 *
 *  Created on: Dec 28, 2020
 *      Author: mawildoer
 */

#include "uavcan_messages.h"

void sendHeartbeat(void) {
  static uint8_t message_transfer_id = 0;
  const uint8_t buffer[100] = {0};

  uavcan_node_Heartbeat_1_0 heartbeat = {
      .health = uavcan_node_Health_1_0_NOMINAL,
      .mode = uavcan_node_Mode_1_0_OPERATIONAL,
      .uptime = 10,
      .vendor_specific_status_code = 1
  };

  size_t msg_size = 100;
  uavcan_node_Heartbeat_1_0_serialize_(&heartbeat, buffer,  &msg_size);

  const CanardTransfer transfer = {
        .timestamp_usec = 0, // Zero if transmission deadline is not limited.
        .priority       = CanardPriorityNominal,
        .transfer_kind  = CanardTransferKindMessage,
        .port_id        = uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, // This is the subject-ID.
        .remote_node_id = CANARD_NODE_ID_UNSET, // Messages cannot be unicast, so use UNSET.
        .transfer_id    = message_transfer_id,
        .payload_size   = msg_size,
        .payload        = buffer
    };

  message_transfer_id++;  // The transfer-ID shall be incremented after every transmission on this subject.
  int32_t result = canardTxPush(&uavcan_canard, &transfer);
}
