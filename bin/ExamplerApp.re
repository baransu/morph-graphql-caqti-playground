open Library;

Fmt_tty.setup_std_outputs();
Logs.set_level(Some(Logs.Info));
Logs.set_reporter(Logs_fmt.reporter());

let graphql_handler = Morph_graphql_server.make(Library.Schema.schema);

let handler = (request: Morph.Request.t) => {
  open Morph;

  /* TODO: move to Morph as Morph.Uri.split ??? */
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
  let Ok(_) =
    DB.pool
    |> DB.drop_table(~table_name=User.Table.table_name)
    |> Lwt_main.run;
  let Ok(_) =
    DB.pool
    |> DB.create_table(~query=User.Table.create_table())
    |> Lwt_main.run;

  handler
  |> Morph.start(
       ~servers=[http_server],
       ~middlewares=[Library.Middleware.(logger(~format=Tiny))],
     )
  |> Lwt_main.run;
};
