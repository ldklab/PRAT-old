FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y
RUN apt-get install -y xdot 

ADD ./artifacts/graphs ./graphs

# Just show xdot output? Test later.
CMD ["xdot", "callgraph_tls.dot"]
