FROM alpine

# build tools for native dependencies
RUN apk add --update gcc g++ make

COPY . /usr/src/
WORKDIR /usr/src/

RUN make
# place all build deps here

CMD [ "./program"]
