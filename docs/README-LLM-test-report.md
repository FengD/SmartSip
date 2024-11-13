# Introduction

``` mermaid
flowchart LR

A[Camera] --> |Picture| B[LLM]
C[Text] --> |Prompt| B[LLM]
D[Microphone-Optional] --> |Audio| B[LLM]
B --> |Json| E(Control Signal)
E --> |Serial Signal| F[Actuator]

```

* What we want is shown in the mindmap, make multi-modal input as a signal to control the actuator(pump, heater).
* To make the SmartSip work, a LLM with good perfermance is compulsory. So core part of the project is to `find an adapt LLM provider` with the correspondant `prompt`.

So this doc present the LLM provider that we test and its result.

# Test Report

* With the guidance of [https://artificialanalysis.ai/](https://artificialanalysis.ai/) we test 
* We use serval test case to test the basic use of mutimodal model of each llm provider

## Test image

<img width="600" alt="doc" src="https://github.com/user-attachments/assets/c9c471b4-3188-4170-b4f2-a1b2b5604c0d">

## Prompt

"Based on the image taken by the camera located below the water dispenser, identify whether the container in the current image is capable of receiving water. If the container is upside down, covered by other objects, placed outside the water outlet area, or if no container is placed, define it as not capable of receiving water, and set the water volume to 0. If the container is capable of receiving water, set a reasonable water volume. If the container already contains water or solid substances, ensure that the water volume does not exceed the container's capacity to avoid overflow. Based on the contents inside the cup, set an appropriate temperature and water volume. Pay attention to details, as the contents may include tea leaves such as black tea, green tea, white tea, each with its own suitable temperature, as well as other substances like milk powder, concentrated coffee, or instant noodles, all of which have different temperature settings. If any children's items or the presence of a child is detected in the image, ensure the water temperature does not exceed 40°C to prevent the risk of burns. Output the results in JSON format, including 'volume' in milliliters, 'degree' in Celsius, and 'type' indicating the container type in English. No explanation is needed, just output the JSON."

### Openai gpt-4o

All correct! By the way, `Chinese` also used as prompts, and there is no difference.

* [x] 1. {"volume": 300,"degree": 85,"type": "stainless steel thermos"}
* [x] 2. {"volume": 0,"degree": 0,"type": "covered or invalid container"}
* [x] 3. {"volume":"200","degree":"80","type":"cup"}
* [x] 4. {"volume": 0,"degree": 0,"type": "inverted bottle"}
* [x] 5. {"volume": 0,"degree": 0,"type": "upside-down thermos with lid"}
* [x] 6. {"volume": 0,"degree": 0,"type": "closed milk carton"}
* examples:
<img height="300" alt="WXWorkLocal_20241105100855" src="https://github.com/user-attachments/assets/e6c174a2-2960-40ae-bfa0-6379682d1f08">
<img height="300" alt="WXWorkLocal_20241105100836" src="https://github.com/user-attachments/assets/3c492ed7-dfd8-48b5-8851-5e3c794f38b0">

### Qianwen qiannwen2.5

4/6 correct rate. But seems not reasonable.

* [ ] 1. {"volume": 0,"degree": 25,"type": "stainless steel"}
* [x] 2. {"volume": 0,"degree": 25,"type": "plastic bottle"}
* [x] 3. {"volume":"200","degree":"80","type":"ceramic mug"}
* [ ] 4. {"volume": 1500,"degree": 25,"type": "plastic bottle"}
* [x] 5. {"volume": 0,"degree": 25,"type": "stainless steel"}
* [x] 6. {"volume": 0,"degree": 25,"type": "milk carton"}
* examples:
<img height="300" alt="WXWorkLocal_20241105101711" src="https://github.com/user-attachments/assets/65f8ab89-2cea-42df-bc7b-5afc193b59f6">
<img height="300" alt="WXWorkLocal_20241105101752" src="https://github.com/user-attachments/assets/282c4852-6c36-47d9-8793-8ded5d4076fa">

### Ollama llava

Looks like ollama llava is not working for this Scenario.

* [ ] 1. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}
* [x] 2. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}
* [ ] 3. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}
* [x] 4. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}
* [x] 5. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}
* [x] 6. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}

### Ollama llama3.2-vision

llama3.2-vision gives some mistakes, but better than llava. It could gives acurrate type but the volumn and degree is not correct.

* [x] 1. {"volume": 250, "degree": 95, "type": "cup"}
* [ ] 2. {"volume": 500,"degree": 95,"type": "tea pot"}
* [x] 3. {"volume": 250,"degree": 95,"type": "cup"}
* [ ] 4. {"volume": 500,"degree": 80,"type": "Plastic Bottle"}
* [ ] 5. {"volume": 250,"degree": 95,"type": "cup"}
* [ ] 6. {"volume": 250,"degree": 95,"type": "Milk Carton"}

<img width="586" alt="WXWorkLocal_20241113141450" src="https://github.com/user-attachments/assets/519f9cac-a061-41bf-a20d-cec7f839ae09">

### Ollama llama3.2-vision:90b

llama3.2-vision 90b gives more mistakes than 11b, and takes more than 6seconds to get the response on RTX4090.

* [ ] 1. {"volume": 0,"degree": 0,"type": "No container"}
* [x] 2. {"volume": 0,"degree": 0,"type": "No container"}
* [x] 3. {"volume": 250,"degree": 95,"type": "cup"}
* [x] 4. {"volume": 0,"degree": 0,"type": "No container"}
* [x] 5. {"volume": 0,"degree": 0,"type": "No container"}
* [x] 6. {"volume": 0,"degree": 0,"type": "not capable of receiving water"}

### Doubao

Bytedance Doubao is not working for this Scenario. And gives that it could not recognize the volume of the container.

* [ ] 1. {"volume": 0,"degree": 0,"type": "unknown"}
* [x] 2. {"volume": 0,"degree": 0,"type": "unknown"}
* [ ] 3. {"volume": 0,"degree": 0,"type": "unknown"}
* [x] 4. {"volume": 0,"degree": 0,"type": "unknown"}
* [x] 5. {"volume": 0,"degree": 0,"type": "unknown"}
* [x] 6. {"volume": 0,"degree": 0,"type": "unknown"}
* examples:
<img height="300" alt="WXWorkLocal_20241105101054" src="https://github.com/user-attachments/assets/fdd71447-22be-416b-9dc9-8be1e0efb02c">
<img height="300" alt="WXWorkLocal_20241105101015" src="https://github.com/user-attachments/assets/9cbfa711-38dd-495c-a775-370dbd6ddda7">



