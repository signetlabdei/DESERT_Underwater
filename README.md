DESERT Underwater
=================

**DESERT Underwater: an NS-Miracle extension to** 
**DEsign, Simulate, Emulate and Realize Test-beds**
**for Underwater network protocols**

DESERT Underwater is a complete set of public C++ libraries that extend
the NS-Miracle simulator to support the design and implementation of 
underwater network protocols. Its creation stems from the will to push 
the studies on underwater networking beyond simulations. 
Implementing research solutions on actual devices, in fact, is of key 
importance to realize a communication and networking architecture that 
allows heterogeneous nodes to communicate reliably in the underwater 
environment.



### Installation

To install DESERT Underwater follow the instructions at the [doxygen documentation](http://telecom.dei.unipd.it/ns/desert/DESERT3_HTML_doxygen_doc/)

### Samples

You can find a lot of samples at
```
DESERT_Underwater/DESERT/samples
```
along with some guides on how to use them.

### Basic structure

The basic structure of DESERT adheres to the 
**ISO/OSI communication protocol stack**: this allows for ease of development and integration. 
A set of **packers** allows DESERT to also be connected to real devices, thus using the same 
code in both simulation and a real sea experiment.

### Contributing to the project

Contributions from the community are welcome and appreciated. 
To maintain consinstency and provide ease of integration and readability, the project has a set of rules for 
coding and submitting changes. Please refer to [CONTRIBUTING.md](CONTRIBUTING.md) 
if you want to contribute.
