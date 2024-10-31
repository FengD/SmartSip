import requests
from backend.tools.s3_utils import get_image_s3_url
import base64
from PIL import Image
import io
import json

def encode_image_to_base64(image_url):
    response = requests.get(image_url)
    
    if response.status_code != 200:
        raise Exception(f"Failed to fetch image: {response.status_code}")

    # Open the image using Pillow
    img = Image.open(io.BytesIO(response.content))
    buffered = io.BytesIO()
    img.save(buffered, format="JPEG")
    img_bytes = buffered.getvalue()
    img_base64 = base64.b64encode(img_bytes).decode('utf-8')
    return img_base64

if __name__ == "__main__":

    url = "http://47.105.74.27:11434/api/generate"
    prompt = "Based on the image taken by the camera located below the water dispenser, identify whether the container in the current image is capable of receiving water. If the container is upside down, covered by other objects, placed outside the water outlet area, or if no container is placed, define it as not capable of receiving water, and set the water volume to 0. If the container is capable of receiving water, set a reasonable water volume. If the container already contains water or solid substances, ensure that the water volume does not exceed the container's capacity to avoid overflow. Based on the contents inside the cup, set an appropriate temperature and water volume. Pay attention to details, as the contents may include tea leaves such as black tea, green tea, white tea, each with its own suitable temperature, as well as other substances like milk powder, concentrated coffee, or instant noodles, all of which have different temperature settings. If any children's items or the presence of a child is detected in the image, ensure the water temperature does not exceed 40°C to prevent the risk of burns. Output the results in JSON format, including 'volume' in milliliters, 'degree' in Celsius, and 'type' indicating the container type in English. No explanation is needed, just output the JSON."

    image_url = get_image_s3_url("image_20241025_164659.jpg")
    print(image_url)

    base64_string = encode_image_to_base64(image_url)
    # print(base64_string)
    payload = {
        "model": "llava",
        "prompt": prompt,
        "stream": False,
        "format": "json",
        "images": [base64_string],
        "options": {
            "temperature": 0,
            "top_p": 0.9
        }
    }

    # Send the POST request
    response = requests.post(url, json=payload)

    # Print the response
    if response.status_code == 200:
        print(response.json()["response"])
    else:
        print("Error:", response.status_code, response.text)


    # {
    # "volume": 0,
    # "degree": null,
    # "type": "not capable of receiving water"
    # } 