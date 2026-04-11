MAKEFLAGS += --no-print-directory

FILE_EXT := cpp
BUILD_DIR := build

CXX := clang++
CXXFLAGS := -O3 -I.
LDFLAGS =
LINKS := -lraylib
TARGET ?= linux
EXE ?= app

ifeq ($(OS),Windows_NT)
	LDFLAGS += -lopengl32 -lgdi32 -lwinmm -static -static-libgcc -static-libstdc++
	EXE := app.exe
else
	ifeq ($(TARGET),windows)
		CXX := x86_64-w64-mingw32-g++
        LDFLAGS += -lopengl32 -lgdi32 -lwinmm -L.
		EXE := app.exe
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			LDFLAGS += -lGL -lm -lpthread -ldl -lrt -lX11 -fsanitize=address
			CXXFLAGS += -g -fsanitize=address
		endif
		EXE := app
	endif
endif

SRCS := $(shell find . -not -path '*/.*' -name "*.$(FILE_EXT)")
OBJS := $(addprefix build/, $(notdir $(SRCS:.$(FILE_EXT)=.o)))
VPATH := $(sort $(dir $(SRCS)))

all: build $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LINKS)

$(BUILD_DIR)/%.o: %.$(FILE_EXT) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(EXE)
