#!/usr/bin/env python
# -*- coding: utf-8 -*-

```
@Time     : 2024/10/28
@Author   : FengD
@File     : base_llm.py

```

from abc import ABC, abstractmethod
from typing import Optional, Union

class BaseLLM(ABC):
    ```
        LLM API abstract class

    ```


    config: LLMConfig

    @abstractmethod
    def __init__(self, config: LLMConfig):
        pass


    def _user_msg(self, msg: str, images: Optional[Union[str, list[str]]] = None) -> dict[str, Union[str, dict]]:
        if images:
            # as gpt-4v, chat with image
            return self._user_msg_with_imgs(msg, images)
        else:
            return {"role": "user", "content": msg}

    def _user_msg_with_imgs(self, msg: str, images: Optional[Union[str, list[str]]]):
        """
        images: can be list of http(s) url or base64
        """
        if isinstance(images, str):
            images = [images]
        content = [{"type": "text", "text": msg}]
        for image in images:
            # image url or image base64
            url = image if image.startswith("http") else f"data:image/jpeg;base64,{image}"
            # it can with multiple-image inputs
            content.append({"type": "image_url", "image_url": {"url": url}})
        return {"role": "user", "content": content}

