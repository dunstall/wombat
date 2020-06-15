INC_DIR := include
SRC_DIR := src
OBJ_DIR := build

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS += -std=c++17  -Wall -Werror -MMD
INC_FLAGS := -I$(INC_DIR)
LDFLAGS := -lstdc++fs

# TODO(AD) build lib
.PHONY: all
all: $(OBJ_DIR) $(OBJ_FILES)
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(LDFLAGS) -o log

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: install
install:
	./bin/installtest.sh

TEST_DIR := tests

TEST_INC_FLAGS := -Ilib/googletest/googletest/include
TEST_LDFLAGS := -Llib/googletest/$(OBJ_DIR)/lib -lgtest -lgtest_main -lpthread

TEST_EXE := log_test

TEST_SRC_FILES := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ_FILES := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRC_FILES))

.PHONY: test
test: $(OBJ_FILES) $(TEST_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(TEST_OBJ_FILES) $(LDFLAGS) $(TEST_LDFLAGS) -o $(TEST_EXE)
	./$(TEST_EXE)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(TEST_INC_FLAGS) -c -o $@ $<

-include $(TEST_OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -rf lib $(OBJ_DIR) $(TEST_EXE)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
