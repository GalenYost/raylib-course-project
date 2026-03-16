MAKEFLAGS += --no-print-directory

FILE_EXT := cpp
BUILD_DIR := build

CXX := clang++
FLAGS := -O3 -Iobjects -I. -Iinclude
LINKS := -lraylib
TARGET ?= linux
EXE ?= app

ifeq ($(OS),Windows_NT)
	LINKS := -lopengl32 -lgdi32 -lwinmm -L./lib
	EXE := app.exe
else
	ifeq ($(TARGET),windows)
		CXX := x86_64-w64-mingw32-g++
        LINKS += -lopengl32 -lgdi32 -lwinmm -L./lib
		EXE := app.exe
	else
		UNAME_S := $(shell uname -s)
		ifeq ($(UNAME_S),Linux)
			LINKS += -lGL -lm -lpthread -ldl -lrt -lX11
		endif
		EXE := app
	endif
endif

SRCS := $(shell find . -not -path '*/.*' -name "*.$(FILE_EXT)")
OBJS := $(addprefix build/, $(notdir $(SRCS:.$(FILE_EXT)=.o)))
VPATH := $(sort $(dir $(SRCS)))

all: build $(EXE)

$(EXE): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LINKS)

$(BUILD_DIR)/%.o: %.$(FILE_EXT) | build
	$(CXX) $(FLAGS) -c $< -o $@

build:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(EXE)
