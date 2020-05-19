FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils

ADD src $HOME/src
COPY setup.sh $HOME

RUN ./setup.sh
