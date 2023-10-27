# smc7-semester-project

# Run the python server
Simple python server running on port 8080. It accepts a file, puts it into a directory with a random id, copies necessary build files into that directory and compiles the file.

## Requirements
- Python 3
- uuid
- http.server
- shutil
- subprocess

## Manual
1. python server.py
2. Do a request to upload file
`curl -X PUT --upload-file <your_file_name> http://localhost:8000`