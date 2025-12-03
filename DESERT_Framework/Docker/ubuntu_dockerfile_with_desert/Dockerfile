FROM ubuntu:noble

SHELL ["/bin/bash", "-xo", "pipefail", "-c"]

# Generate locale C.UTF-8
ENV LANG C.UTF-8
ENV TZ=UTC
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update \
  && apt upgrade -y \
  && apt install -y --no-install-recommends \
    ca-certificates \
    curl \
    htop \
    iotop \
    rsync \
    tar \
    vim \
    unzip \
  && apt clean all \
  && apt autoremove

# CPP deps
RUN DEBIAN_FRONTEND=noninteractive \
       apt-get install -qy \
         build-essential \
         autoconf \
         automake \
         libxmu-dev \
         libx11-dev \
         libxmu-headers \
         libxt-dev \
         libxml2-dev \
         libtool \
         bison \
         flex \
         --no-install-recommends

RUN DEBIAN_FRONTEND=noninteractive apt-get install -qy software-properties-common
RUN DEBIAN_FRONTEND=noninteractive add-apt-repository -y ppa:git-core/ppa
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -qy git


RUN DEBIAN_FRONTEND=noninteractive \ 
        apt-get install -qy gfortran

RUN DEBIAN_FRONTEND=noninteractive \ 
        apt-get install -qy netcat-traditional \
        && apt-get install -qy net-tools \
        && apt-get install -qy iputils-ping \
        && apt-get install -qy wget \
        && apt-get install -qy iproute2

WORKDIR /home
RUN git clone -b master https://github.com/signetlabdei/DESERT_Underwater.git \ 
        && cd DESERT_Underwater/DESERT_Framework \
        && ./install.sh --target LOCAL --with-woss --inst_mode development --addons ALL --dest_folder /home/DESERT_Underwater/DESERT_buildCopy_LOCAL   

WORKDIR /home/DESERT_Underwater/DESERT_Framework/DESERT/samples/desert_samples

RUN chmod +x /home/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment

ENTRYPOINT ["/bin/bash", "-c", "source /home/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment && exec \"$@\"", "--"]
