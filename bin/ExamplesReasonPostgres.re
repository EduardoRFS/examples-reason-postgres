open Opium.Std;

let hello_world =
  get("/hello", _req => Response.of_plain_text("Hello World") |> Lwt.return);

App.empty |> hello_world |> App.port(3000) |> App.run_command;
