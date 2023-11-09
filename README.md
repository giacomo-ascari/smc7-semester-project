# smc7-semester-project

# Run the web-compiler docker container
The web-compiler takes care of the configuration of the various tools required to compile C++ for the Dubby. The base image of the container is a raw Ubuntu image, to which the build tools, the ARM toolchain and the webserver are later added.

## Manual
1. Change current directory with `cd web-compiler`
2. Build the image with `docker build -t web-compiler .`
3. Run the image interactively with `docker run -i -p 8000:8000 web-compiler`
In order to run the same image detached from the terminal substitute `-i` with `-d`
For M1 users, on step 2, use `docker build -t web-compiler . --platform linux/x86_64`

# Run the python server
Simple python server running on port 8000. It accepts a file, puts it into a directory with a random id, copies necessary build files into that directory and compiles the file.

## Requirements
- Python 3
- uuid
- http.server
- shutil
- subprocess

## Manual
1. python3 server.py
2. Do a request to upload file
`curl -X PUT --upload-file <your_file_name> http://localhost:8000`


