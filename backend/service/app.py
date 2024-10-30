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

# curl -X GET http://localhost:8888
@app.get("/")
async def hi(request):
    return f"Hello, Sphinx!"


# call example: curl -X POST http://localhost:8888/smartsip/upload -H "Content-Type: application/octet-stream" --data-binary @image_20241025_164659.jpg
@app.post("/smartsip/upload")
async def upload(request):
    llm_type = request.query_params.get("llm_type", "openai")
    body = request.body
    file = bytearray(body)
    
    print(llm_type)

    with open('test.jpeg', 'wb') as f:
        f.write(file)

    return {'message': 'success'}

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