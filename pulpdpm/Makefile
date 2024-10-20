
SHELL := /bin/bash
BUILDDIR = $(CURDIR)/build
RT_SRCS = 
RT_FLAGS =
APP              = test
# App sources
APP_SRCS         =  $(CURDIR)/main.c


.PHONY: app

clean:
	rm -f *.o $(BUILDDIR)/test
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) clean

app:
	make -C app clean
	make -C app APP_SRCS=$(APP_SRCS)

	

gvsoc:
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) build
	g++ -o launcher launcher.cpp -I../gvsoc/core/engine/include -L../gvsoc/install/lib -lpulpvp


config:
	gvsoc --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=$(CURDIR)/app/BUILD/PULP/GCC_RISCV/test/test prepare $(runner_args) --trace=pm  --vcd --event=.*
run:
	gvsoc --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=$(CURDIR)/app/BUILD/PULP/GCC_RISCV/test/test image flash  run  $(runner_args) --vcd --event=.* --trace=pm

run_launcher:
	LD_LIBRARY_PATH=$(CURDIR)/../gvsoc/install/lib:$(LD_LIBRARY_PATH) ./launcher --config=build/gvsoc_config.json

open_vcd:
	gtkwave waves.gtkw