[@deriving yojson]
type message = {
  id: string,
  user: string,
  body: string,
};
[@deriving yojson]
type t = list(message);

let read: unit => Lwt.t(list(message));
let read_by_id: (~id: string) => Lwt.t(option(message));
let create: (~user: string, ~body: string) => Lwt.t(message);
let delete: (~id: string) => Lwt.t(unit);
let update: (~id: string, ~body: string) => Lwt.t(unit);
