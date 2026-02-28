#pragma once
#include <sys/types.h>
#include <arpa/inet.h>

ssize_t
init_addr(struct sockaddr_in* addr);
