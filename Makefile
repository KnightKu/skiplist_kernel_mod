obj-m += skiplist_test.o
all:
	make -C /usr/src/kernels/2.6.32-504.16.2.el6_lustre.2.5.37.ddn2.x86_64/ M=$(PWD) modules
clean:
	make -C /usr/src/kernels/2.6.32-504.16.2.el6_lustre.2.5.37.ddn2.x86_64/ M=$(PWD) clean
