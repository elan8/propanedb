FROM alpine:3.14
RUN apk add --update alpine-sdk
WORKDIR /propanedb
COPY buildBoost.sh buildBoost.sh
RUN ls -la
RUN ./buildBoost.sh
#RUN apk add --no-cache mysql-client
#ENTRYPOINT ["mysql"]