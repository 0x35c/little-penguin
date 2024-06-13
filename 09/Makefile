LOGIN := ulayus

obj-m += main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test: all
	rmmod -f main
	insmod main.ko
	mount -t debugfs none /sys/kernel/debug/
	echo -n $(LOGIN) > /sys/kernel/debug/fortytwo/id || true
	cat /sys/kernel/debug/fortytwo/id && echo
	echo "hello world!" > /sys/kernel/debug/fortytwo/foo
	cat /sys/kernel/debug/fortytwo/foo
	cat /sys/kernel/debug/fortytwo/jiffies

.PHONY: all clean test
.SILENT:
