FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils build-essential clang xdot

WORKDIR /PRAT

ADD ./src ./src
ADD ./artifacts/mosquitto2 ./mosquitto

# Run the main PRAT functions in a WORKDIR.
WORKDIR /PRAT/mosquitto
#CMD ["make", "binary", "-j"]
