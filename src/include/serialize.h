#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__

#include "tcp.h"

int to_data_serialize(to_packet_t * packet);
int to_data_deserialize(to_packet_t * packet);

#endif /* __SERIALIZE_H__ */
