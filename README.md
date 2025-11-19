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

Checkout our [website](http://desert-underwater.dei.unipd.it/)

## Basic structure

The basic structure of DESERT adheres to the 
**ISO/OSI communication protocol stack**: this allows for ease of development and integration.
The `DESERT_Framework` folder contains all core modules organized according 
to this stack along with the installer folder.
A set of packers, contained within the `DESERT_Addons` folders, allows DESERT to connect to real devices, so the same code can be used for both simulations and 
real sea experiments. 
`DESERT_Addons` also includes other external modules that do not necessarily 
fit within the ISO/OSI stack or the DESERT core.

### Documentation

A detailed doxygen documentation is embedded in the source code.
If you need to read it in html format please install doxygen on your system, 
and launch the following:
```
doxygen
```
further info and documentation can be found at our [github.io webpage](https://signetlabdei.github.io/DESERT_Underwater_doc/).

### Installation
The complete installation procedure can be found at [INSTALL.md](./DESERT_Framework/Installer/INSTALL.md).


### Samples

You can find a lot of samples at
```
DESERT_Framework/DESERT/samples
```
along with some guides on how to use them.

## Contributing to the project

Contributions from the community are welcome and appreciated. 
To maintain consistency and provide ease of integration and readability, the project has a set of rules for 
coding and submitting changes. Please refer to [CONTRIBUTING.md](CONTRIBUTING.md) if you want to contribute.

### Code style

Please take a look at the [code style](coding_rules.md)

## Authors

SIGNET lab, University of Padova
