#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@Time     : 2024/10/28
@Author   : FengD
@File     : ollama_provider.py
"""

import base64
from typing import Union, Optional
import requests
import os
from backend.configs.llm_config import LLMType,LLMConfig
from backend.provider.base_llm import BaseLLM
from backend.provider.llm_provider import register_provider
from backend.tools.ollama_utils import encode_image_to_base64


@register_provider(LLMType.OLLAMA)
class OllamaProvider(BaseLLM):

    def __init__(self, config: LLMConfig):
        self._config = config
        self._config.base_url = os.environ.get("OLLAMA_API_URL") # "http://<url>:<port>/api/generate"
    

    def call_llm(self, images = None):
        prompt, base64_string = self._user_msg(self._config.user_msg, images)
        payload = {
            "model": self._config.model,
            "prompt": prompt,
            "stream": False,
            "format": "json",
            "images": [base64_string],
            "options": {
                "temperature": 0,
                "top_p": 0.9
            }
        }
        
        response = requests.post(self._config.base_url, json=payload)

        # Print the response
        if response.status_code == 200:
            return response.json()["response"]
        else:
            return None



    def _user_msg(self, msg: str, images: Optional[Union[str, list[str]]] = None) -> dict[str, Union[str, dict]]:
        prompt = msg
        base64_string = encode_image_to_base64(images)
        return prompt, base64_string
