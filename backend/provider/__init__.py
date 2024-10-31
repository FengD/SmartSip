#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : __init__.py
"""

from backend.provider.openai_provider import OpenAIProvider
from backend.provider.ollama_provider import OllamaProvider

__all__ = [
    "OpenAIProvider",
    "OllamaProvider"
]
