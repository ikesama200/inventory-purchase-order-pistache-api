select 
    inventory_table.product_id,
    products_master.product_code,
    products_master.product_name,
    products_master.category_id,
    categories_master.category_name,
    inventory_table.quantity,
    inventory_table.last_updated
from inventory_table
left join products_master
    on inventory_table.product_id = products_master.product_id
left join categories_master
    on products_master.category_id = categories_master.category_id;