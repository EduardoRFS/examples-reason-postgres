open Opium.Std;

let (let.await) = Lwt.bind;

let not_found = Response.make(~status=`Not_found, ()) |> Lwt.return;

let json_from_req = (f, req) => {
  let.await json_string = req.Request.body |> Body.to_string;
  switch (json_string |> Yojson.Safe.from_string |> f) {
  | Ok(data) => Lwt.return(data)
  | Error(error) => Lwt.fail(Invalid_argument(error))
  };
};
let response_from_json = (~status=?, f, data) => {
  let response =
    switch (status) {
    | Some(status) => Response.of_json(~status, f(data))
    | None => Response.of_json(f(data))
    };
  response |> Lwt.return;
};

let read_all =
  get("/messages", _req => {
    let.await storage = Storage.read();
    storage |> response_from_json(Storage.to_yojson);
  });
let read_by_id =
  get("/message/:id", req => {
    let id = Router.param(req, "id");
    let.await found_message = Storage.read_by_id(~id);
    switch (found_message) {
    | Some(message) =>
      message |> response_from_json(Storage.message_to_yojson)
    | None => not_found
    };
  });

[@deriving yojson]
type submit_message = {
  user: string,
  body: string,
};

let create =
  post("/message", req => {
    let.await payload = req |> json_from_req(submit_message_of_yojson);

    let.await message =
      Storage.create(~user=payload.user, ~body=payload.body);

    message |> response_from_json(~status=`Created, Storage.message_to_yojson);
  });

[@deriving yojson]
type update_message = {body: string};
let update_by_id =
  put("/message/:id", req => {
    let id = Router.param(req, "id");
    let.await payload = req |> json_from_req(update_message_of_yojson);
    let.await () = Storage.update(~id, ~body=payload.body);
    Response.make(~status=`No_content, ()) |> Lwt.return;
  });

let delete_by_id =
  delete("/message/:id", req => {
    let id = Router.param(req, "id");
    let.await () = Storage.delete(~id);
    Response.make(~status=`No_content, ()) |> Lwt.return;
  });

let app =
  App.empty
  |> create
  |> read_all
  |> read_by_id
  |> update_by_id
  |> delete_by_id
  |> App.port(3000);

let main = () => {
  Logs.set_reporter(Logs_fmt.reporter());
  Logs.set_level(Some(Logs.Error));

  switch (App.run_command'(app)) {
  | `Error
  | `Not_running =>
    Console.error(<Pastel color=Red> "failed to start" </Pastel>)
  | `Ok(promise) =>
    Console.log(<Pastel color=Green> "Starting on port 3000" </Pastel>);
    Lwt_main.run(promise);
  };
};
main();
