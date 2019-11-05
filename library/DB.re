let connection_url = "postgresql://postgres:postgres@localhost:5432/db_name";
let pool = Requery.Client.create_pool(~size=8, ~connection_url, ());

include Requery.Client;
