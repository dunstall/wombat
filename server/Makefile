INC_DIR := include
SRC_DIR := src
OBJ_DIR := build
LIB_DIR := lib

SRC_FILES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS += -std=c++17  -Wall -Werror -MMD
INC_FLAGS := -I$(INC_DIR) -I../log/$(INC_DIR)
LDFLAGS := -L../log -lwombatlog -lstdc++fs -lglog -lpthread -Wl,-rpath -Wl,/usr/local/lib

# TODO(AD) build lib
.PHONY: all
all: $(OBJ_DIR) $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: install
install:
	../bin/install.sh

.PHONY: installtest
installtest:
	../bin/installtest.sh

.PHONY: clean
clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $@
