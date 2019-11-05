open Requery;

let or_error = fun
| Error(error) => Error("DB Error")
| Ok(ok) => Ok(ok);

let add_user = (~email, ~name) => {
  let user = User.{id: Id.generate(), email, name};

  let query = User.Table.insert_one(user);

  let%lwt result = DB.(pool |> insert(~query));

  result |> Belt.Result.map(_, _ => Some(user)) |> or_error |> Lwt.return;
};

let get_all = () => {
  let query = QueryBuilder.([
    e(col("id")), e(col("email")), e(col("name"))
  ] |> from(table(User.Table.table_name)) |> select)

  let%lwt result = {
    DB.pool |> DB.select_all(~table=(module User.Table), ~query)
  }

  Console.log(result);

  result |> Belt.Result.map(_, Array.map(row => {
    User.Table.(
      User.{
        id: row |> get_exn("id"),
        name: row |> get_exn("name"),
        email: row |> get_exn("email"),
      }
    )
  }))
  |> Belt.Result.map(_, Array.to_list) 
  |> or_error
  |> Lwt.return;
};

let get_by_id = id => {
  let query = QueryBuilder.([e(col("id")), e(col("email")), e(col("name"))] |> from(table(User.Table.table_name)) |> select);

  let%lwt result = DB.(pool |> select_one(~table=(module User.Table), ~query));

  result |> Belt.Result.map(_, Belt.Option.map(_, row => {
    User.Table.(
      User.{
        id: row |> get_exn("id"),
        name: row |> get_exn("name"),
        email: row |> get_exn("email"),
      }
    )
  }))
  |> or_error
  |> Lwt.return;
}
