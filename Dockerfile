FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y make gcc g++ libgl-dev libglu1-mesa-dev libalut-dev libgtk-3-dev pkg-config

VOLUME /trunk

WORKDIR /trunk/src
ENTRYPOINT ["/trunk/src/build_lin64.sh"]
