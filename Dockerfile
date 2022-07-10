FROM ubuntu:focal

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y make gcc g++ libgl-dev libglu1-mesa-dev libalut-dev

VOLUME /trunk

WORKDIR /trunk/src
ENTRYPOINT ["/trunk/src/build.sh"]
