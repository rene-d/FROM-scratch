#! /bin/sh
set -e

# build image (compile a.c statically)
docker build --quiet --squash -t z:latest .

# print size in bytes
echo "Docker image size: $(docker inspect z:latest | jq '.[] | .Size')"
echo "Docker image content:"
docker save z:latest | tar -xO '*/layer.tar' | tar -tv

# run a.out
docker run -ti --rm z:latest
