open Graphql_lwt;

let user =
  Schema.(
    obj("user", ~fields=_ =>
      [
        field(
          "id",
          ~typ=non_null(int),
          ~args=Arg.[],
          ~resolve=(info, user: User.t) =>
          user.id
        ),
        field(
          "name",
          ~typ=non_null(string),
          ~args=Arg.[],
          ~resolve=(info, user: User.t) =>
          user.name
        ),
        field(
          "email",
          ~typ=non_null(string),
          ~args=Arg.[],
          ~resolve=(info, user: User.t) =>
          user.email
        ),
      ]
    )
  );

let schema: Schema.schema(Hmap.t) =
  Schema.(
    schema(
      ~mutations=[
        io_field(
          "addUser",
          ~typ=user,
          ~args=
            Arg.[
              arg("name", non_null(string)),
              arg("email", non_null(string)),
            ],
          ~resolve=(_info, (), name, email) =>
          DB.add_user(~name, ~email)
          |> Lwt.map(
               fun
               | Ok((id, name, email)) =>
                 Some(User.make(~id, ~name, ~email, ()))
               | _ => None,
             )
          |> Lwt_result.ok
        ),
      ],
      [
        io_field(
          "users",
          ~typ=non_null(list(non_null(user))),
          ~args=Arg.[],
          ~resolve=(_info, ()) =>
          DB.get_all()
          |> Lwt.map(
               fun
               | Ok(result) => result
               | Error(_error) => [],
             )
          |> Lwt_result.ok
        ),
        io_field(
          "userById",
          ~typ=user,
          ~args=Arg.[arg("id", non_null(int))],
          ~resolve=(_info, (), id) =>
          DB.get_by_id(id)
          |> Lwt.map(
               fun
               | Ok(Some((id, name, email))) =>
                 Some(User.make(~id, ~name, ~email, ()))
               | _ => None,
             )
          |> Lwt_result.ok
        ),
      ],
    )
  );
