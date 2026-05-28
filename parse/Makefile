CXX = g++
CXXFLAGS = -g -O2 $(shell llvm-config --cxxflags) -std=c++20
LDFLAGS = $(shell llvm-config --ldflags --libs symbolize support) -lpthread -ldl -lz -lm -fexceptions

EXE = main
SOURCES = main.cpp
SOURCES += perfdata.cpp eventconsumer.cpp layout.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

run: main
	./main perf.data

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

gdb: main
	gdb --args ./main perf.data

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm $(EXE) $(OBJS)
