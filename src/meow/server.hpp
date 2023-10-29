#pragma once

#include <meow/net.hpp>
#include <asio.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>

namespace meow::net {
    class Server {
    protected:
        uint16_t port;
        asio::io_context io_context;
        asio::ip::tcp::acceptor acceptor;

        std::thread thread_context;

        TSQueue<OwnedMessage<MessageId>> msgInQueue;
        std::deque<std::shared_ptr<Connection>> connections;

    public:
        Server(const uint16_t port)
            : port(port), acceptor(asio::ip::tcp::acceptor(
                              io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))) {}
        ~Server() { stop(); }

        bool start() {
            try {
                wait_for_client();
                thread_context = std::thread([this]() { io_context.run(); });
            } catch (std::exception &e) {
                std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
                return false;
            }
            std::cout << "[Info] Server started" << std::endl;
            std::cout << "[Info] Server is running on port " << port << std::endl;
            return true;
        }

        void stop() {
            io_context.stop();
            if (thread_context.joinable())
                thread_context.join();
            std::cout << "[Info] Server stopped" << std::endl;
        }

        void wait_for_client() {
            acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    std::cout << "[Info] New connection: " << socket.remote_endpoint() << std::endl;
                    auto new_connection = std::make_shared<Connection>(Connection::Owner::Server, io_context,
                                                                       std::move(socket), msgInQueue);
                    connections.emplace_back(std::move(new_connection));
                    connections.back()->connect_to_client();
                    
                } else {
                    std::cout << "[ERROR] New connection error: " << ec.message() << std::endl;
                }
                wait_for_client();
            });
        }

        void sendMessage(std::shared_ptr<Connection> client, const Message<MessageId> &message) {
            if (client && client->isConnected()) {
                client->send(message);
            } else {
                client.reset();
                connections.erase(std::remove(connections.begin(), connections.end(), client),
                                  connections.end());
            }
        }

        void update(size_t maxMessages = -1, bool wait = false) {
            if (wait) {
                msgInQueue.wait();
            }
            size_t messageCount = 0;
            while (messageCount < maxMessages && !msgInQueue.empty()) {
                auto msg = msgInQueue.pop_front();
                onMessage(msg.remote, msg.message);
                messageCount++;
            }
        }

        virtual void onMessage(std::shared_ptr<Connection> client, Message<MessageId> &msg) {
            std::cout << "[DEBUG] This should be overrided" << std::endl;
        }
    };
} // namespace meow::net