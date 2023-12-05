from flask import Flask, request, send_file
from codegen.cpp_parse import *
from compile import *
import json
import io
import uuid


app = Flask(__name__)

@app.route("/compiler", methods=['POST'])
def getBinary():
    try:
        data = request.get_json()
        reqId = uuid.uuid4() 
        
        if not genCpp(data, reqId):
            app.logger.error("Couldn't generate code")
            return "Error", 404
        
        compile(reqId)

        current_directory = os.getcwd()
        final_directory = os.path.join(current_directory, 'buildspace', rf'{str(reqId)}', 'build')
        filename = f'{final_directory}/' + os.path.basename("Main.bin")

        if not os.path.exists(filename):
            return "Build failed", 404

        return send_file(filename, as_attachment=True)
    except Exception as e:
        return str(e)