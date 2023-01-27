FROM ubuntu:22.04

WORKDIR /app

RUN set -xe &&\
    apt update &&\
    apt install -y build-essential &&\
    apt-get install -y manpages-dev &&\
    gcc --version

COPY . /app/