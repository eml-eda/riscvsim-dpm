
SHELL := /bin/bash
BUILDDIR = $(CURDIR)/build
RT_SRCS = 
RT_FLAGS =
APP              = test

# App sources
ifdef SOURCE
    APP_SRCS := "$(CURDIR)/examples/$(SOURCE)"
else
	APP_SRCS  =  "$(CURDIR)/examples/main.c"
endif

.PHONY: help app

help:
	@echo "Makefile Help:"
	@echo "clean: clean all, including gvsoc build directory and builded firmware"
	@echo "config: Prepares GVSoC config file with tracing and VCD enabled. Other arguments can be added using the runner_args variable, e.g. runner_args="--trace=fc/comp""
	@echo "config_notrace: Prepares the build directory and configures gvsoc with VCD enabled but without tracing."
	@echo "run: Runs gvsoc GVSoC without the launcher, the communication between the PULP core and the PowerManager component will not work."
	@echo "run_launcher: Run GVSoC using the simple developed launcher, that forward the communication from the PULP core to the PowerManager and viceversa."
	@echo "open_vcd: Opens the GTKWave tool with the generated vcd file."

clean:
	rm -f *.o $(BUILDDIR)/test
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) clean

app:
	make -C app clean
	echo "compiling $(APP_SRCS)"
	make -C app APP_SRCS=$(APP_SRCS)

	

gvsoc:
	mkdir -p build
	make -C ../gvsoc TARGETS=my_system MODULES=$(CURDIR) build
	g++ -g -o launcher launcher.cpp -I../gvsoc/core/engine/include -L../gvsoc/install/lib -lpulpvp


config:
	mkdir -p build
	gvsoc --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=$(CURDIR)/app/BUILD/PULP/GCC_RISCV/test/test prepare $(runner_args) --trace=pm  --vcd --event=.*

config_notrace:
	mkdir -p build
	gvsoc --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=$(CURDIR)/app/BUILD/PULP/GCC_RISCV/test/test prepare --vcd 

run:
	gvsoc --target-dir=$(CURDIR) --target=my_system --work-dir=$(BUILDDIR) --binary=$(CURDIR)/app/BUILD/PULP/GCC_RISCV/test/test image flash  run  $(runner_args) --vcd --event=.* --trace=pm

run_launcher:
	LD_LIBRARY_PATH=$(CURDIR)/../gvsoc/install/lib:$(LD_LIBRARY_PATH) ./launcher --config=build/gvsoc_config.json

open_vcd:
	gtkwave waves.gtkw

