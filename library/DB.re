let connection_url = "postgresql://postgres:postgres@localhost:5432/db_name";


/* TODO: in morph g two things should be provided, logger and pool */

/* This is the connection pool we will use for executing DB operations. */
let pool =
  switch (
    Caqti_lwt.connect_pool(~max_size=10, Uri.of_string(connection_url))
  ) {
  | Ok(pool) => pool
  | Error(err) => failwith(Caqti_error.show(err))
  };

type error =
  | Database_error(string);

let or_error = m =>
  switch%lwt (m) {
  | Ok(a) => Ok(a) |> Lwt.return
  | Error(e) => 
  let error = Caqti_error.show(e);
    Console.log(error);
    Error(Database_error(error)) |> Lwt.return;
  };

let migrate_query =
  Caqti_request.exec(
    Caqti_type.unit,
    {|CREATE TABLE users (
        id SERIAL NOT NULL PRIMARY KEY,
        name VARCHAR NOT NULL,
        email VARCHAR NOT NULL
      )
    |},
  );

let migrate = () =>
  pool
  |> Caqti_lwt.Pool.use((module C: Caqti_lwt.CONNECTION) =>
       C.exec(migrate_query, ())
     )
  |> or_error
  |> Lwt.map(ignore);

let rollback_query = Caqti_request.exec(Caqti_type.unit, "DROP TABLE IF EXISTS users");

let rollback = () =>
  pool
  |> Caqti_lwt.Pool.use((module C: Caqti_lwt.CONNECTION) =>
       C.exec(rollback_query, ())
     )
  |> or_error
  |> Lwt.map(ignore);

let get_all_query =
  Caqti_request.collect(
    Caqti_type.unit,
    Caqti_type.(tup3(int32, string, string)),
    "SELECT id, name, email FROM users",
  );

let get_all = () => {
  pool
  |> Caqti_lwt.Pool.use((module C: Caqti_lwt.CONNECTION) =>
       C.fold(
         get_all_query, /* query */
         ((id, name, email), acc) => {
           [User.make(~id, ~name, ~email, ()), ...acc]
         },
         (), /* arguments */
         [] /* accumulator */
       )
     )
  |> or_error;
};

let get_by_id_query =
  Caqti_request.find_opt(
    Caqti_type.int,
    Caqti_type.(tup3(int32, string, string)),
    "SELECT id, name, email FROM users WHERE id = ?",
  );

let get_by_id = id => {
  pool
  |> Caqti_lwt.Pool.use((module C: Caqti_lwt.CONNECTION) =>
       C.find_opt(get_by_id_query, id) 
     )
  |> or_error;
};


let add_user_query =
  Caqti_request.find(
    Caqti_type.(tup2(string, string)),
    Caqti_type.(tup3(int32, string, string)),
    "INSERT INTO users (name, email) VALUES ? RETURNING id, name, email",
  );

let add_user = (~name, ~email) => {
  pool
  |> Caqti_lwt.Pool.use((module C: Caqti_lwt.CONNECTION) =>
       C.find(add_user_query, (name, email)) 
     )
  |> or_error;
};
