CXX = g++
# Using C++17 is recommended for modern Boost
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 

# We need OpenSSL for TLS and pthread for asynchronous tasks later
LDFLAGS = -lssl -lcrypto -lpthread

# Depending on your OS and Boost version, you might also need to link boost_system
# LDFLAGS += -lboost_system

TARGET = my_bot
SRC = main.cpp bot.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)