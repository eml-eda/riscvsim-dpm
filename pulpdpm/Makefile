
SHELL := /bin/bash
BUILDDIR = $(CURDIR)/build
RT_SRCS = 
RT_FLAGS =
APP              = test
# App sources
APP_SRCS         = main.c

clean:
	rm -f *.o $(BUILDDIR)/test
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) clean

all:
	docker run -v $(CURDIR):/messy messy:latest  make -C app
	

gvsoc:
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) build

run:
	gvsoc --verbose debug --py-stack --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=/home/lorenzo/Documents/EMIOT_PROJECT/riscvsim-dpm/pulpdpm/app/BUILD/PULP/GCC_RISCV/test/test  image flash run $(runner_args) --vcd --event=.* --trace=pm --trace=axi_proxy --trace=axi_ico 