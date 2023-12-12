from flask import Flask, request, send_file
from flask_cors import CORS
from codegen.cpp_parse import *
from compile import *
import json
import io
import uuid


app = Flask(__name__)
CORS(app)

@app.route("/compiler", methods=['POST'])
def getBinary():
    try:
        data = request.get_json()
        reqId = uuid.uuid4() 
        
        try:
            genCpp(data, reqId)
        except Exception as e:
            app.logger.error(e)
            # traceback.print_stack()
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