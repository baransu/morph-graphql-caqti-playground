// open Library;
// /* It's required because sqlf uses PGOCaml module internaly */
// module PGOCaml = DB.PostgreSQL;
// module Query = {
//   let all = [%sqlf "SELECT * from users;"];
// };
// let () = {
//   Migration.migrate();
//   let pool = DB.create_pool(~size=16, ());
//   User.insert(~id=1l, ~name="John", ~email="john@doe.com")
//   |> DB.run(~pool)
//   |> Lwt.map(result =>
//        result |> List.iter(id => Console.log(Int32.to_int(id)))
//      )
//   |> Lwt_main.run;
//   Query.all
//   |> DB.run(~pool)
//   |> Lwt.map(result =>
//        result
//        |> List.iter(user => {
//             let user = User.untupled(user);
//             Console.log(user);
//            })
//      )
//   |> Lwt_main.run;
// };

Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

let graphql_handler = Morph_graphql_server.make(Library.Schema.schema);

let handler = (request: Morph.Request.t) => {
  open Morph;

  /* TODO: move to Morph  as Morph.Uri.split ??? */
  let path_parts =
    request.target
    |> Uri.of_string
    |> Uri.path
    |> String.split_on_char('/')
    |> List.filter(s => s != "");

  switch (request.meth, path_parts) {
  | (_, ["graphql"]) => graphql_handler(request)
  | (_, _) => Response.not_found(Response.empty)
  };
};

let http_server = Morph_server_http.make();

let () = {
  /* We drop users and create new on every start */
  let _ = Library.DB.rollback() |> Lwt_main.run;
  let _ = Library.DB.migrate() |> Lwt_main.run;

  handler
  |> Morph.start(
       ~servers=[http_server],
       ~middlewares=[Library.Middleware.(logger(~format=Tiny))],
     )
  |> Lwt_main.run;
};
