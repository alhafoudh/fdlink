# $HeadURL$
# $Id$

obj-m := flink_dev.o 

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
CFLAGS += -g
LDFLAGS += -g

all:	default flinkapp
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

flinkapp:	flinkapp.o flink.o

clean:
	$(RM) -fr flinkapp *.o *.ko flink_dev.mod.* .*.cmd Module.symvers \
	  .tmp_versions
