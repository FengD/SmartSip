import logging
from robyn import Robyn, html
from robyn.robyn import Request, Response
import argparse


app = Robyn(__file__)
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@app.exception
def handle_exception(error):
    return Response(status_code=500, description=f"error msg: {error}", headers={})

@app.get("/")
async def hi(request):
    return f"Hello, Sphinx!"

@app.post("/smartsip/call")
async def call_smartsip(request):
    type = request.query_params.get("output_type", "html")
    try:
        body = request.body
        file = bytearray(body)
        filename = 'temp.wav'
        with open(filename, 'wb') as f:
            f.write(file)
        text = audiototext(model, filename)
        return sphinx(text, type)
    except Exception as e:
        error_message = ''.join(traceback.format_exception(None, e, e.__traceback__))
        print("Error:", error_message)

def show_args(args):
    for key, value in vars(args).items():
        logger.info(f'{key}: {value}')

def main():
    parser = argparse.ArgumentParser(description='SmartSip Application')
    parser.add_argument('--host', type=str, help='The IP address of the application', default='0.0.0.0')
    parser.add_argument('--port', type=int, help='The port of the application', default=8888)
    parser.add_argument('--verbose', action='store_true', help='Enable verbose mode')
    
    args = parser.parse_args()
    show_args(args)
    

    if args.verbose:
        logger.info('Verbose mode is enabled')

    logger.info("SmartSip is ready!")
    app.start(host=args.host, port=args.port)

if __name__ == "__main__":
    main()