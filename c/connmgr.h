#pragma once
#include <stddef.h>
#include "netutils.h"
#define CONNMGR_MAXCONN 8

struct connmgr {
    struct socketinfo* sockets;
    size_t open;
    size_t max;
};

struct connmgr
init_connmgr(size_t connections);
