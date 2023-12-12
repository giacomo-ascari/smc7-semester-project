FROM ubuntu:22.04

WORKDIR /app

# update ubuntu
RUN DEBIAN_FRONTEND=noninteractive apt-get update -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y python3-pip
RUN DEBIAN_FRONTEND=noninteractive pip3 install flask
RUN DEBIAN_FRONTEND=noninteractive pip3 install flask_cors

# copy the current folder to the image
COPY . .

# setting up the ARM toolchain
# if no file in repo, download...
# https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2?revision=ca0cbf9c-9de2-491c-ac48-898b5bbc0443&rev=ca0cbf9c9de2491cac48898b5bbc0443&hash=72D7BCC38C586E3FE39D2A1DB133305C64CA068B
RUN tar xjf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
ENV GCC_PATH=/app/gcc-arm-none-eabi-10-2020-q4-major/bin
ENV PATH=$GCC_PATH:$PATH
RUN echo $PATH

# check ARM toolchain
RUN arm-none-eabi-gcc --version

WORKDIR /app/build_template/lib/libDaisy
RUN make

WORKDIR /app/build_template/lib/DaisySP
RUN make

WORKDIR /app
# set up 
#RUN

# start webserver
CMD python3 backend.py