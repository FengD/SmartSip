#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : base_llm.py
"""

from abc import ABC, abstractmethod
from typing import Optional, Union

class BaseLLM(ABC):
    _config: LLMConfig

    @abstractmethod
    def __init__(self):
        pass
    
    @abstractmethod
    def call_llm(self):
        pass

    @abstractmethod
    def _user_msg(self, msg: str, images: Optional[Union[str, list[str]]] = None) -> dict[str, Union[str, dict]]:
        pass

    

