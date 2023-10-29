#include <meow.hpp>
#include <unordered_map>
#include <string>

namespace catnest {
    using namespace meow::net;
    struct Hasher {
        size_t operator()(const Token &token) const { return std::hash<uint64_t>()(token.value); }
    };

    struct Equal {
        bool operator()(const Token &lhs, const Token &rhs) const { return lhs.value == rhs.value; }
    };

    class Catnest : public Server {

        std::unordered_map<Token, std::string, Hasher, Equal> profiles;

    public:
        Catnest(const uint16_t port) : Server(port) {}
        ~Catnest() {}

        void test_setup();

        void onMessage(std::shared_ptr<Connection> client, Message<MessageId> &msg) override {
            switch (msg.header.id) {
            case MessageId::Login: {
                std::cout << "[Info] Login request" << std::endl;
                Token token;
                read_data(msg, 0, token);
                std::cout << "[Info] Token: " << token.value << std::endl;
                Message<MessageId> response;
                response.header.id = MessageId::Accept;
                sendMessage(client, response);
                break;
            }
            case MessageId::Message: {
                std::cout << "[Info] Message request" << std::endl;
                std::string message = "";
                for (size_t i = 0; i < msg.body.size(); i++) {
                    message += msg.body[i];
                }
                std::cout << "[Info] Message: " << message << std::endl;
                Message<MessageId> response;
                response.header.id = MessageId::Message;
                response << "You said: ";
                for (size_t i = 0; i < msg.body.size(); i++) {
                    response << msg.body[i];
                }
                response << "!";
                sendMessage(client, response);
                break;
            }
            case MessageId::Profile: {
                std::cout << "[Info] Profile request" << std::endl;
                Token token;
                read_data(msg, 0, token);
                std::cout << "[Info] Token: " << token.value << std::endl;
                auto profile = profiles[token];
                std::cout << "[Info] Profile: " << profile << std::endl;
                Message<MessageId> response;
                response.header.id = MessageId::Profile;
                for (size_t i = 0; i < profile.size(); i++) {
                    response << profile[i];
                }
                sendMessage(client, response);
                break;
            }
            }
        }
    };
} // namespace catnest

int main(int argc, char *argv[]) {
    catnest::Catnest server(8080);
    server.test_setup();
    server.start();
    while (true) {
        server.update(-1, true);
    }
    return 0;
}

namespace catnest {
    void Catnest::test_setup() {
        std::cout << "[DEBUG] Test setup" << std::endl;
        Token token{.value = 123};
        std::string profile = "{ \"username\": \"cat\", \"email\": \"example@catnest.org\" }";
        profiles.insert({token, profile});
    }
} // namespace catnest