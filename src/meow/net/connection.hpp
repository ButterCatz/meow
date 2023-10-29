// Connection
// Handles the connection between the client and the server.
// ---------------------------------------------------------------------------
#pragma once

#include <asio.hpp>

#include <meow/net/tsqueue.hpp>
#include <meow/net/message.hpp>

#include <thread>
#include <string>
#include <iostream>
#include <vector>

namespace meow::net {

    struct ServerInfo {
        std::string host;
        std::string port;
    };

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        enum class Owner { Server, Client };

    private:
        asio::error_code ec;
        Owner owner;
        asio::io_context &io_context;
        asio::ip::tcp::socket socket;
        TSQueue<Message<MessageId>> msgOutQueue;
        TSQueue<OwnedMessage<MessageId>> &msgInQueue;
        Message<MessageId> msgBuffer;

    public:
        Connection(Owner owner, asio::io_context &io_context, asio::ip::tcp::socket socket,
                   meow::net::TSQueue<OwnedMessage<MessageId>> &msgInQueue)
            : owner(owner), io_context(io_context), socket(std::move(socket)), msgInQueue(msgInQueue) {}
        ~Connection() {}

        bool isConnected() const { return socket.is_open(); }

        void connect_to_client() {
            if (owner == Owner::Server) {
                if (isConnected()) {
                    read_header();
                } else {
                    std::cout << "[ERROR] Connect to client error: not connected" << std::endl;
                }
            }
        }

        void connect_to_server(const asio::ip::tcp::resolver::results_type &endpoints) {
            if (owner == Owner::Client) {
                asio::async_connect(
                    socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
                        if (!ec) {
                            read_header();
                        } else {
                            std::cout << "[ERROR] Connect error: " << ec.message() << std::endl;
                        }
                    });
            }
        }

        void disconnect() {
            if (isConnected()) {
                asio::post(io_context, [this]() { socket.close(); });
            }
        }

        template <typename T>
        void send(const Message<T> &message) {
            asio::post(io_context, [this, message]() {
                bool writing_message = !msgOutQueue.empty();
                msgOutQueue.emplace_back(message);
                if (!writing_message) {
                    write_header();
                }
            });
        }

        void write_header() {
            asio::async_write(
                socket, asio::buffer(&msgOutQueue.front().header, sizeof(MessageHeader<MessageId>)),
                [this](std::error_code ec, std::size_t length) {
                    if (!ec) {
                        if (msgOutQueue.front().body.size() > 0) {
                            write_body();
                        } else {
                            msgOutQueue.pop_front();
                            if (!msgOutQueue.empty()) {
                                write_header();
                            }
                        }
                    } else {
                        std::cout << "[ERROR] Write header error: " << ec.message() << std::endl;
                        socket.close();
                    }
                });
        }

        void write_body() {
            asio::async_write(socket,
                              asio::buffer(msgOutQueue.front().body.data(), msgOutQueue.front().body.size()),
                              [this](std::error_code ec, std::size_t length) {
                                  if (!ec) {
                                      msgOutQueue.pop_front();
                                      if (!msgOutQueue.empty()) {
                                          write_header();
                                      }
                                  } else {
                                      std::cout << "[ERROR] Write body error: " << ec.message() << std::endl;
                                      socket.close();
                                  }
                              });
        }

        void read_header() {
            asio::async_read(socket, asio::buffer(&msgBuffer.header, sizeof(MessageHeader<MessageId>)),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     if (msgBuffer.header.size > 0) {
                                         msgBuffer.body.resize(msgBuffer.header.size);
                                         read_body();
                                     } else {
                                         add_to_message_in_queue();
                                     }
                                 } else {
                                     std::cout << "[ERROR] Read header error: " << ec.message() << std::endl;
                                     socket.close();
                                 }
                             });
        }

        void read_body() {
            asio::async_read(socket, asio::buffer(msgBuffer.body.data(), msgBuffer.body.size()),
                             [this](std::error_code ec, std::size_t length) {
                                 if (!ec) {
                                     add_to_message_in_queue();
                                 } else {
                                     std::cout << "[ERROR] Read body error: " << ec.message() << std::endl;
                                     socket.close();
                                 }
                             });
        }

        void add_to_message_in_queue() {
            if (owner == Owner::Server) {
                msgInQueue.emplace_back({this->shared_from_this(), msgBuffer});
            } else {
                msgInQueue.emplace_back({nullptr, msgBuffer});
            }
            read_header();
        }
    };

} // namespace meow::net