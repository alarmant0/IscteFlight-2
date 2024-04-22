# IscteFlight-2

This project was developed as part of the Operating Systems course at ISCTE and is a flight control system application.

## About the Project

IscteFlight-2 is a simplified flight control system consisting of a server and a client. The server controls flight data, and the client interacts with the server to send commands and receive check-in information.

## Project Architecture

![Arquitetura](https://github.com/alarmant0/IscteFlight-2/blob/main/arquitetura.png)

**Requirements**

- Bash interpreter.
- Linux or macOS operating system (may work on Windows with WSL or Cygwin).

## How to Use

### Clone the Repository

```bash
git clone https://github.com/seu_utilizador/IscteFlight-2.git
```

Execution

Server and Client:

```bash
gcc servidor.c -o servidor
gcc cliente.c -o cliente
```

```bash
./servidor
./cliente
```

## Integrity Check

To verify the integrity of the project, you must:

```
cd so_2023_trab2_validator
./so_2023_trab2_validator.py ..
```


## Documentação

For more details, please refer to the [documentation in PDF](https://github.com/alarmant0/IscteFlight-2/blob/main/so-2022-practical-assignment-part-2-v3.pdf). ( Only availabe in Portuguese /: )


# License
This project is licensed under the [MIT License](LICENSE).

:)
