FROM ubuntu:jammy
MAINTAINER Open Source Integrators <support@opensourceintegrators.com>

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
         ccache \
         cmake \
         libad9361-0 \
         libasound2 \
        #  libboost-date-time1.65.1 \
        #  libboost-filesystem1.65.1 \
        #  libboost-program-options1.65.1 \
        #  libboost-thread1.65.1 \
        #  libcomedi0 \
        #  libfftw3-bin \
        #  libgmp10 \
        #  libgsl23 \
        #  libgtk-3-0 \
        #  libiio0 \
        #  libjack-jackd2-0 \
        #  liblog4cpp5v5 \
        #  libpangocairo-1.0-0 \
        #  libportaudio2 \
        #  libqwt6abi1 \
        #  libsdl-image1.2 \
        #  libsndfile1-dev \
        #  libuhd003.010.003 \
        #  libusb-1.0-0 \
        #  libzmq5 \
        #  libpango-1.0-0 \
        #  gir1.2-gtk-3.0 \
        #  gir1.2-pango-1.0 \
        #  pkg-config \
         --no-install-recommends
RUN DEBIAN_FRONTEND=noninteractive \
       apt-get install -qy \
       flex \
       bison \
       automake \
       autoconf \
       libtool


RUN DEBIAN_FRONTEND=noninteractive apt-get install -qy software-properties-common
RUN DEBIAN_FRONTEND=noninteractive add-apt-repository -y ppa:git-core/ppa
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -qy git

# RUN mkdir /src && cd /src \
#     && git clone https://github.com/apache/thrift.git --branch 0.10.0 --depth 1 \
#     && cd thrift \
#     && ./bootstrap.sh \
#     && ./configure --with-c_glib --with-cpp --with-libevent --with-python --without-csharp --without-d --without-erlang --without-go --without-haskell --without-java --without-lua --without-nodejs --without-perl --without-php --without-ruby --without-zlib --without-qt4 --without-qt5 --disable-tests --disable-tutorial --prefix=/usr \
#     && make -j4 install \
#     && mv /usr/lib/python2.7/site-packages/thrift /usr/lib/python2.7/dist-packages
# RUN cd /src/thrift \
#     && PYTHON=/usr/bin/python3 ./configure --with-c_glib --with-cpp --with-libevent --with-python --without-csharp --without-d --without-erlang --without-go --without-haskell --without-java --without-lua --without-nodejs --without-perl --without-php --without-ruby --without-zlib --without-qt4 --without-qt5 --disable-tests --disable-tutorial --prefix=/usr \
#     && make -j4 install \
#     && mv /usr/lib/python3.6/site-packages/thrift /usr/lib/python3/dist-packages

# RUN DEBIAN_FRONTEND=noninteractive \
#         apt-get -y clean \
#         && apt-get -y autoclean \
#         && apt-get -y autoremove \
#         && rm -r /src


RUN DEBIAN_FRONTEND=noninteractive \ 
        apt-get install -qy gfortran

RUN DEBIAN_FRONTEND=noninteractive \ 
        apt-get install -qy netcat \
        && apt-get install -qy net-tools \
        && apt-get install -qy iputils-ping \
        && apt-get install -qy wget \
        && apt-get install -qy vim \
        && apt-get install -qy iproute2


WORKDIR /tmp
RUN git clone -b master https://github.com/signetlabdei/DESERT_Underwater.git \ 
    && cd DESERT_Underwater/DESERT_Framework \
    && ./install.sh --target LOCAL --with-woss --inst_mode development --addons ALL --dest_folder /tmp/DESERT_Underwater/DESERT_buildCopy_LOCAL   

RUN echo "DESERT is installed in the /tmp/ folder" 
RUN echo "IMPORTANT: run the following command before starting" 
RUN echo "source /tmp/DESERT_Underwater/DESERT_buildCopy_LOCAL/environment"