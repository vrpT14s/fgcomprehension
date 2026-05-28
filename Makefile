#CXX = g++
#CXX = clang++

EXE = main
IMGUI_DIR = ./imgui
BUILD_DIR= ./build
SOURCES = main.cpp 
SOURCES += app/application.cpp
SOURCES += parse/perfdata.cpp parse/eventconsumer.cpp parse/layout.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++20 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I.
CXXFLAGS += -g -Wall -Wformat
LIBS =

ECHO_MESSAGE = "Linux"
LIBS += $(LINUX_GL_LIBS) -ldl `pkg-config sdl3 --libs`

CXXFLAGS += `pkg-config sdl3 --cflags`
CFLAGS = $(CXXFLAGS)


PARSE_LDFLAGS = $(shell llvm-config --ldflags --libs symbolize support) -lpthread -ldl -lz -lm -fexceptions
PARSE_CXXFLAGS = $(shell llvm-config --cxxflags)
$(BUILD_DIR)/%.o:parse/%.cpp
	$(CXX) $(PARSE_CXXFLAGS) $(CXXFLAGS) -c -o $@ $< $(PARSE_LDFLAGS) 

$(BUILD_DIR)/%.o:app/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	@echo $(OBJS)
	$(CXX) -o $@ $^ $(PARSE_LDFLAGS) $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
