# Introduction

* Product Name: `SmartSip` - The Intelligent Water Dispenser

<p align="center">
<a href="README.md"><img src="https://img.shields.io/badge/document-English-blue.svg" alt="EN doc"></a>
<a href="docs/README_CN.md"><img src="https://img.shields.io/badge/文档-中文版-blue.svg" alt="CN doc"></a>
<a href="docs/README-backend.md"><img src="https://img.shields.io/badge/document-BackendDoc-blue.svg" alt="Backend doc"></a>
<a href="docs/README-esp32.md"><img src="https://img.shields.io/badge/document-DeviceDoc-blue.svg" alt="Device doc"></a>
<a href="docs/README-LLM-test-report.md"><img src="https://img.shields.io/badge/document-LLMProviderTestReport-blue.svg" alt="LLM doc"></a>
<a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License: MIT"></a>
</p>

* Product Description: SmartSip stands out from other water dispensers on the market with its minimalist design, featuring a single touchscreen as the only button. Equipped with a `camera` located below the dispenser's nozzle, SmartSip use `LLM Provider || LLM Edge on mobile` intelligently analyzes the cup validation, cup type, its contents, and other relevant factors. Based on this information, it autonomously adjusts the water temperature and volume, delivering an effortless user experience. With SmartSip, the interaction is reduced to a simple touch, while the device takes care of all the details, embodying the true essence of minimalist functionality in modern hydration technology. With the microphone on the device we could also use `voice` to control the SmartSip. The smartsip could also connect with mobile phone by wifi or use the only screen, to display the details such as water temperature and the amount of the water. By connecting the SmartSip to a mobile device, users can unlock additional features, such as utilizing the built-in camera for facial recognition. This allows for personalized settings and enhances safety. For instance, when the dispenser recognizes a child, it can automatically prevent dispensing overly hot water, reducing the risk of accidents and ensuring safer use. This personalization feature offers convenience while addressing important safety concerns, making SmartSip suitable for all users, including families with young children.

* We open all the documentation and source code of the project.

# Demo

(interaction diagram)

(video)

# Highlight Functionalities

1. `One touch` and all automatic. (v0.1)
2. `LLM support`. Analysis the type of the container and gives the volume and temperature of the water needed. (v0.1)
3. `Wireless control by mobile`.(v1.0)

# Project
* `docs`: user guide and technical reports [docs](https://github.com/FengD/SmartSip/tree/main/docs)
* `smartsip`: arduino project [README-esp32](https://github.com/FengD/SmartSip/blob/main/docs/README-esp32.md)
* `backend`: backend project [README-backend](https://github.com/FengD/SmartSip/blob/main/docs/README-backend.md)
* `Certain LLM Provider test report`: [README-LLM-test-report](https://github.com/FengD/SmartSip/blob/main/docs/README-LLM-test-report.md)
* `hardware`: [v0.1](), [v1.0]()

# Thanks

* `Chatgpt`: for the methology validation
* `SeeedStudio`: for the amazing device and the helpful documentation of the usage. [https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)

# Contributors

Sort by joining time

* [FengD](https://github.com/FengD)
* [gong-333](https://github.com/gong-333)
