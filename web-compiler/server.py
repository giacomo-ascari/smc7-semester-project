# SOURCE: https://floatingoctothorpe.uk/2017/receiving-files-over-http-with-python.html
## Requires Python3 !

import os
import uuid
import http.server as server
import shutil
import subprocess
   
class HTTPRequestHandler(server.SimpleHTTPRequestHandler):
    """Put not supported out of the box Extend SimpleHTTPRequestHandler to handle PUT requests"""
    def do_PUT(self):
        # Generate unique id for upload file
        identifier = uuid.uuid4() 
        current_directory = os.getcwd()
        final_directory = os.path.join(current_directory, 'output', rf'{str(identifier)}')
        if not os.path.exists(final_directory):
            os.makedirs(final_directory)
        """Save a file following a HTTP PUT request"""
        filename = f'{final_directory}/' + os.path.basename("Main.cpp")

        # Don't overwrite files...
        if os.path.exists(filename):
            self.send_response(409, 'Conflict')
            self.end_headers()
            reply_body = '"%s" already exists\n' % filename
            self.wfile.write(reply_body.encode('utf-8'))
            return

        file_length = int(self.headers['Content-Length'])
        with open(filename, 'wb') as output_file:
            output_file.write(self.rfile.read(file_length))

        
        self.send_response(201, 'Created')
        self.end_headers()
        reply_body = 'Saved "%s"\n' % filename
        self.wfile.write(reply_body.encode('utf-8'))
        copyBuildFiles(final_directory)
        buildTarget(final_directory)

def copyBuildFiles(toDir):
    currentDirectory = os.getcwd()
    fullPath = os.path.join(currentDirectory, r'build_template')
    files = os.listdir(fullPath)
    shutil.copytree(fullPath, toDir, dirs_exist_ok=True)
   
def buildTarget(dir):
    # subprocess.call(['sh', f'{dir}/build.sh'])
    # subprocess.call(['cd',dir,';','sh', f'./build.sh'])
    subprocess.Popen(["make"], stdout=subprocess.PIPE, cwd=dir)


if __name__ == '__main__':
    server.test(HandlerClass=HTTPRequestHandler)