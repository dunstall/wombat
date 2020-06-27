INC_DIR := include
SRC_DIR := src
OBJ_DIR := build
LIB_DIR := lib

SRC_FILES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS += -std=c++17  -Wall -Werror -MMD
INC_FLAGS := -I$(INC_DIR)
LDFLAGS := -lstdc++fs -lglog -lpthread -Wl,-rpath -Wl,/usr/local/lib

.PHONY: all
all: $(OBJ_DIR) $(OBJ_FILES)
	ar rvs libwombatlog.a $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: install
install:
	../bin/install.sh

TEST_DIR := tests

TEST_INC_FLAGS := -I../$(LIB_DIR)/googletest/googletest/include
TEST_LDFLAGS := -L../$(LIB_DIR)/googletest/$(OBJ_DIR)/lib -lgtest -lgtest_main -lpthread


# =============================================================================
# Unit tests.
# =============================================================================

UNITTEST_EXE := log_unittest

UNITTEST_OBJ_DIR = $(OBJ_DIR)/unit

UNITTEST_DIR = $(TEST_DIR)/unit
UNITTEST_SRC_FILES := $(wildcard $(UNITTEST_DIR)/*.cpp)
UNITTEST_OBJ_FILES := $(patsubst $(UNITTEST_DIR)/%.cpp,$(UNITTEST_OBJ_DIR)/%.o,$(UNITTEST_SRC_FILES))

.PHONY: unittest
unittest: $(UNITTEST_OBJ_DIR) $(OBJ_FILES) $(UNITTEST_OBJ_FILES)
	echo $(UNITTEST_OBJ_FILES)
	echo $(UNITTEST_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(UNITTEST_OBJ_FILES) $(LDFLAGS) $(TEST_LDFLAGS) -o $(UNITTEST_EXE)
	GLOG_logtostderr=1 ./$(UNITTEST_EXE)

$(OBJ_DIR)/unit/%.o: $(TEST_DIR)/unit/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(TEST_INC_FLAGS) -c -o $@ $<

-include $(UNITTEST_OBJ_FILES:.o=.d)


# =============================================================================
# Integration tests.
# =============================================================================

INTEGRATIONTEST_EXE := log_integrationtest

INTEGRATIONTEST_OBJ_DIR = $(OBJ_DIR)/integration

INTEGRATIONTEST_DIR = $(TEST_DIR)/integration
INTEGRATIONTEST_SRC_FILES := $(wildcard $(INTEGRATIONTEST_DIR)/*.cpp)
INTEGRATIONTEST_OBJ_FILES := $(patsubst $(INTEGRATIONTEST_DIR)/%.cpp,$(INTEGRATIONTEST_OBJ_DIR)/%.o,$(INTEGRATIONTEST_SRC_FILES))

.PHONY: integrationtest
integrationtest: $(INTEGRATIONTEST_OBJ_DIR) $(OBJ_FILES) $(INTEGRATIONTEST_OBJ_FILES)
	echo $(INTEGRATIONTEST_OBJ_FILES)
	echo $(INTEGRATIONTEST_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(INTEGRATIONTEST_OBJ_FILES) $(LDFLAGS) $(TEST_LDFLAGS) -o $(INTEGRATIONTEST_EXE)
	GLOG_logtostderr=1 ./$(INTEGRATIONTEST_EXE)

$(OBJ_DIR)/integration/%.o: $(TEST_DIR)/integration/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(TEST_INC_FLAGS) -c -o $@ $<

-include $(INTEGRATIONTEST_OBJ_FILES:.o=.d)


.PHONY: clean
clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR) $(UNITTEST_EXE) $(INTEGRATIONTEST_EXE) $(COMPONENTTEST_EXE)

$(OBJ_DIR) $(UNITTEST_OBJ_DIR) $(INTEGRATIONTEST_OBJ_DIR) $(COMPONENTTEST_OBJ_DIR):
	mkdir -p $@
