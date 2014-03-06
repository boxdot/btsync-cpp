# Bit Torrent Sync credentials for running tests
# Before running test, make sure that BitTorrent is started
BTSYNC_LOGIN = api
BTSYNC_PASSWORD = secret

JSONCPP = deps/json-cpp/tags/jsoncpp/0.5.0
CATCH = deps/catch

INCLUDE = -I include -I $(JSONCPP)/include -I $(CATCH)/include

LIBS = -L build -lcurl -ljson
TEST_LIBS = -L build -L lib -lbtsync -ljson -lboost_system-mt -lboost_filesystem

JSONCPP_SRC = json_reader.cpp json_value.cpp json_writer.cpp
JSONCPP_OBJ = $(patsubst %.cpp,build/%.o,$(JSONCPP_SRC))

vpath %.cpp src:test:$(JSONCPP)/src/lib_json

all: lib/libbtsync.a
tests: lib/libbtsync.a bin/client_test

#
# lib json-cpp
#

build/libjson.a: $(JSONCPP_OBJ)
	$(CXX) -shared $^ -o $@

#
# lib btsync
#

lib/libbtsync.a: build/client.o lib build/libjson.a
	$(CXX) -shared $< -o $@ $(LIBS)

#
# tests
#

bin/client_test: build/client_test.o lib/libbtsync.a bin
	$(CXX) $< -o $@ $(TEST_LIBS)

test: bin/client_test
	@bin/client_test

#
#
# 

build lib bin:
	@mkdir -p $@

$(JSONCPP_OBJ) build/client.o client_test.o: | build

build/client_test.o:: client_test.cpp
	$(CXX) $(INCLUDE) -c $< -o $@ -D BTSYNC_LOGIN=\"$(BTSYNC_LOGIN)\" -D BTSYNC_PASSWORD=\"$(BTSYNC_PASSWORD)\"

build/%.o: %.cpp
	$(CXX) $(INCLUDE) -c $< -o $@

.PHONY:
clean:
	@rm -r build lib bin
