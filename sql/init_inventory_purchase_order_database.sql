-- データベース作成
CREATE DATABASE inventory_purchase_order
    WITH ENCODING 'UTF8'
    LC_COLLATE='en_US.UTF-8'
    LC_CTYPE='en_US.UTF-8'
    TEMPLATE=template0;

\connect inventory_purchase_order;

-- テーブル: categories_master
CREATE TABLE categories_master (
    category_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- テーブル: products_master
CREATE TABLE products_master (
    product_id SERIAL PRIMARY KEY,
    product_code VARCHAR(50) UNIQUE NOT NULL,
    product_name VARCHAR(255) NOT NULL,
    category_id INTEGER REFERENCES categories_master(category_id),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_userid VARCHAR(20)
);

-- テーブル: inventory_table
CREATE TABLE inventory_table (
    product_id INTEGER PRIMARY KEY REFERENCES products_master(product_id),
    quantity INTEGER NOT NULL,
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- テーブル: purchase_orders
CREATE TABLE purchase_orders (
    order_id SERIAL PRIMARY KEY,
    order_date DATE NOT NULL,
    status INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_userid VARCHAR(20),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_userid VARCHAR(20)
);

-- テーブル: purchase_order_items
CREATE TABLE purchase_order_items (
    order_item_id SERIAL PRIMARY KEY,
    order_id INTEGER NOT NULL REFERENCES purchase_orders(order_id),
    product_id INTEGER NOT NULL REFERENCES products_master(product_id),
    quantity INTEGER NOT NULL,
    expected_date DATE
);

-- テーブル: users
CREATE TABLE users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- テーブル: code_master
CREATE TABLE code_master (
    code_category VARCHAR(50) NOT NULL,
    code_value VARCHAR(50) NOT NULL,
    code_name VARCHAR(100) NOT NULL,
    description TEXT,
    sort_order INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (code_category, code_value)
);
