INC_DIR := include
SRC_DIR := src
OBJ_DIR := build
LIB_DIR := lib

SRC_FILES = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS += -std=c++17  -Wall -Werror -MMD
INC_FLAGS := -I$(INC_DIR) -I../log/$(INC_DIR) -I ../record/$(INC_DIR)
LDFLAGS := -L../record -lwombatrecord -L../log -lwombatlog -lstdc++fs -lglog -lpthread -Wl,-rpath -Wl,/usr/local/lib

.PHONY: all
all: $(OBJ_DIR) $(OBJ_FILES)
	ar rvs libwombatconsumeserver.a $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) -c -o $@ $<

-include $(OBJ_FILES:.o=.d)

.PHONY: install
install:
	../bin/install.sh

TEST_DIR := tests

TEST_INC_FLAGS := -I../lib/googletest/googletest/include
TEST_LDFLAGS := -L../lib/googletest/$(OBJ_DIR)/lib -lgtest -lgtest_main -lpthread

.PHONY: unittest
unittest:
	echo "No unittests"

# =============================================================================
# Integration tests.
# =============================================================================

INTEGRATIONTEST_EXE := server_integrationtest

INTEGRATIONTEST_OBJ_DIR = $(OBJ_DIR)/integration

INTEGRATIONTEST_DIR = $(TEST_DIR)/integration
INTEGRATIONTEST_SRC_FILES := $(wildcard $(INTEGRATIONTEST_DIR)/*.cpp)
INTEGRATIONTEST_OBJ_FILES := $(patsubst $(INTEGRATIONTEST_DIR)/%.cpp,$(INTEGRATIONTEST_OBJ_DIR)/%.o,$(INTEGRATIONTEST_SRC_FILES))

.PHONY: integrationtest
integrationtest: $(INTEGRATIONTEST_OBJ_DIR) $(OBJ_FILES) $(INTEGRATIONTEST_OBJ_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJ_FILES) $(INTEGRATIONTEST_OBJ_FILES) $(LDFLAGS) $(TEST_LDFLAGS) -o $(INTEGRATIONTEST_EXE)
	GLOG_logtostderr=1 ./$(INTEGRATIONTEST_EXE)

$(OBJ_DIR)/integration/%.o: $(TEST_DIR)/integration/%.cpp
	 $(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_FLAGS) $(TEST_INC_FLAGS) -c -o $@ $<

-include $(INTEGRATIONTEST_OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -rf $(LIB_DIR) $(OBJ_DIR) $(INTEGRATIONTEST_EXE)

$(OBJ_DIR) $(INTEGRATIONTEST_OBJ_DIR):
	mkdir -p $@
