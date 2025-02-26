#  SPDX-License-Identifier: BSD-3-Clause
#  Copyright (c) Intel Corporation.
#  Copyright (c) 2015-2016, Micron Technology, Inc.
#  All rights reserved.
#

SPDK_ROOT_DIR := $(abspath $(CURDIR)/../../../spdk)
DMCACHE_DIR := $(abspath $(CURDIR)/../../dm-cache)
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

FIO_PLUGIN := myfio

C_SRCS := \
    myfio.c \
    $(wildcard ../src/*.c) \
    $(wildcard $(DMCACHE_DIR)/lib/*.c) \

CFLAGS += -I$(abspath $(CURDIR)/../../dm-cache/inc)
CFLAGS += -I$(abspath $(CURDIR)/../src)
CFLAGS += -g -O2

SPDK_LIB_LIST = $(ALL_MODULES_LIST) event event_bdev

include $(SPDK_ROOT_DIR)/mk/spdk.fio.mk