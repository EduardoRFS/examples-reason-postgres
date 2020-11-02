open Opium.Std;
open Pastel;

let hello_world =
  get("/hello", _req => Response.of_plain_text("Hello World") |> Lwt.return);

let run_command = app =>
  switch (App.run_command'(app)) {
  | `Error
  | `Not_running =>
    Console.error(<Pastel color=Red> "failed to start" </Pastel>)
  | `Ok(promise) =>
    Console.log(<Pastel color=Green> "Starting on port 3000" </Pastel>);
    Lwt_main.run(promise);
  };

App.empty |> hello_world |> App.port(3000) |> run_command;
