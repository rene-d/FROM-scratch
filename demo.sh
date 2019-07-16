#! /bin/sh
set -e

# build image (compile a.c statically)
docker build --quiet -t demo:latest .

# print size in bytes
echo "Docker image size: $(docker inspect demo:latest | jq '.[] | .Size')"

echo "Docker container filesystem:"
docker create --rm -t --name demo_list demo:latest > /dev/null
docker export demo_list | tar -tv

# run a.out then quit and remove the container
docker start -i demo_list
