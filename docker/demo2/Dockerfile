FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils build-essential cloc clang

WORKDIR /PRAT

ADD ./src ./src
ADD ./artifacts/mosquitto2 ./mosquitto

# Run the main PRAT functions in a WORKDIR.
WORKDIR /PRAT/mosquitto
RUN make binary -j
RUN ls -lh src/mosquitto && ls -lh lib/libmosquitto.so.1
CMD [ "cloc", "src", "lib" ]
