-- categories_master
INSERT INTO categories_master (name)
VALUES
  ('食品'),
  ('飲料'),
  ('文房具'),
  ('家電'),
  ('衣料品');

-- products_master
INSERT INTO products_master 
(product_code, product_name, category_id, created_at, updated_at, updated_userid)
VALUES
  ('FOOD-001', 'おにぎり',        1, NOW(), NOW(), 'system'),
  ('FOOD-002', 'カップラーメン',  1, NOW(), NOW(), 'system'),
  ('DRINK-001', 'ミネラルウォーター', 2, NOW(), NOW(), 'system'),
  ('DRINK-002', 'コーヒー',        2, NOW(), NOW(), 'system'),
  ('STAT-001', 'ボールペン',       3, NOW(), NOW(), 'system'),
  ('STAT-002', 'ノート',           3, NOW(), NOW(), 'system'),
  ('ELEC-001', 'USBケーブル',      4, NOW(), NOW(), 'system'),
  ('ELEC-002', 'イヤホン',         4, NOW(), NOW(), 'system'),
  ('CLOT-001', 'Tシャツ',          5, NOW(), NOW(), 'system'),
  ('CLOT-002', 'ジーンズ',         5, NOW(), NOW(), 'system');

-- inventory_table
INSERT INTO inventory_table (product_id, quantity, last_updated)
VALUES
  (1, 100, NOW()),  -- おにぎり
  (2,  50, NOW()),  -- カップラーメン
  (3, 200, NOW()),  -- ミネラルウォーター
  (4, 150, NOW()),  -- コーヒー
  (5, 300, NOW()),  -- ボールペン
  (6, 120, NOW()),  -- ノート
  (7,  80, NOW()),  -- USBケーブル
  (8,  60, NOW()),  -- イヤホン
  (9,  70, NOW()),  -- Tシャツ
  (10, 40, NOW());  -- ジーンズ

-- purchase_orders
INSERT INTO purchase_orders (order_date, status, created_at, created_userid, updated_at, updated_userid)
VALUES
  (CURRENT_DATE - INTERVAL '10 days', 1, NOW(), 'user1', NOW(), 'user1'), -- 新規発注
  (CURRENT_DATE - INTERVAL '5 days',  2, NOW(), 'user2', NOW(), 'user2'), -- 承認済
  (CURRENT_DATE,                     1, NOW(), 'user1', NOW(), 'user1'); -- 今日の発注

-- purchase_order_items
INSERT INTO purchase_order_items (order_id, product_id, quantity, expected_date)
VALUES
  -- 発注1（食品）
  (1, 1, 50, CURRENT_DATE + INTERVAL '7 days'),  -- おにぎり
  (1, 2, 30, CURRENT_DATE + INTERVAL '7 days'),  -- カップラーメン

  -- 発注2（文房具）
  (2, 5, 100, CURRENT_DATE + INTERVAL '14 days'), -- ボールペン
  (2, 6, 80,  CURRENT_DATE + INTERVAL '14 days'), -- ノート

  -- 発注3（家電）
  (3, 7, 20,  CURRENT_DATE + INTERVAL '10 days'), -- USBケーブル
  (3, 8, 15,  CURRENT_DATE + INTERVAL '10 days'); -- イヤホン

-- users
INSERT INTO users (username, created_at, updated_at)
VALUES
  ('user1', NOW(), NOW()),
  ('user2', NOW(), NOW()),
  ('admin', NOW(), NOW());
-- code_master
INSERT INTO code_master (code_category, code_value, code_name, description, sort_order, created_at, updated_at)
VALUES
  -- ORDER_STATUS
  ('ORDER_STATUS', '1', '新規', '新規登録された発注', 1, NOW(), NOW()),
  ('ORDER_STATUS', '2', '承認済', '承認が完了した発注', 2, NOW(), NOW()),
  ('ORDER_STATUS', '3', '納品済', '発注品が納品された状態', 3, NOW(), NOW()),

  -- USER_TYPE
  ('USER_TYPE', 'NORMAL', '一般ユーザー', 'システムを利用できる通常ユーザー', 1, NOW(), NOW()),
  ('USER_TYPE', 'ADMIN',  '管理者',       'システム全体を管理できるユーザー', 2, NOW(), NOW());
