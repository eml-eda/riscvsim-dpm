
all: 0 1 2 14

0:
	make -C 0_how_to_build_a_system_from_scratch gvsoc
	make -C 0_how_to_build_a_system_from_scratch all run

1:
	make -C 1_* gvsoc
	make -C 1_* all run 

2:
	make -C 2_* gvsoc
	make -C 2_* all run
6:
	make -C 6_* gvsoc
	make -C 6_* all run
14:
	make -C 14_* gvsoc
	make -C 14_* all run runner_args="--vcd --event=.*"
	echo "The vcd output can be viewed with gtkwave in  build/view.gtkw"


clean: 
	-rm -r ./*/build 
	-rm -r ./*/__pycache__ 
	make -C ../gvsoc clean

