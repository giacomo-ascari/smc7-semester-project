## Requires Python3 !

import os
import uuid
import shutil
import subprocess
   
def compile(requestId):
    # Generate unique id for upload file
    identifier = requestId 
    current_directory = os.getcwd()
    final_directory = os.path.join(current_directory, 'buildspace', rf'{str(identifier)}')
    if not os.path.exists(final_directory):
        os.makedirs(final_directory)
    """Save a file following a HTTP PUT request"""
    filename = f'{final_directory}/' + os.path.basename("Main.cpp")

    # Don't overwrite files...
    if not os.path.exists(filename):
        raise Exception(f"Could not compile, because the Main.cpp for {requestId} could not be found")
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
    subprocess.Popen(["make"], stdout=subprocess.PIPE, cwd=dir).wait()