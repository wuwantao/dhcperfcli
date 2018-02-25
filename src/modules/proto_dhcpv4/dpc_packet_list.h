#ifndef _DPC_PACKET_LIST_H
#define _DPC_PACKET_LIST_H

void dpc_packet_list_free(dpc_packet_list_t *pl);
dpc_packet_list_t *dpc_packet_list_create(uint32_t base_id);
RADIUS_PACKET **dpc_packet_list_find_byreply(dpc_packet_list_t *pl, RADIUS_PACKET *reply);

#endif
