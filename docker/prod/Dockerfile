FROM ghcr.io/elan8/propanedb-base-alpine as builder
WORKDIR /app 
COPY CMakeLists.txt CMakeLists.txt 
COPY protos protos
COPY src src
COPY test test
COPY cmake cmake
RUN mkdir build 
WORKDIR /app/build 
RUN cmake -DCMAKE_BUILD_TYPE=Release .. 
RUN make -j 2

FROM alpine:3.14.1
LABEL org.opencontainers.image.source https://github.com/elan8/propanedb
WORKDIR /app 
RUN mkdir /var/rocksdb && chmod 777 /var/rocksdb
RUN apk add --no-cache libstdc++ supervisor
RUN addgroup -S propane && adduser -S propane -G propane
RUN chown -R propane /app
COPY --from=builder /app/dependencies/deploy/lib/* /usr/local/lib/
COPY --from=builder /app/build/libpropane_lib.a .
COPY --from=builder /app/build/server .
RUN chmod 777 /app/server *
USER propane
EXPOSE 50051
WORKDIR /app
COPY docker/prod/supervisord.conf .
CMD ["/usr/bin/supervisord"]
