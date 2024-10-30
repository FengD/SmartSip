#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : llm_config.py
"""

from enum import Enum
from typing import Optional

class LLMType(Enum): 
    OPENAI = "openai"
    OLLAMA = "ollama"

    def __missing__(self, key):
        raise ValueError(f"{key} is not a valid llm type")

class LLMConfig():
    api_key: str = "secrat key"
    api_type: LLMType = LLMType.OPENAI
    base_url: str = ""
    api_version: Optional[str] = None

    model: Optional[str] = None
    response_format: Optional[object] = None
