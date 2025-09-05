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
    // SELECT
    void handleSelect(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select.sql");
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

            // CORS 対応
            response.headers()
                .add<Http::Header::AccessControlAllowOrigin>("*") // 全オリジン許可
                .add<Http::Header::AccessControlAllowMethods>("GET, POST, PUT, DELETE, OPTIONS")
                .add<Http::Header::AccessControlAllowHeaders>("Content-Type");

            response.send(Http::Code::Ok, ss.str(), MIME(Application, Json));
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
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

    // 商品マスタ全件取得
    void getProducts(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select_products.sql");
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

    // 在庫一覧取得
    void getInventoryList(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select_inventory_list.sql");
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
            // CORS 対応
            response.headers()
                .add<Http::Header::AccessControlAllowOrigin>("*") // 全オリジン許可
                .add<Http::Header::AccessControlAllowMethods>("GET, POST, PUT, DELETE, OPTIONS")
                .add<Http::Header::AccessControlAllowHeaders>("Content-Type");

            response.send(Http::Code::Ok, result.dump(), MIME(Application, Json));
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }

    // 発注書一覧取得
    void getPurchaseOrders(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select_purchase_orders.sql");
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

    // コードマスタ取得
    void getCodeMaster(const Rest::Request&, Http::ResponseWriter response) {
        try {
            std::string query = loadSqlQuery("sql/select_code.sql");
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
    void setCategory(const Rest::Request& request, Http::ResponseWriter response) {
        try {

            std::string query = loadSqlQuery("sql/insert_category_master.sql");
            auto body = json::parse(request.body());
            auto categoryName = body["category_name"];

            pqxx::work txn(conn);
            txn.exec_params(query, categoryName); 
            txn.commit();

            response.send(Http::Code::Ok, "Insert successful");
        } catch (const std::exception& e) {
            response.send(Http::Code::Internal_Server_Error, e.what());
        }
    }
    void setProducts(const Rest::Request& request, Http::ResponseWriter response) {
        try {

            std::string query = loadSqlQuery("sql/insert_products_master.sql");
            auto body = json::parse(request.body());
            auto productCode = body["product_code"];
            auto productName = body["product_name"];
            auto categoryId = body["category_id"];
            auto updatedUserid = body["updated_userid"];

            pqxx::work txn(conn);
            txn.exec_params(query, productCode, productName, categoryId, updatedUserid); 
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

    // テーブル情報取得API
    Rest::Routes::Get(router, "/get-category", Rest::Routes::bind(&ApiHandler::getCategory, &handler));
    Rest::Routes::Get(router, "/get-product", Rest::Routes::bind(&ApiHandler::getProducts, &handler));
    Rest::Routes::Get(router, "/get-inventory-list", Rest::Routes::bind(&ApiHandler::getInventoryList, &handler));
    Rest::Routes::Get(router, "/get-purchase-orders", Rest::Routes::bind(&ApiHandler::getPurchaseOrders, &handler));
    Rest::Routes::Get(router, "/get-code-master", Rest::Routes::bind(&ApiHandler::getCodeMaster, &handler));

    // テーブル書き込みAPI
    Rest::Routes::Post(router, "/get-category", Rest::Routes::bind(&ApiHandler::setCategory, &handler));
    Rest::Routes::Post(router, "/get-product", Rest::Routes::bind(&ApiHandler::setProducts, &handler));
    
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
