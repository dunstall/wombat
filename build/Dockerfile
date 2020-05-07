FROM rust:1.41 AS build

WORKDIR /usr/local/wombat
COPY . .
WORKDIR /usr/local/wombat/server

RUN cargo build --release


FROM ubuntu

WORKDIR /usr/local/wombat

COPY --from=build /usr/local/wombat/target/release/wombatbroker wombatbroker

CMD ./wombatbroker
