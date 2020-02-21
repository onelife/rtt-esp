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
    python3 \
    python3-pip \
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
COPY requirements.txt /tmp
RUN python3 -m pip install -U pip setuptools wheel \
    && python3 -m pip install -r /tmp/requirements.txt \
    rm -f /tmp/requirements.txt

# change user
USER rtt
WORKDIR /home/rtt

# esp-idf
# To build the image for a branch or a tag of IDF, pass --build-arg IDF_CLONE_BRANCH_OR_TAG=name.
# To build the image with a specific commit ID of IDF, pass --build-arg IDF_CHECKOUT_REF=commit-id.
# It is possibe to combine both, e.g.:
#   IDF_CLONE_BRANCH_OR_TAG=release/vX.Y
#   IDF_CHECKOUT_REF=<some commit on release/vX.Y branch>.

ARG IDF_CLONE_URL=https://github.com/espressif/esp-idf.git
ARG IDF_CLONE_BRANCH_OR_TAG=master
ARG IDF_CHECKOUT_REF=

ENV IDF_PATH=/home/rtt/esp/idf
ENV IDF_TOOLS_PATH=/home/rtt/esp
ENV RTT_PATH=/home/rtt/rtt

RUN echo IDF_CHECKOUT_REF=$IDF_CHECKOUT_REF IDF_CLONE_BRANCH_OR_TAG=$IDF_CLONE_BRANCH_OR_TAG && \
    git clone --recursive \
      ${IDF_CLONE_BRANCH_OR_TAG:+-b $IDF_CLONE_BRANCH_OR_TAG} \
      $IDF_CLONE_URL $IDF_PATH && \
    if [ -n "$IDF_CHECKOUT_REF" ]; then \
      cd $IDF_PATH && \
      git checkout $IDF_CHECKOUT_REF && \
      git submodule update --init --recursive; \
    fi

RUN $IDF_PATH/install.sh && \
  rm -rf $IDF_TOOLS_PATH/dist \
  rm -rf $IDF_PATH/components/freertos

COPY --chown=rtt entrypoint.sh /home/rtt/esp/entrypoint.sh
RUN chmod u+x /home/rtt/esp/entrypoint.sh

ENTRYPOINT [ "/home/rtt/esp/entrypoint.sh" ]
CMD [ "/bin/bash" ]
