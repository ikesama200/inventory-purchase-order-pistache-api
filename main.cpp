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

// DB設定を読み込む関数
std::unordered_map<std::string, std::string> loadDbConfig(const std::string& filepath) {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    return config;
}

// SQLファイル読み込み
std::string loadSqlQuery(const std::string& filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

class ApiHandler {
public:
    explicit ApiHandler(const std::unordered_map<std::string, std::string>& config)
        : connStr("dbname=" + config.at("dbname") +
                  " user=" + config.at("user") +
                  " password=" + config.at("password") +
                  " host=" + config.at("host") +
                  " port=" + config.at("port")) {}

/*
    void init(size_t threads = 2) {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(threads));  // flags 削除
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

    void shutdown() { httpEndpoint->shutdown(); }
*/
    // SELECT
    void handleSelect(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select.sql");
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            pqxx::result r = txn.exec(query);
            txn.commit();

            std::stringstream ss;
            ss << "[";
            for (auto row = r.begin(); row != r.end(); ++row) {
                if (row != r.begin()) ss << ",";
                ss << "{";
                for (auto field = row.begin(); field != row.end(); ++field) {
                    if (field != row.begin()) ss << ",";
                    ss << "\"" << field.name() << "\":\"" << field.c_str() << "\"";
                }
                ss << "}";
            }
            ss << "]";

            response.send(Http::Code::Ok, ss.str(), MIME(Application, Json));
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }

    // INSERT
    void handleInsert(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            auto body = request.body();
            // 例: {"name":"Taro","age":30}
            std::string query = loadSqlQuery("sql/insert.sql");

            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            txn.exec_params(query, body);  // ⚠️ 本来はJSONパースして個別の値をセットする
            txn.commit();

            response.send(Http::Code::Ok, "Insert successful");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }

    // UPDATE
    void handleUpdate(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/update.sql");

            auto body = json::parse(request.body());
            int id = body["id"];
            std::string name = body["name"];
            int age = body["age"];

            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            //txn.exec_params(query, body);  // ⚠️ JSONから値を抜き出す処理を追加予定
            txn.exec_params(sql, name, age, id);
            txn.commit();

            response.send(Http::Code::Ok, "Update successful");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }

    // DELETE
    void handleDelete(const Rest::Request& request, Http::ResponseWriter response) {
        try {
            auto body = request.body();
            std::string query = loadSqlQuery("sql/delete.sql");

            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            txn.exec_params(query, body);  // ⚠️ JSONから値を抜き出す処理を追加予定
            txn.commit();

            response.send(Http::Code::Ok, "Delete successful");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
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

private:
    void setupRoutes() {
        using namespace Rest;
        Routes::Get(router, "/hello", Routes::bind(&ApiHandler::handleHello, this));
        Routes::Get(router, "/data", Routes::bind(&ApiHandler::handleQuery, this));
    }

    std::string connStr;
};

int main() {
    auto config = loadDbConfig("config/db.conf");

    Http::Endpoint server(Address(Ipv4::any(), Port(8080)));
    Rest::Router router;
    ApiHandler handler(config);

    Rest::Routes::Get(router, "/hello", Routes::bind(&ApiHandler::handleHello, &handler));
    //Rest::Routes::Get(router, "/data", Routes::bind(&ApiHandler::handleQuery, this));
    Rest::Routes::Get(router, "/data", Rest::Routes::bind(&ApiHandler::handleSelect, &handler));
    Rest::Routes::Post(router, "/insert", Rest::Routes::bind(&ApiHandler::handleInsert, &handler));
    Rest::Routes::Put(router, "/update", Rest::Routes::bind(&ApiHandler::handleUpdate, &handler));
    Rest::Routes::Delete(router, "/delete", Rest::Routes::bind(&ApiHandler::handleDelete, &handler));

    server.init(Http::Endpoint::options().threads(1));
    server.setHandler(router.handler());
    server.serve();

    return 0;
}
