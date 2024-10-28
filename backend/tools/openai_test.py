from openai import OpenAI

client = OpenAI()

completion = client.chat.completions.create(
    model="gpt-4o",
    # temperature=0,
    # top_p=0.9 ,
    messages=[
        {
            "role": "user",
            "content": [
                {"type": "text", "text": "Based on the image taken by the camera located below the water dispenser, identify whether the container in the current image is capable of receiving water. If the container is upside down, covered by other objects, placed outside the water outlet area, or if no container is placed, define it as not capable of receiving water, and set the water volume to 0. If the container is capable of receiving water, set a reasonable water volume. If the container already contains water or solid substances, ensure that the water volume does not exceed the container's capacity to avoid overflow. Based on the contents inside the cup, set an appropriate temperature and water volume. Pay attention to details, as the contents may include tea leaves such as black tea, green tea, white tea, each with its own suitable temperature, as well as other substances like milk powder, concentrated coffee, or instant noodles, all of which have different temperature settings. If any children's items or the presence of a child is detected in the image, ensure the water temperature does not exceed 40°C to prevent the risk of burns. Output the results in JSON format, including 'volume' in milliliters, 'degree' in Celsius, and 'type' indicating the container type in English. No explanation is needed, just output the JSON."},
                {
                    "type": "image_url",
                    "image_url": {
                        "url": "http://47.105.74.27:9000/test/image_20241025_164820.jpg?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=ihrzKxdmeXR275zTw8Ju%2F20241028%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20241028T100550Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=be283f995a6afa712302a4e5f2e4888deea7d28465cc89f5357453b800f1c44b",
                    }
                },
            ],
        }
    ],
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
)

print(completion.choices[0].message.content)