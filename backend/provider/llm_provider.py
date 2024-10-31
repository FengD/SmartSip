#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : llm_provider.py
@Brief    : registry and getter of the llm provider
"""

from backend.configs.llm_config import LLMConfig, LLMType
from backend.provider.base_llm import BaseLLM

class LLMProvider:
    def __init__(self):
        self.providers = {}

    def register(self, key, provider_cls):
        self.providers[key] = provider_cls

    def get_provider(self, enum: LLMType):
        """get provider instance according to the enum"""
        return self.providers[enum]


def register_provider(key):
    """register provider to registry"""
    def decorator(cls):
        if key:
            LLM_REGISTER.register(key, cls)
        return cls
    print("key:", key)
    return decorator


def create_llm_instance(config: LLMConfig) -> BaseLLM:
    """get the default llm provider"""
    llm = LLM_REGISTER.get_provider(config.api_type)(config)
    return llm

# Registry instance
LLM_REGISTER = LLMProvider()