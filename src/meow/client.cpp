#include <meow/client.hpp>

namespace meow::net {
    Client::Client(meow::net::ServerInfo server, Token token) {
        this->server = server;
        this->token = token;
    }

    Client::~Client() { disconnect(); }

    bool Client::connect() {
        try {
            asio::ip::tcp::resolver resolver(context);
            auto endpoints = resolver.resolve(server.host, server.port);
            connection = std::make_unique<Connection>(Connection::Owner::Client, context,
                                                      asio::ip::tcp::socket(context), msgInQueue);
            connection->connect_to_server(endpoints);
            thread_context = std::thread([this]() { context.run(); });
        } catch (std::exception &e) {
            std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    void Client::disconnect() {
        if (isConnected()) {
            connection->disconnect();
        }
        context.stop();
        if (thread_context.joinable()) {
            thread_context.join();
        }
        connection.release();
    }

    bool Client::isConnected() const {
        if (connection) {
            return connection->isConnected();
        }
        return false;
    }

    void Client::login() {
        std::cout << "[Client] Logging in with token: " << token.value << std::endl;
        meow::net::Message<meow::net::MessageId> message;
        message.header.id = meow::net::MessageId::Login;
        message << token.value;
        connection->send(message);
    }

    void Client::say(std::string message) {
        std::cout << "[Client] Sending message: " << message << std::endl;
        meow::net::Message<meow::net::MessageId> msg;
        msg.header.id = meow::net::MessageId::Message;
        for (size_t i = 0; i < message.size(); i++) {
            msg << message[i];
        }
        connection->send(msg);
    }

    void Client::getProfile() {
        std::cout << "[Client] Requesting profile" << std::endl;
        meow::net::Message<meow::net::MessageId> msg;
        msg.header.id = meow::net::MessageId::Profile;
        msg << token;
        connection->send(msg);
        Profile profile;
    }
} // namespace meow::net