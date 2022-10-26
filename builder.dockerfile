FROM debian:bullseye

# Source: https://stackoverflow.com/questions/14170590/building-qt-5-on-linux-for-windows/14170591

# http://mxe.cc/#requirements
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		autoconf \
		automake \
		autopoint \
		bash \
		bison \
		bzip2 \
		flex \
		g++ \
		g++-multilib \
		gettext \
		git \
		gperf \
		intltool \
		libc6-dev-i386 \
		libgdk-pixbuf2.0-dev \
		libltdl-dev \
		libssl-dev \
		libtool-bin \
		libxml-parser-perl \
		lzip \
		make \
		openssl \
		p7zip-full \
		patch \
		perl \
		python3 \
		python-is-python3 \
		python3-mako \
		ruby \
		sed \
		unzip \
		wget \
		xz-utils &&\
	apt-get clean autoclean && apt-get autoremove --yes

# Preapre and download cross development environment
RUN mkdir /build
WORKDIR  /build
RUN git clone https://github.com/mxe/mxe.git

# Build cross environment
RUN cd mxe && make MXE_TARGETS=x86_64-w64-mingw32.static qtbase

# Enhance path
ENV PATH /build/mxe/usr/bin:$PATH

# Add a qmake alias
RUN ln -s /build/mxe/usr/bin/x86_64-w64-mingw32.static-qmake-qt5 /build/mxe/usr/bin/qmake

RUN ln /build/mxe/usr/bin/* /usr/local/bin

# Switch to the source directory. The source code will have to be mounted as a volume.
RUN mkdir /src
WORKDIR /src