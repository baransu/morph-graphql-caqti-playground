type t = {
  id: int,
  name: string,
  email: string,
};

let make = (~id, ~name, ~email, ()) => {id: Int32.to_int(id), name, email};
