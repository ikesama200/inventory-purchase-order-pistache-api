select
    purchase_orders.order_id,
    purchase_orders.order_date,
    purchase_orders.status,
    purchase_orders.created_at,
    purchase_orders.created_userid,
    creat_users.username
    purchase_orders.updated_at,
    purchase_orders.updated_userid,
    update_users.username,
    purchase_order_items.order_item_id,
    purchase_order_items.quantity,
    purchase_order_items.expected_date,
    purchase_order_items.product_id,
    products_master.product_name,
    products_master.category_id,
    categories_master.category_name
-- 発注書ヘッダーテーブル
from purchase_orders
-- 発注書明細テーブル
inner join purchase_order_items
    on purchase_orders.order_id = purchase_order_items.order_id
left join products_master
    on purchase_order_items.product_id = products_master.product_id
left join categories_master
    on products_master.category_id = categories_master.category_id
left join users as creat_users
    on purchase_orders.created_userid = creat_users.user_id
left join users as update_users
    on purchase_orders.created_userid = update_users.user_id
;