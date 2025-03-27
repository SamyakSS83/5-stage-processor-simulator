CXX = g++ -g
SRC_DIR = src
BIN_DIR = src

all: assembly-forwarding noforward forward assembly-no-forwarding fun
assembly-forwarding: $(SRC_DIR)/assembly-forwarding.cpp
	$(CXX) $< -o $(BIN_DIR)/assembly-forwarding

noforward: $(SRC_DIR)/noforward.cpp
	$(CXX) $< -o $(BIN_DIR)/noforward

forward: $(SRC_DIR)/forward.cpp
	$(CXX) $< -o $(BIN_DIR)/forward

assembly-no-forwarding: $(SRC_DIR)/assembly-no-forwarding.cpp
	$(CXX) $< -o $(BIN_DIR)/assembly-no-forwarding

fun : 
	./fun.sh compiled

clean:
	rm -f $(BIN_DIR)/assembly-forwarding $(BIN_DIR)/noforward $(BIN_DIR)/forward $(BIN_DIR)/assembly-no-forwarding
	./fun.sh cleaned
