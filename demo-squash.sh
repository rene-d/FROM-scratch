#! /bin/sh
set -e

# build image (compile a.c statically)
docker build --quiet --squash -t demo:latest .

# print size in bytes
echo "Docker image size: $(docker inspect demo:latest | jq '.[] | .Size')"
echo "Docker image content:"
docker save demo:latest | tar -xO '*/layer.tar' | tar -tv

# run a.out
docker run -ti --rm demo:latest
