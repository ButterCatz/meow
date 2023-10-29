// Message
// Defines the message structure used by the server and client.
// ---------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <vector>
#include <memory>

namespace meow::net {

    enum class MessageId : uint32_t { Accept, Login, Logout, Message, Profile };

    template <typename T>
    struct MessageHeader {
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct Message {
    public:
        MessageHeader<T> header{};
        std::vector<uint8_t> body;

        size_t size() const { return body.size(); }

        friend std::ostream &operator<<(std::ostream &os, const Message<T> &msg) {
            os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
            return os;
        }

        template <typename DataType>
        friend Message<T> &operator<<(Message<T> &msg, const DataType &data) {
            static_assert(std::is_standard_layout<DataType>::value,
                          "Data is too complex to be pushed into vector");
            size_t i = msg.body.size();
            msg.body.resize(msg.body.size() + sizeof(DataType));
            std::copy(reinterpret_cast<const uint8_t *>(&data),
                      reinterpret_cast<const uint8_t *>(&data) + sizeof(DataType), msg.body.begin() + i);
            msg.header.size = msg.size();
            return msg;
        }
    };

    class Connection;

    template <typename T>
    struct OwnedMessage {
        std::shared_ptr<Connection> remote = nullptr;
        Message<T> message;

        friend std::ostream &operator<<(std::ostream &os, const OwnedMessage<T> &msg) {
            os << msg.message;
            return os;
        }
    };

    template <typename T, typename Datatype>
    void read_data(Message<T> &msg, size_t offset, Datatype &data) {
        static_assert(std::is_standard_layout<Datatype>::value, "Data is too complex to be read from vector");
        if (msg.body.size() < offset + sizeof(Datatype)) {
            std::cout << "[ERROR] Message body is too small to read data" << std::endl;
            return;
        }
        std::copy(msg.body.data() + offset, msg.body.data() + offset + sizeof(Datatype),
                  reinterpret_cast<uint8_t *>(&data));
    }

    template <typename T, typename DataType, typename... Args>
    void read_data(Message<T> &msg, size_t offset, DataType &data, Args &...args) {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be read from vector");
        if (msg.body.size() < offset + sizeof(DataType)) {
            std::cout << "[ERROR] Message body is too small to read data" << std::endl;
            return;
        }
        std::copy(msg.body.data() + offset, msg.body.data() + offset + sizeof(DataType),
                  reinterpret_cast<uint8_t *>(&data));
        read_data(msg, offset + sizeof(DataType), args...);
    }

} // namespace meow::net