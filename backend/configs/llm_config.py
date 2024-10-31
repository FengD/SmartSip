#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@Time     : 2024/10/28
@Author   : FengD
@File     : llm_config.py
@brief    : used to define the useful parameters of a LLM Config
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
    
    user_msg: str = "Based on the image taken by the camera located below the water dispenser, identify whether the container in the current image is capable of receiving water. If the container is upside down, covered by other objects, placed outside the water outlet area, or if no container is placed, define it as not capable of receiving water, and set the water volume to 0. If the container is capable of receiving water, set a reasonable water volume. If the container already contains water or solid substances, ensure that the water volume does not exceed the container's capacity to avoid overflow. Based on the contents inside the cup, set an appropriate temperature and water volume. Pay attention to details, as the contents may include tea leaves such as black tea, green tea, white tea, each with its own suitable temperature, as well as other substances like milk powder, concentrated coffee, or instant noodles, all of which have different temperature settings. If any children's items or the presence of a child is detected in the image, ensure the water temperature does not exceed 40°C to prevent the risk of burns. Output the results in JSON format, including 'volume' in milliliters, 'degree' in Celsius, and 'type' indicating the container type in English. No explanation is needed, just output the JSON."
    
