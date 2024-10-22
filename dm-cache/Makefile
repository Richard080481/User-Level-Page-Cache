# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2015 Intel Corporation.
# All rights reserved.
#

SPDK_ROOT_DIR := $(abspath $(CURDIR)/../../spdk)
include $(SPDK_ROOT_DIR)/mk/spdk.common.mk
include $(SPDK_ROOT_DIR)/mk/spdk.modules.mk

APP = app

C_SRCS := \
$(abspath $(CURDIR)/testing.c) \
$(abspath $(CURDIR)/lib/pio.c) \
$(abspath $(CURDIR)/lib/shm.c) \
$(abspath $(CURDIR)/lib/spdk.c) \
$(abspath $(CURDIR)/lib/atomic.c) \
$(abspath $(CURDIR)/lib/work_queue.c) \
$(abspath $(CURDIR)/lib/ssd_cache_api.c)  \
$(abspath $(CURDIR)/lib/ssd_cache_target.c) \
$(abspath $(CURDIR)/lib/ssd_cache_mapping.c) \


# Ensure to generate object files from C source files
C_OBJS := $(C_SRCS:.c=.o)

CFLAGS += -I$(abspath $(CURDIR)/inc)
CFLAGS += -g -O2

SPDK_LIB_LIST = $(ALL_MODULES_LIST) event event_bdev

include $(SPDK_ROOT_DIR)/mk/spdk.app.mk
