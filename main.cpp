#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>

using namespace Pistache;

class HelloHandler {
public:
    explicit HelloHandler(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t threads = 1) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(threads));  // flags 削除
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/hello", Routes::bind(&HelloHandler::handleHello, this));
    }

    void handleHello(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Hello, Pistache!");
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main() {
    Address addr(Ipv4::any(), Port(8080));
    HelloHandler server(addr);
    server.init(1);   // スレッド数1で初期化
    server.start();
}
