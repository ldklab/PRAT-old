FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils

WORKDIR /PRAT

ADD ./src ./src
ADD ./artifacts/mosquitto ./mosquitto
ADD ./App/mosquitto ./App/mosquitto
COPY ./setup-docker.sh .

# Setup all the dependencies/etc.
RUN ./setup-docker.sh

# Run the main PRAT functions in a WORKDIR.
WORKDIR /PRAT/src
ENTRYPOINT /bin/bash
