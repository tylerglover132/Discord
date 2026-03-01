# Default compiler flags (Fixed case to match usage)
CXXFLAGS = -std=c++17 -Wall

# Detect the Operating System
UNAME_S := $(shell uname -s)

# macOS Configuration (Removed tabs on variable assignments)
ifeq ($(UNAME_S),Darwin)
    CXX = clang++
    OPENSSL_DIR := $(shell brew --prefix openssl)
    INCLUDES = -I$(OPENSSL_DIR)/include
    LDFLAGS = -L$(OPENSSL_DIR)/lib -lssl -lcrypto
else
# Linux / Raspberry Pi Configuration
    CXX = g++
    INCLUDES = 
    LDFLAGS = -lssl -lcrypto
endif

# The actual build command (Fixed missing closing parentheses)
discord_bot: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp $(INCLUDES) -o discord_bot $(LDFLAGS)

clean:
	rm -f discord_bot