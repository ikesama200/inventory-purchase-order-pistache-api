#include <pistache/endpoint.h>
#include <pistache/router.h>
#include <pistache/http.h>
#include <pistache/net.h>

#include <pqxx/pqxx>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>

using namespace Pistache;
using json = nlohmann::json;

// db.conf を読み込む関数
std::unordered_map<std::string, std::string> loadDbConfig(const std::string &filename) {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    return config;
}

// 外部 SQL ファイルを読み込む関数
std::string loadSqlFile(const std::string &filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

class ApiHandler {
public:
    //explicit HelloHandler(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}
    explicit ApiHandler(Address addr) : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {}

    void init(size_t threads = 2) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(threads));  // flags 削除
                        //.flags(Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() { httpEndpoint->shutdown(); }

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/hello", Routes::bind(&ApiHandler::handleHello, this));
        Routes::Get(router, "/data", Routes::bind(&ApiHandler::handleQuery, this));
    }

    void handleHello(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "Hello, Pistache!");
    }

    void handleQuery(const Rest::Request&, Http::ResponseWriter response) {
        try {
            // DB設定を読み込み
            auto config = loadDbConfig("config/db.conf");
            std::string connStr =
                "host=" + config["host"] +
                " port=" + config["port"] +
                " dbname=" + config["dbname"] +
                " user=" + config["user"] +
                " password=" + config["password"];

            // 外部 SQL 読み込み
            // std::string sql = loadSqlFile("query.sql");
            std::string sql = "SELECT * FROM categories_master;";

            pqxx::connection conn(connStr);
            pqxx::work txn(conn);

            pqxx::result r = txn.exec(sql);

            json result = json::array();
            for (auto row : r) {
                json obj;
                for (auto field : row) {
                    obj[field.name()] = field.c_str() ? field.c_str() : "";
                }
                result.push_back(obj);
            }

            response.send(Http::Code::Ok, result.dump());

        } catch (const std::exception &e) {
            response.send(Http::Code::Internal_Server_Error,
                          std::string("Error: ") + e.what());
        }
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

int main() {
    Address addr(Ipv4::any(), Port(8080));
    ApiHandler server(addr);

    server.init(2);   // スレッド数1で初期化
    server.start();

    return 0;
}
