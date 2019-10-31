module Date = {
  type format =
    | CLF
    | ISO
    | WEB;

  let month_to_string =
    fun
    | 0 => "Jan"
    | 1 => "Feb"
    | 2 => "Mar"
    | 3 => "Apr"
    | 4 => "May"
    | 5 => "Jun"
    | 6 => "Jul"
    | 7 => "Aug"
    | 8 => "Sep"
    | 9 => "Oct"
    | 10 => "Nov"
    | 11 => "Dec"
    | _ => "-";

  let day_to_string =
    fun
    | 0 => "Sun"
    | 1 => "Mon"
    | 2 => "Tue"
    | 3 => "Wed"
    | 4 => "Thu"
    | 5 => "Fri"
    | 6 => "Sat"
    | _ => "-";

  let print = (format: format) => {
    let {tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, _}: Unix.tm =
      Unix.gettimeofday() |> Unix.gmtime;

    let pad_digits = v =>
      v < 10 ? "0" ++ string_of_int(v) : string_of_int(v);

    switch (format) {
    | ISO =>
      /* common ISO 8601 date time format (2000-10-10T13:55:36.000Z) */
      Format.sprintf(
        "%d-%s-%sT%s:%s:%s.000Z",
        tm_year + 1900,
        pad_digits(tm_mon + 1),
        pad_digits(tm_mday),
        pad_digits(tm_hour),
        pad_digits(tm_min),
        pad_digits(tm_sec),
      )
    | CLF =>
      /* common log format ("10/Oct/2000:13:55:36 +0000") */
      Format.sprintf(
        "%s/%s/%d:%s:%s:%s +0000",
        pad_digits(tm_mday),
        month_to_string(tm_mon),
        tm_year + 1900,
        pad_digits(tm_hour),
        pad_digits(tm_min),
        pad_digits(tm_sec),
      )
    | WEB =>
      /* common RFC 1123 date time format (Tue, 10 Oct 2000 13:55:36 GMT) */
      Format.sprintf(
        "%s, %s %s %d %s:%s:%s GMT",
        day_to_string(tm_wday),
        pad_digits(tm_mday),
        month_to_string(tm_mon),
        tm_year + 1900,
        pad_digits(tm_hour),
        pad_digits(tm_min),
        pad_digits(tm_sec),
      )
    };
  };
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

/* TODO: move to Utils */
let map_opt = fn =>
  fun
  | None => None
  | Some(v) => Some(fn(v));

/* TODO: move to Utils */
let get_or_else = default =>
  fun
  | None => default
  | Some(v) => v;

/* TODO: move to Headers */
let get_header = (key, headers) =>
  headers
  |> List.find_opt(((header, _)) =>
       String.lowercase_ascii(header) == String.lowercase_ascii(key)
     )
  |> map_opt(snd);

let rec print =
        (
          ~response_time,
          ~request: Morph.Request.t,
          ~response: Morph.Response.t,
        ) =>
  fun
  | Dev =>
    /* :method :url :status :response-time ms - :res[content-length] */
    Custom([
      Method,
      Url,
      Status,
      ResponseTime,
      S("ms"),
      S("-"),
      ResHeader("content-length"),
    ])
    |> print(~response_time, ~request, ~response)
  | Tiny =>
    /* :method :url :status :res[content-length] - :response-time ms */
    Custom([
      Method,
      Url,
      Status,
      ResHeader("content-length"),
      S("-"),
      ResponseTime,
      S("ms"),
    ])
    |> print(~response_time, ~request, ~response)
  | Custom(tokens) =>
    tokens
    |> List.map(
         fun
         | Url => request.target
         | Status => response.status |> Morph.Status.to_code |> string_of_int
         | Method => request.meth |> Morph.Method.to_string
         | ResHeader(header) =>
           response.headers |> get_header(header) |> get_or_else("-")
         | ReqHeader(header) =>
           request.headers |> get_header(header) |> get_or_else("-")
         | S(separator) => separator
         | ResponseTime => response_time |> string_of_float
         | UserAgent =>
           request.headers |> get_header("user-agent") |> get_or_else("-")
         | Date(format) => Date.print(format),
       )
    |> List.fold_left((acc, item) => acc ++ " " ++ item, "")
    |> String.trim;

let logger = (~format: t, service, request: Morph.Request.t) => {
  let start_request = Mtime_clock.elapsed();

  service(request)
  |> Lwt.map((response: Morph.Response.t) => {
       let end_request = Mtime_clock.elapsed();
       let response_time =
         Mtime.Span.abs_diff(start_request, end_request) |> Mtime.Span.to_ms;

       Logs.info(m =>
         m("%s", print(~response_time, ~request, ~response, format))
       );

       response;
     });
};
