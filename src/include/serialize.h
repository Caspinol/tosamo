#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

#include "tcp.h"
#include "lists.h"
#include "crc.h"

int to_data_deserialize(to_packet_t *);
int to_data_serialize(to_packet_t *, L_HEAD *);

#endif /* __SERIALIZE_H__ */
