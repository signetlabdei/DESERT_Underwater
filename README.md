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

### Documentation

See our [github.io webpage](https://signetlabdei.github.io/DESERT_Underwater_doc/)

### Website

Checkout our [website](http://desert-underwater.dei.unipd.it/)

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

### Code style

Please take a look at the [code style](coding_rules.md)

### Contributing to the project

Contributions from the community are welcome and appreciated. 
To maintain consinstency and provide ease of integration and readability, the project has a set of rules for 
coding and submitting changes. Please refer to [CONTRIBUTING.md](CONTRIBUTING.md) 
if you want to contribute.
Before committing your changes, please give a read at this [set of rules](commit_rules.md)
