# smc7-semester-project

## Folder structure:
**web-compiler**:
Contains everything required to compile a project for DaisyDub. This includes the python-software to execute the compilation and the required libraries for compilation:
- DaisySP
- libDaisy
- Dubby.h
- DspBlock.h

> All files of the web-compiler should be changed with caution!

**playground**:
Contains a playground, to test your DspBlock implementations on either the DaisySeed or DaisyDub.

## Deploy the project locally

In order for the project to run, both web-compiler and web-interface must be set up and running simultaneously.

### Run the web-compiler docker container
The web-compiler takes care of the configuration of the various tools required to generate and compile C++ code for the Dubby. The base image of the container is a raw Ubuntu image, to which the build tools, the ARM toolchain and the webserver are later added.

#### Requirements
- Docker

#### Manual
1. Change current directory with `cd web-compiler`
2. Download [ARM Toolchain](#https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2?revision=ca0cbf9c-9de2-491c-ac48-898b5bbc0443&rev=ca0cbf9c9de2491cac48898b5bbc0443&hash=72D7BCC38C586E3FE39D2A1DB133305C64CA068B
) in the current directory
2. Build the image with `docker build -t webcompiler .`
3. Run the image interactively with `docker run -i -p 5000:5000 webcompiler`

#### Warnings
- In order to run the same image detached from the terminal substitute `-i` with `-d`.
- For M1 users, on step 3, use `docker build -t webcompiler . --platform linux/x86_64`.
- In case Docker Desktop is used you might have some problems with deprecated commands. If installed,. you can use `buildx`, with `docker buildx build -t webcompiler .`.

### Run the web-interface React app
The web interface consists of a React app. Make sure that your browser is updated and avoid running it inside a virtual machine, otherwise WebUSB might not be available for use.

#### Requirements
- NodeJS
- Node Package Manager (NPM)

#### Manual
1. Change current directory with `cd web-interface`
2. Run the installation of packages with `npm i`
3. Run the application with `npm start`
4. Browse to [localhost:3000](http://localhost:3000)


