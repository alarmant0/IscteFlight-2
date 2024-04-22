# IscteFlight-2

Este projeto foi feito no âmbito da cadeira Sistemas Operativos no ISCTE e é uma aplicação de sistema de controle de vôo desenvolvido.

## Sobre o Projeto

O IscteFlight-2 é um sistema de controle de vôo simplificado que consiste em um servidor e um cliente. O servidor controla os dados de vôo e o cliente interage com o servidor para enviar comandos e receber informações sobre os checkins.

## Arquitetura do Projeto

![Arquitetura](https://github.com/alarmant0/IscteFlight-2/blob/main/arquitetura.png)

## Como Usar

### Clonar o Repositório

```bash
git clone https://github.com/seu_utilizador/IscteFlight-2.git
```

### Executar

O servidor:

```bash
gcc servidor.c -o servidor
gcc cliente.c -o cliente
```

```bash
./servidor
./cliente
```

## Verificação de integridade

Para verificar a integridade do projeto tem de:

```
cd so_2023_trab2_validator
./so_2023_trab2_validator.py ..
```

## Documentação

Para mais detalhes, consulte a [documentação em PDF](https://github.com/alarmant0/IscteFlight-2/blob/main/so-2022-practical-assignment-part-2-v3.pdf).



# Licença
Este projeto está licenciado sob a Licença MIT.

:)
