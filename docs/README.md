# gr-spectre: GNU Radio OOT module containing custom file sinks

## Overview
This OOT module contains custom file sinks used in the [`spectre`](https://github.com/jcfitzpatrick12/spectre) repository. 

📢 **This project is under active development. Detailed documentation will be provided in the future.**  📢 

## Blocks

- ```Batched File Sink```: writes complex data streams continuously into batched binary files of a user-determined size, saving appropriate metadata in a detached header.
- ```Sweep Driver```: periodically publishes messages instructing frequency re-tuning for compatible receiver blocks.
- ```Tagged Staircase```: source block used to test ```Batched File Sink``` and ```spectre``` frequency sweep modules.





