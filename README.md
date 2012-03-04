### Introduction
This software provide a suit for sync file and/or directory between two
computers which connected by network.
It has two part, one is server named as sync-server, another is client
named as sync-client.

### Usage
The server start using:

``` bash
$ sync-server [-h ip | --host ip] [-p port | --port port] <recv_path>
```

it will recevie file through port which is default to `8081`, and put
recevied file under `recv_path` which must be directory if existed or server
will create it.

And the client start using:

``` bash
$ sync-client push <--host server-IP | -h server-IP> [-p port | --port port] <path>
```

it will push the file or the all contents under directory to server which
is specified by `server-IP` and `port`(which is default to `8081`).
