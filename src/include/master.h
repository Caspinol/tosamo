#ifndef __MASTER_H__
#define __MASTER_H__

#include <pthread.h>

#include "settings.h"
#include "tcp.h"
#include "timed.h"
#include "config.h"
#include "lists.h"
#include "log.h"

void master_monitor_files(void);

#endif /* __MASTER_H__ */
