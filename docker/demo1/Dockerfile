FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y apt-utils

WORKDIR /PRAT

#RUN mkdir -p ~/git/TARGETS/

ADD ./src ./src
ADD artifacts/mosquitto ./mosquitto
COPY ./setup-docker.sh .

# Setup all the dependencies/etc.
RUN ./setup-docker.sh

# Run the main PRAT functions in a WORKDIR.
WORKDIR /PRAT/src
#CMD ["make", "demo1"]
RUN make demo1
#RUN ./extract_features.pl diff_with_tls
CMD ["./extract_features.pl", "diff_with_tls/"]
