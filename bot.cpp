#include "bot.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void Bot::run(std::string token) {
    bot_token = token;
    std::string host = "gateway.discord.gg";
    std::string port = "443";

    try {
        // Synchronous Setup
        ctx.set_default_verify_paths();
        auto const results = resolver.resolve(host, port);
        net::connect(ws.next_layer().next_layer(), results.begin(), results.end());

        if(!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), host.c_str())){
            throw beast::system_error(
                beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()),
                "Failed to set SNI Hostname"
            );
        } // <-- FIXED: This bracket was missing!

        ws.next_layer().handshake(ssl::stream_base::client);
        ws.handshake(host, "/?v=10&encoding=json");
        std::cout << "Connected securely to Discord Gateway!\n";

        ws.read(buffer);
        std::string hello_str = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());

        json hello_json = json::parse(hello_str);
        int heartbeat_interval = hello_json["d"]["heartbeat_interval"];
        std::cout << "Received Hello. Heartbeat interval: " << heartbeat_interval << "ms\n";

        json identify_payload = {
            {"op", 2},
            {"d", {
                {"token", bot_token},
                {"intents", 513}, 
                {"properties", {
                    {"os", "linux"},
                    {"browser", "my_cpp_bot"},
                    {"device", "my_cpp_bot"}
                }}
            }}
        };
        ws.write(net::buffer(identify_payload.dump()));
        std::cout << "Sent Identify Payload.\n\n";

        // Start Asynchronous loops
        start_heartbeat(heartbeat_interval);
        read_loop();

        ioc.run();
        
    } catch(std::exception const& e) {
        std::cerr << "Fatal Error in Bot::run: " << e.what() << std::endl;
    }
}

// --- ASYNCHRONOUS HEARTBEAT LOGIC ---

void Bot::start_heartbeat(int interval_ms) {
    heartbeat_timer.expires_after(std::chrono::milliseconds(interval_ms));
    heartbeat_timer.async_wait([this, interval_ms](beast::error_code ec){
        on_heartbeat(ec, interval_ms);
    });
}

void Bot::on_heartbeat(beast::error_code ec, int interval_ms) {
    if (ec) {
        std::cerr << "Heartbeat Timer Error: " << ec.message() << "\n";
        return;
    }

    json heartbeat_payload = {
        {"op", 1},
        {"d", nullptr}
    };

    std::cout << "[Heartbeat] Queuing thump...\n";
    send_message(heartbeat_payload.dump());

    start_heartbeat(interval_ms);
}

// --- WRITE QUEUE LOGIC ---

void Bot::send_message(std::string payload) {
    write_queue.push(payload);

    if (!is_writing) {
        is_writing = true;
        do_write();
    }
}

void Bot::do_write() {
    ws.async_write(
        net::buffer(write_queue.front()),
        [this](beast::error_code ec, std::size_t bytes_transferred) {
            on_write(ec, bytes_transferred);
        }
    );
}

void Bot::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        std::cerr << "Write Error: " << ec.message() << "\n";
        is_writing = false;
        return;
    }

    write_queue.pop();

    if (!write_queue.empty()) {
        do_write();
    } else {
        is_writing = false;
    }
}

void Bot::read_loop() {
    ws.async_read(buffer, [this](beast::error_code ec, std::size_t bytes_transferred) {
        on_read(ec, bytes_transferred);
    });
}

void Bot::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec == websocket::error::closed) {
        std::cout << "\nDisconnected! Close Code: " << ws.reason().code << "\n";
        return;
    }
    if (ec) {
        std::cerr << "Read Error: " << ec.message() << "\n";
        return;
    }

    std::string message = beast::buffers_to_string(buffer.data());
    buffer.consume(buffer.size());

    try {
        json event = json::parse(message);

        std::cout << "Received Event -> Opcode: " << event["op"];
        if (event.contains("t") && !event["t"].is_null()) {
            std::cout << " | Type: " << event["t"];
        }
        std::cout << "\n";
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << "\n";
    }

    read_loop();
}