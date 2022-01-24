FROM ghcr.io/elan8/propanedb-base-ubuntu  as builder
WORKDIR /app 
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
RUN mkdir build 
WORKDIR /app/build 
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN make

FROM ubuntu:20.04
RUN apt-get update -y && apt-get -y --no-install-recommends install supervisor libgflags-dev  ca-certificates \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
LABEL org.opencontainers.image.source https://github.com/elan8/propanedb
WORKDIR /app 
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
RUN addgroup --system propane && adduser --system propane --ingroup propane
RUN chown -R propane /app
# COPY --from=builder /app/dependencies/deploy/lib/* /usr/local/lib/
COPY --from=builder /app/dependencies/deploy/lib/libboost* /usr/local/lib/
COPY --from=builder /app/dependencies/deploy/lib/libglog* /usr/local/lib/
COPY --from=builder /app/dependencies/deploy/lib/libPocoFoundation* /usr/local/lib/
COPY --from=builder /app/dependencies/deploy/lib/libPocoZip* /usr/local/lib/
# COPY --from=builder /app/build/libpropane_lib.a .
COPY --from=builder /app/build/server .
ENV LD_LIBRARY_PATH="/usr/lib;/usr/local/lib:${LD_LIBRARY_PATH}"
RUN chmod 777 /app/server *
USER propane
EXPOSE 50051
WORKDIR /app
COPY docker/prod/supervisord.conf .
CMD ["/usr/bin/supervisord"]
