---
title: Install 3dfier using a Docker image
keywords: install docker
sidebar: 3dfier_sidebar
permalink: install_docker.html
---
The Dockerfile for building 3dfier (`Dockerfile`) contains the complete instruction for setting up a build environment. If you don't want to install the required dependencies on your local machine, you can use this docker image to build and run 3dfier. If you want to build 3dfier locally, then please look into the Dockerfile for the libraries that you'll need.

## Using Docker to compile and run 3dfier

The builder image (`Dockerfile_builder`) does not contain the source code of 3dfier, neither the executable. We use this image for compiling 3dfier from different branches during development and testing. Nevertheless, it is suitable for 3dfying any data set.

The complete process of building the docker image, compiling 3dfier and running looks like the following.


**1. Build the docker image**

Build the docker image. If you are not familiar with `docker build`, please refer to the [documentation](https://docs.docker.com/engine/reference/commandline/build/). 

In this case the image is named `3dfier` and tagged as `latest`. You can choose any name and tag.

```
$ docker build -t 3dfier:latest -f Dockerfile .
```

**2. Running 3dfier in a container**

The executable is compiled and stored in the docker image. For running *3dfier* you need to create a container and execute *3dfier* in there.

By setting up [bind mounts](https://docs.docker.com/storage/bind-mounts/) or [volumes](https://docs.docker.com/storage/volumes/) you can map your local data to the container so *3dfier* can access it.

In case the input data is stored in a database, and you don't want to fiddle with setting up the networking, you can use `--network=host` to map the network ports from your host to the container. 

An example command would be then:

```
$ docker run --rm --network=host -v "$(pwd)":/data 3dfier:latest 3dfier testarea_config_docker.yml --CSV-BUILDINGS test.csv
```