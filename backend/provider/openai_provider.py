#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : openai_provider.py
"""

from openai import OpenAI

from backend.provider.base_llm import BaseLLM
from backend.configs.llm_config import LLMConfig, LLMType
from backend.provider.llm_provider import register_provider
from typing import Union,Optional

@register_provider(LLMType.OPENAI)
class OpenAIProvider(BaseLLM):
    _client = {}
    def __init__(self, config: LLMConfig):
        self._config = config
        self._client = OpenAI()
        
    def call_llm(self, images: Optional[Union[str, list[str]]] = None):
        messages, response_format = self._user_msg(self._config.user_msg, images)
        
        print(messages)
 
        completion = self._client.chat.completions.create(
            model=self._config.model,
            temperature=0,
            top_p=0.9,
            messages=messages,
            response_format=response_format
        )
        return completion.choices[0].message.content
        
        
    def _user_msg(self, msg, images: Optional[Union[str, list[str]]] = None):
        messages = [{
            "role": "user",
            "content": [
                {
                    "type": "text",
                    "text": msg
                }
            ],
        }]
        
        messages[0]["content"].append({
            "type": "image_url",
            "image_url": {
                "url": images,
            }
        })

        response_format={
            "type": "json_schema",
            "json_schema": {
                "name": "smartsip_action",
                "schema": {
                    "type": "object",
                    "properties": {
                        "volume": {
                            "description": "The amount of the water",
                            "type": "string"
                        },
                        "degree": {
                            "description": "The temperature of the water in degree centigure",
                            "type": "string"
                        },
                        "type": {
                            "description": "The type of the container",
                            "type": "string"
                        },
                        "additionalProperties": False
                    }
                }
            }
        }
        
        return messages, response_format
