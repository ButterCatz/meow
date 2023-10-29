#include <meow.hpp>
#include <deque>

const meow::net::ServerInfo SERVER = {.host = "127.0.0.1", .port = "8080"};

const meow::net::Token TOKEN = {.value = 123};

int main() {
    meow::net::Client client(SERVER, TOKEN);
    client.connect();
    client.login();
    client.say("Hello World!");
    client.getProfile();
    while (client.isConnected()) {
        if (!client.msgInQueue.empty()) {
            std::cout << "[Client] Message received" << std::endl;
            auto msg = client.msgInQueue.pop_front().message;

            switch (msg.header.id) {
            case meow::net::MessageId::Accept: {
                std::cout << "[Client] Login accepted" << std::endl;
                break;
            }
            case meow::net::MessageId::Message: {
                std::string message = "";
                for (size_t i = 0; i < msg.body.size(); i++) {
                    message += msg.body[i];
                }
                std::cout << "[Client] Message: " << message << std::endl;
                break;
            }
            case meow::net::MessageId::Profile: {
                std::string profile = "";
                for (size_t i = 0; i < msg.body.size(); i++) {
                    profile += msg.body[i];
                }
                std::cout << "[Client] Profile: " << profile << std::endl;
                break;
            }
            }
        }
    }
    std::cout << "[Client] Disconnected" << std::endl;
    client.disconnect();
}