-- 商品マスタ全件取得
select
    product_id,
    product_code,
    product_name,
    category_id,
    created_at,
    updated_at,
    updated_userid
from products_master;