INSERT INTO inventory_table (
    product_id,
    quantity
)
VALUES ($1, $2)
ON CONFLICT (product_id) 
DO UPDATE SET
    quantity = inventory_table.quantity + EXCLUDED.quantity,
    last_updated = CURRENT_TIMESTAMP;
