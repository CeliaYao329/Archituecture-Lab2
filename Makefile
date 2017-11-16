# build Simulator
all: Simulator.cpp ReadElf.cpp
	g++ -g Simulator.cpp ReadElf.cpp -o Simulator
clean:
	$(RM) Simulator
