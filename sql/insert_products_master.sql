INSERT INTO products_master (
    product_code,
    product_name,
    category_id,
    updated_userid
)
VALUES ($1, $2, $3, $4);
