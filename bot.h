#ifndef DISCORD_BOT_H
#define DISCORD_BOT_H

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>

#include <string>
#include <queue>


namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class Bot {
    private:
        std::string bot_token;

        // Core Networking Contexts
        net::io_context ioc;
        ssl::context ctx{ssl::context::tlsv12_client};
        tcp::resolver resolver{ioc};
        websocket::stream<beast::ssl_stream<tcp::socket>> ws{ioc, ctx};

        // Async State Variables
        beast::flat_buffer buffer;
        net::steady_timer heartbeat_timer{ioc};

        // Write Queue Variables
        std::queue<std::string> write_queue;
        bool is_writing = false;

        // Async Callbacks
        void read_loop();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);

        void start_heartbeat(int interval_ms);
        void on_heartbeat(beast::error_code ec, int interval_ms);

        // Queue Methods
        void do_write();
        void on_write(beast::error_code ec, std::size_t bytes_transferred);

    public:
        void run(std::string token);
        void send_message(std::string payload);
};

#endif // DISCORD_BOT_H