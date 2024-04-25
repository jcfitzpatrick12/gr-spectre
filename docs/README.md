# gr-spectre: GNU Radio OOT module containing custom file sinks

## Overview
This OOT module contains custom file sinks used in the [`spectre`](https://github.com/jcfitzpatrick12/spectre) repository. 

📢 **This project is under active development. Detailed documentation will be provided in the future.**  📢 

## Blocks

### Batched File Sink 
Streams complex data continously into batched binary files of a user-determined size, into a directory derived from system time. 
  
e.g. [...]/[year]/[month]/[day]/[t0]_[tag].bin  where:
- [...] is a user defined parent directory.
- [t0] is the start time of the first sample of that batched chunk of data.
- [tag] is any string tag defined by the user.



