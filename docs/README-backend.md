# Brief

The backend contains the OSS and the Service API.

* Why need a backend?
  * Because the SmartSip application needs to give a prompt and a picture to get the End2End control command from the LLM, so it is possible to have a backend to adapt all kinds of LLM provider call, rather than update the code from the device.
  * If you are in China, certain LLM API cannot be requested directly, such as open-ai. So a better way is to use a server which could connect certain service by using VPN.
  * OSS could be used to save the data for further use and gives an image url. The openai api only accept url of the image recently, rather than base64 encode like other llm api.
 
# How to use?

## Install

* install the requirements
``` bash
pip install -r requirements.txt
```

* The environment variable `MINIO_ACCESS_KEY`, `MINIO_SECRET_KEY`, `MINIO_ENDPOINT_URL`(for OSS, S3); `OPENAI_API_KEY` (optional if use openai api); `OLLAMA_API_URL`(optional if use ollama api) needs to set.
* Set the python path `export PYTHONPATH=./`.
* `python backend/service/app.py` to execute

``` bash
Output:

key: LLMType.OPENAI
key: LLMType.OLLAMA
INFO:robyn.logger:SERVER IS RUNNING IN VERBOSE/DEBUG MODE. Set --log-level to WARN to run in production mode.
INFO:robyn.logger:Added route HttpMethod.GET /
INFO:robyn.logger:Added route HttpMethod.POST /smartsip/upload
INFO:__main__:host: 0.0.0.0
INFO:__main__:port: 8888
INFO:__main__:verbose: False
INFO:__main__:SmartSip is ready!
INFO:robyn.logger:Added route HttpMethod.GET /openapi.json
INFO:robyn.logger:Added route HttpMethod.GET /docs
INFO:robyn.logger:Docs hosted at http://0.0.0.0:8888/docs
INFO:robyn.logger:Robyn version: 0.62.1
INFO:robyn.logger:Starting server at http://0.0.0.0:8888
INFO:actix_server.builder:starting 1 workers
INFO:actix_server.server:Actix runtime found; starting in Actix runtime
```

## Request

* `http://localhost:8888/docs` to read the api documents
* `curl -X POST 'http://localhost:8888/smartsip/upload?llm_type=&image_name=&model='`  llm_type can be `openai, ollama`; the image_name is the image successfully upload to the OSS; model type could be `gpt-4o`, `gpt-4o-mini`, etc. for openai. `llava` for ollama. A json format response will be given.

![test](https://github.com/user-attachments/assets/41a34f62-0253-487a-96fa-d0249534c6d1)
```
Output:
{"volume":"200","degree":"80","type":"cup"}
```

# Component

* `Robyn` is a good backend framework, which could help us easily create a backend service with good performence. [https://github.com/sparckles/Robyn](https://github.com/sparckles/Robyn)
* `Minio` is a good open source OSS. [https://github.com/minio/minio](https://github.com/minio/minio)
* `Ollama` is a great llm inference framework to use many open source LLM. [https://github.com/ollama/ollama](https://github.com/ollama/ollama)
* The design pattern factory and decorator are used to crete the llm provider. which could extend other llm provider.

``` mermaid
classDiagram

LLMConfig

BaseLLM
BaseLLM <|-- OpenAIProvider
BaseLLM <|-- OllamaProvider
<<Interface>> BaseLLM
BaseLLM : LLMConfig config
BaseLLM : __init__()
BaseLLM : call_llm(images)

BaseLLM : _user_msg(prompt, images)
```



