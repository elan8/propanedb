FROM propanedb-prod1 as builder

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
COPY docker-prod2/supervisord.conf .
CMD ["/usr/bin/supervisord"]
