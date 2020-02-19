FROM alpine:latest

RUN apk --no-cache add \
      build-base \
      libressl-dev \
      c-ares-dev \
      curl \
      util-linux-dev \
      libwebsockets-dev \
      libxslt \
      python3

EXPOSE 1883
#CMD [ "", "" ]
