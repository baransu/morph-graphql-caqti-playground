open Requery;

module Impl = {
  type t = {
    id: string,
    name: string,
    email: string,
  };

  let make = (~id, ~name, ~email, ()) => {id, name, email};

  let tname = "users";

  let columns =
    QueryBuilder.[
      Table.Col.make(
        ~key="id",
        ~encode=t => string(t.id),
        ~primary_key=true,
        ~unique=true,
        (),
      ),
      Table.Col.make(~key="name", ~encode=t => string(t.name), ()),
      Table.Col.make(
        ~key="email",
        ~encode=t => string(t.email),
        ~unique=true,
        (),
      ),
    ];
};

module Table = Requery.Table.Make(Impl);
include Impl;
