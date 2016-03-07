
#ifndef __SLAVE_H__
#define __SLAVE_H__

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tcp.h"
#include "serialize.h"
#include "config.h"
#include "settings.h"
#include "utils.h"
#include "log.h"

void slave_handle_updates(void);

#endif /* __SLAVE_H__ */
