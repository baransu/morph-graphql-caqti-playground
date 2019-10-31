module Date: {
  type format =
    | CLF
    | ISO
    | WEB;
};

type t =
  | Dev
  | Tiny
  | Custom(list(token))
and token =
  | Url
  | Status
  | Method
  | ResHeader(string)
  | ReqHeader(string)
  | S(string)
  | ResponseTime
  | UserAgent
  | Date(Date.format);

let logger: (~format: t) => Morph.Server.middleware;
