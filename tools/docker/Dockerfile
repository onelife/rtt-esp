FROM ubuntu:18.04

ARG ROOT_PASSWORD
ARG USER_PASSWORD
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get -yq dist-upgrade \
    && apt-get install -yq --no-install-recommends \
    sudo \
    tmux \
    python3 \
    python3-pip \
    apt-utils \
    bison \
    ca-certificates \
    ccache \
    check \
    cmake \
    curl \
    flex \
    git \
    gperf \
    lcov \
    libncurses-dev \
    libusb-1.0-0-dev \
    make \
    ninja-build \
    unzip \
    wget \
    xz-utils \
    zip \
   && apt-get autoremove -y \
   && rm -rf /var/lib/apt/lists/* \
   && update-alternatives --install /usr/bin/python python /usr/bin/python3 10

# user and password
RUN echo "root:$ROOT_PASSWORD" | chpasswd \
    && groupadd rtt \
    && useradd -d /home/rtt -ms /bin/bash -g rtt -G sudo rtt \
    && echo "rtt:$USER_PASSWORD" | chpasswd

# python library
COPY requirements.txt /home/rtt/
RUN python3 -m pip install -U pip setuptools wheel \
    && python3 -m pip install -r /home/rtt/requirements.txt

# change user
USER rtt
WORKDIR /home/rtt
