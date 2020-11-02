let (let.await) = Lwt.bind;

[@deriving yojson]
type message = {
  id: string,
  user: string,
  body: string,
};
[@deriving yojson]
type t = list(message);

let read_lines = filename => {
  open Lwt_io;
  let.await ic = open_file(~mode=Input, filename);
  let.await lines = ic |> Lwt_io.read_lines |> Lwt_stream.to_list;
  let.await () = close(ic);
  Lwt.return(lines);
};
let write_file = (filename, ~data) => {
  open Lwt_io;
  let.await oc = open_file(~mode=Output, filename);
  let.await () = Lwt_io.write(oc, data);
  close(oc);
};

let read = () => {
  let.await lines = read_lines("storage.json");
  let storage_result =
    lines |> String.concat("\n") |> Yojson.Safe.from_string |> of_yojson;
  switch (storage_result) {
  | Ok(storage) => Lwt.return(storage)
  | Error(error) => Lwt.fail(Invalid_argument(error))
  };
};
let read_by_id = (~id) => {
  let.await storage = read();
  storage |> List.find_opt(msg => msg.id == id) |> Lwt.return;
};

let write = storage => {
  let json = storage |> to_yojson |> Yojson.Safe.to_string;
  write_file("storage.json", ~data=json);
};

let create = (~user, ~body) => {
  let id = Uuidm.create(`V4) |> Uuidm.to_string;
  let message = {id, user, body};
  let.await storage = read();
  let storage = [message, ...storage];
  let.await () = write(storage);
  message |> Lwt.return;
};

let delete = (~id) => {
  let.await storage = read();
  let storage = storage |> List.filter(msg => msg.id != id);
  write(storage);
};

let update = (~id, ~body) => {
  let.await storage = read();
  let storage =
    storage |> List.map(msg => msg.id == id ? {...msg, body} : msg);
  write(storage);
};
