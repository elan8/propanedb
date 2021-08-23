#FROM propanedb-prod1 as builder

FROM jevon82/propanedb-base as builder

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
WORKDIR /app 
COPY --from=builder /app/dependencies/deploy/lib/* /usr/local/lib/
COPY --from=builder /app/build/libpropane_lib.a .
COPY --from=builder /app/build/server .
RUN chmod 777 /app/server

RUN apk add --no-cache libstdc++ supervisor
RUN addgroup -S propane && adduser -S propane -G propane
RUN chown -R propane /app

USER propane
EXPOSE 50051
WORKDIR /app
RUN ls -la
COPY docker-prod2/supervisord.conf .
CMD ["/usr/bin/supervisord"]
