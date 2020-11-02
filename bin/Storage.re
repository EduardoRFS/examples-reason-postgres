let (let.await) = Lwt.bind;

module Db = {
  exception Query_failed(string);
  let connection_url = "postgresql://postgres:password@localhost:5432";
  let pool =
    switch (
      Caqti_lwt.connect_pool(~max_size=10, Uri.of_string(connection_url))
    ) {
    | Ok(pool) => pool
    | Error(err) => failwith(Caqti_error.show(err))
    };

  let migrate_query = [%rapper
    execute(
      {|
        CREATE TABLE IF NOT EXISTS messages (
          id uuid PRIMARY KEY NOT NULL,
          user_id VARCHAR,
          body VARCHAR
        );
      |},
    )
  ];
  let dispatch = f => {
    let.await result = Caqti_lwt.Pool.use(f, pool);
    switch (result) {
    | Ok(data) => Lwt.return(data)
    | Error(error) => Lwt.fail(Query_failed(Caqti_error.show(error)))
    };
  };
  let () = {
    Caqti_lwt.Pool.use(migrate_query(), pool)
    |> Lwt_main.run
    |> (
      fun
      | Ok(_) => ()
      | Error(error) => {
          failwith(Caqti_error.show(error));
        }
    );
  };
};

[@deriving yojson]
type message = {
  id: string,
  user: string,
  body: string,
};
[@deriving yojson]
type t = list(message);

let read = {
  let query = [%rapper
    get_many(
      {sql|
        SELECT @string{id}, @string{user} as user_id, @string{body}
        FROM messages
      |sql},
      record_out,
    )
  ];
  () => query() |> Db.dispatch;
};
let read_by_id = {
  let query = [%rapper
    get_opt(
      {sql|
        SELECT @string{id}, @string{user} as user_id, @string{body}
        FROM messages
        WHERE id = %string{id}
      |sql},
      record_out,
    )
  ];
  (~id) => query(~id) |> Db.dispatch;
};

let create = {
  let query = [%rapper
    execute(
      {sql|
        INSERT INTO messages
        VALUES(%string{id}, %string{user}, %string{body})
      |sql},
      record_in,
    )
  ];
  (~user, ~body) => {
    let id = Uuidm.create(`V4) |> Uuidm.to_string;
    let message = {id, user, body};
    let.await () = query(message) |> Db.dispatch;
    Lwt.return(message);
  };
};

let delete = {
  let query = [%rapper
    execute(
      {sql|
        DELETE FROM messages
        WHERE id = %string{id}
      |sql},
    )
  ];
  (~id) => query(~id) |> Db.dispatch;
};

// TODO: check if id exists
let update = {
  let query = [%rapper
    execute(
      {sql|
        UPDATE messages
        SET body = %string{body}
        WHERE id = %string{id}
      |sql},
    )
  ];
  (~id, ~body) => query(~id, ~body) |> Db.dispatch;
};
