# gr-spectre: The primary GNU Radio OOT module for SPECTRE

## Overview
This OOT module contains custom GNU Radio blocks used in the [`spectre`](https://github.com/jcfitzpatrick12/spectre) repository. 

📢 **This project is under active development. Detailed documentation will be provided in the future.**  📢 

## Blocks

- ```Batched File Sink```: writes complex data streams continuously into batched binary files of a user-determined size, saving appropriate metadata in detached headers.
- ```Sweep Driver```: periodically publishes messages instructing frequency re-tuning for compatible receiver blocks.
- ```Tagged Staircase```: source block used to test ```Batched File Sink``` and ```spectre``` frequency sweep modules.





