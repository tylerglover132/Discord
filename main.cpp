#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "bot.h"

// Helper function to load .env variables
void load_env(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << file_path << "\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        auto delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            // setenv is a POSIX function available on Linux to set env vars
            setenv(key.c_str(), value.c_str(), 1); 
        }
    }
}

int main() {
    // 1. Load the .env file
    load_env(".env");

    // 2. Retrieve the token from the system environment
    const char* env_token = std::getenv("DISCORD_BOT_TOKEN");
    if (!env_token) {
        std::cerr << "Fatal Error: DISCORD_BOT_TOKEN not found in environment.\n";
        return 1;
    }
    
    std::string bot_token = env_token;

    // 3. Instantiate our asynchronous bot and run it!
    std::cout << "Starting bot engine...\n";
    Bot my_bot;
    my_bot.run(bot_token);

    return 0;
}