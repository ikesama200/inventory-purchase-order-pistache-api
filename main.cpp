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

// PostgreSQLの接続情報の読み込み
std::string makeConnStr(const std::unordered_map<std::string, std::string>& config) {
    return "dbname=" + config.at("dbname") +
           " user=" + config.at("user") +
           " password=" + config.at("password") +
           " host=" + config.at("host") +
           " port=" + config.at("port");
}

class ApiHandler {
public:
    explicit ApiHandler(const std::unordered_map<std::string, std::string>& config)
        : conn(makeConnStr(config))  // ← DB接続文字列を渡して接続確立
        {}
    // 共通関数: SQLファイルを指定してselctを実行
    Rest::Route::Result handleSelect(const std::string& sqlFile,
                                    const Rest::Request&,
                                    Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery(sqlFile);
            pqxx::work txn(conn);
            pqxx::result r = txn.exec(query);

            json result = json::array();
            for (auto row : r) {
                json obj;
                for (auto field : row) {
                    obj[field.name()] = field.c_str() ? field.c_str() : "";
                }
                result.push_back(obj);
            }
            response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
            return Rest::Route::Result::Ok;  // 型で返す
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
            return Rest::Route::Result::Ok;  // 型で返す
        }
    }

    // カテゴリマスタ全件取得
    void getCategory(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select_category.sql");
            pqxx::work txn(conn);
            pqxx::result r = txn.exec(query);
            // txn.commit(); // select処理なのでコメントアウト

            json result = json::array();
            for (auto row : r) {
                json obj;
                for (auto field : row) {
                    obj[field.name()] = field.c_str() ? field.c_str() : "";
                }
                result.push_back(obj);
            }
            response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
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

            pqxx::work txn(conn);
            //txn.exec_params(query, body);  // ⚠️ JSONから値を抜き出す処理を追加予定
            txn.exec_params(query, name, age, id);
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
            std::string sql = loadSqlQuery("sql/select_category.sql");
            // std::string sql = "SELECT * FROM categories_master;";

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
    std::string connStr;
    pqxx::connection conn;
};

int main() {
    auto config = loadDbConfig("config/db.conf");

    Http::Endpoint server(Address(Ipv4::any(), Port(8080)));
    Rest::Router router;
    ApiHandler handler(config);

    // select用のラムダ生成関数
    // auto makeSelectRoute = [&](const std::string& sqlFile) {
    //     return [&, sqlFile](const Rest::Request& req, Http::ResponseWriter res) {
    //         handler.handleSelect(sqlFile, req, std::move(res));
    //     };
    // };
    // 汎用的なラムダ生成関数
    auto makeSelectRoute = [&](const std::string& sqlFile)
        -> std::function<Rest::Route::Result(const Rest::Request&, Http::ResponseWriter)> {
        return [&, sqlFile](const Rest::Request& req, Http::ResponseWriter res)
            -> Rest::Route::Result {
            return handler.handleSelect(sqlFile, req, std::move(res));
        };
    };


    // テーブル情報取得API
    // Rest::Routes::Get(router, "/get-category", Rest::Routes::bind(&ApiHandler::getCategory, &handler));
    // カテゴリマスタ取得
    // Rest::Routes::Get(router, "/get-category", makeSelectRoute("sql/select_category.sql"));
    // カテゴリマスタ取得
    Rest::Routes::Get(router, "/get-category", makeSelectRoute("sql/select_category.sql"));
    // 商品マスタ取得
    Rest::Routes::Get(router, "/get-product",  makeSelectRoute("sql/select_products.sql"));
    // Rest::Routes::Get(router, "/get-category",
    //     Rest::Routes::bind([&](const Rest::Request& req, Http::ResponseWriter res) {
    //         handler.handleSelect("sql/select_category.sql", req, std::move(res));
    //     })
    // );
        
    // 商品マスタ取得
    // Rest::Routes::Get(router, "/get-product", makeSelectRoute("sql/select_products.sql"));
    // Rest::Routes::Get(router, "/get-product",
    //     Rest::Routes::bind([&](const Rest::Request& req, Http::ResponseWriter res) {
    //         handler.handleSelect("sql/select_product.sql", req, std::move(res));
    //     })
    // );

    // テーブル書き込みAPI

    // samapleAPI
    Rest::Routes::Get(router, "/hello", Rest::Routes::bind(&ApiHandler::handleHello, &handler));
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
