import logging
from robyn import Robyn, html
from robyn.robyn import Request, Response
import argparse
from backend.tools.s3_utils import get_image_s3_url
from backend.configs.llm_config import LLMType,LLMConfig
from backend.provider.llm_provider import create_llm_instance


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


# call example: curl -X POST 'http://localhost:8888/smartsip/upload?llm_type=&image_name=&model='
@app.post("/smartsip/upload")
async def upload(request):
    llm_type = request.query_params.get("llm_type", "openai")
    if LLMType(llm_type) not in LLMType:
        return Response(status_code=400, description=f"error msg: invalid llm type", headers={})
    model  = request.query_params.get("model", "gpt-4o")
    
    llm_config = LLMConfig()
    llm_config.model = model
    llm_config.api_type = LLMType(llm_type)
    
    llm_provider = create_llm_instance(llm_config)

    image_name = request.query_params.get("image_name", "")

    if image_name:
        image_url = get_image_s3_url(image_name)
    else:
        return Response(status_code=400, description=f"error msg: empty image name", headers={})

    if image_url is not None:
        response = llm_provider.call_llm(image_url)
    else:
        return Response(status_code=400, description=f"error msg: not valide image url", headers={})
    
    return Response(status_code=200, description=response, headers={})

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