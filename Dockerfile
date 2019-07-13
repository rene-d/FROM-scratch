FROM alpine AS builder
RUN apk add gcc musl-dev glib-static glib-dev jansson-dev upx
COPY src /src
RUN gcc -O2 -static /src/*.c `pkg-config --cflags --libs glib-2.0 jansson` \
&&  strip a.out \
&&  upx -q a.out || true

FROM scratch
COPY --from=builder /a.out /
COPY data /data
ENTRYPOINT ["/a.out"]
