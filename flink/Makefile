obj-m := flink_dev.o 

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

all:	default flinktest
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

flinktest:	flinktest.o flink.o

clean:
	$(RM) -fr flinktest *.o *.ko flink_dev.mod.* .*.cmd Module.symvers \
	  .tmp_versions
