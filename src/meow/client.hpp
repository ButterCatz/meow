// Defines the client for calling the Meow API.
// When constructing the client, pass in the host and port of the server.
// ---------------------------------------------------------------------------
#pragma once

#include <meow/net.hpp>
#include <string>

namespace meow::net {

    struct Token {
        uint64_t value;
    };

    struct Profile {
        std::string username;
        std::string email;
    };

    class Client {
    private:
        ServerInfo server;
        Token token;
        Profile profile;

        asio::io_context context;
        std::thread thread_context;

        std::unique_ptr<Connection> connection;

    public:
        TSQueue<OwnedMessage<MessageId>> msgInQueue;
        Client(ServerInfo server, Token token);
        ~Client();

        bool connect();
        void disconnect();
        bool isConnected() const;
        void login();
        void say(std::string message);
        void getProfile();
    };

} // namespace meow::net
