FROM debian:buster
RUN apt-get update && \
    apt-get install -y libssl-dev libboost1.67 libboost-dev libkrb5-dev cmake g++
COPY . /ssf
WORKDIR /ssf
RUN mkdir build
WORKDIR build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=
RUN make

FROM alpine:latest
WORKDIR /root/
COPY --from=0 /ssf/build/ssfc .
COPY --from=0 /ssf/build/ssfd .
CMD ["./ssfc"]
