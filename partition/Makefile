INC_DIR := include
SRC_DIR := src
OBJ_DIR := build

SRC_FILES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS += -std=c++17  -Wall -Werror -MMD
INC_FLAGS := -I$(INC_DIR) -I../log/$(INC_DIR)
LDFLAGS := -L../log -lwombatlog -lstdc++fs -lglog -lpthread -Wl,-rpath -Wl,/usr/local/lib

# TODO(AD) build lib
.PHONY: all
all: $(OBJ_DIR) $(OBJ_FILES)
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(OBJ_FILES) src/main.cpp $(LDFLAGS) -o log

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: install
install:
	../bin/install.sh
	../bin/installtest.sh

TEST_DIR := tests

TEST_INC_FLAGS := -Ilib/googletest/googletest/include
TEST_LDFLAGS := -Llib/googletest/$(OBJ_DIR)/lib -lgtest -lgtest_main -lpthread


# =============================================================================
# Component tests.
# Ensure the test harness is running.
# =============================================================================

COMPONENTTEST_EXE := log_componenttest

COMPONENTTEST_OBJ_DIR = $(OBJ_DIR)/component

COMPONENTTEST_DIR = $(TEST_DIR)/component
COMPONENTTEST_SRC_FILES := $(wildcard $(COMPONENTTEST_DIR)/*.cpp)
COMPONENTTEST_OBJ_FILES := $(patsubst $(COMPONENTTEST_DIR)/%.cpp,$(COMPONENTTEST_OBJ_DIR)/%.o,$(COMPONENTTEST_SRC_FILES))

.PHONY: componenttest
componenttest: $(COMPONENTTEST_OBJ_DIR) $(OBJ_FILES) $(COMPONENTTEST_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(COMPONENTTEST_OBJ_FILES) $(LDFLAGS) $(TEST_LDFLAGS) -o $(COMPONENTTEST_EXE)
	GLOG_logtostderr=1 ./$(COMPONENTTEST_EXE)

$(OBJ_DIR)/component/%.o: $(TEST_DIR)/component/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(TEST_INC_FLAGS) -c -o $@ $<

-include $(COMPONENTTEST_OBJ_FILES:.o=.d)


.PHONY: clean
clean:
	rm -rf log log.d lib $(OBJ_DIR) $(COMPONENTTEST_EXE)

$(OBJ_DIR) $(COMPONENTTEST_OBJ_DIR):
	mkdir -p $@
