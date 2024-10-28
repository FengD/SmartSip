#Introduction

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

#Test Report
