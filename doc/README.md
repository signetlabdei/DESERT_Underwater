# The DESERT Underwater Libraries

**Project supervisor:**  
Filippo Campagnaro - [filippo.campagnaro@unipd.it](mailto:filippo.campagnaro@unipd.it)

**Maintainers:**  
Federico Favaro - [federico.favaro@dei.unipd.it](mailto:federico.favaro@dei.unipd.it),  
Federico Guerra - [federico@guerra-tlc.com](mailto:federico@guerra-tlc.com),  
Roberto Francescon - [francescon.roberto@gmail.com](mailto:francescon.roberto@gmail.com),  
Antonio Montanari - [montanar@dei.unipd.it](mailto:montanar@dei.unipd.it),  
Vincenzo Cimino - [vincenzo.cimino@unipd.it](mailto:vincenzo.cimino@unipd.it),  
Filippo Donegà - [filippo.donega@unipd.it](mailto:filippo.donega@unipd.it)

**Contributors:**  
[Sara Falleni](https://www1.coe.neu.edu/~sarafalleni/) - [falleni.s@northeastern.edu](mailto:falleni.s@northeastern.edu),  
Riccardo Tumiati - [riccardo.tumiati@studenti.unipd.it](mailto:riccardo.tumiati@studenti.unipd.it)

**Former contributors and members:**  
[Giovanni Toso](./Giovanni_Toso.md)

---

## Introductory Notes

### Brief History

**DESERT Underwater** (or briefly **DESERT**) [1] is a framework to **DEsign, SimulatE and Realize Testbeds** for Underwater network protocols.  
The support DESERT offers to these activities comes in two flavors: support to simulations and support to real-life experimentation.

With DESERT, we distribute several network protocols for underwater networking, some of which are the result of our own development, whereas some represent our own understanding of protocols found in the literature.  
Among the components of DESERT, some libraries and tools help speed up the transition from simulations to sea trials. The latter is achieved by integrating the commands required to communicate with real modems into specific interface modules. In turn, this allows the user to run protocols in real life by re-using the same protocol code already written for simulations.

DESERT is based on ns2 [2] and NS-MIRACLE [3]. In particular, DESERT follows the modular approach of the latter. Other than network protocols, DESERT provides additional libraries and tools to help simulate underwater networks realistically. These tools include mobility models that reproduce realistic mobility patterns, energy consumption models, and channel models.  
The most advanced model available is provided via a close interaction between DESERT and WOSS [4].

DESERT can be cross-compiled for Raspberry Pi (e.g., Zero 2 W, 4, 5) and the latest EvoLogics modem's sandbox. DESERT supports real data transmission via EvoLogics [5], ahoi modems [6], SuM modems [7], and all devices that provide an Ethernet transparent interface (such as Ethernet switches and many commercial optical modems).

Altogether, the many proof-of-concept implementations and complex tests carried out with DESERT so far make DESERT an effective solution for realizing underwater networking experiments by reusing the same protocol code already written for simulations.

DESERT Underwater has been adopted in several real-life [sea trials](http://desert-underwater.dei.unipd.it/index.php/field-experiments/).  
You can find more on the [official website](http://desert-underwater.dei.unipd.it) of the project.


### DESERT Release

A considerable amount of work has been done to make the installation of the software automatic via a modular installation script that includes a fully-guided wizard mode.  
At the same time, the installation is organized into separate modules. Experienced users can easily extend the process to accommodate specific requirements, new functionalities, or even make DESERT part of larger projects.

To help users wishing to experiment with real hardware using DESERT, we also provide different installer modules, suitable for compact/embedded hardware platforms, starting from [Raspberry Pi](https://www.raspberrypi.org/) and the [EvoLogics](https://www.evologics.com/) modem sandbox.  
This set will be extended in the future.

To use DESERT more effectively in real environments, **uwApplication** is provided. This module permits transmission of real payloads among nodes with tunable dimensions. Moreover, real data can be transmitted to uwApplication through a TCP or UDP socket at a given port.  
Using this feature, you can implement a simple "underwater chat" or connect a sensor/device that transmits data to uwApplication through a socket.  
uwApplication encapsulates the payload with control headers and delivers packets to the lower layers.

Finally, the functionalities offered by DESERT have been extended with a **control framework** that provides a set of primitives to remotely control hardware modems and network operations.

All these features are released constantly on our [GitHub repository](https://github.com/signetlabdei/DESERT_Underwater).

---

## References

1. R. Masiero, P. Casari, M. Zorzi, *“The NAUTILUS project: Physical Parameters, Architectures and Network Scenarios,”* Proc. MTS/IEEE OCEANS, Kona, HI, 2011.  
2. [The Network Simulator - ns2](https://www.isi.edu/websites/nsnam/ns/)  
3. N. Baldo, M. Miozzo, F. Guerra, M. Rossi, and M. Zorzi, *“Miracle: the multi-interface cross-layer extension of ns2,”* EURASIP J. Wireless Commun. and Networking, 2010. [Online](https://dl.acm.org/doi/10.1155/2010/761792)  
4. [WOSS - World Ocean Simulation System](https://woss.dei.unipd.it)  
5. [EvoLogics GmbH](https://www.evologics.de/)  
6. [Ahoi modem](http://bcrenner.de/?ahoi-modem)  
7. [SubSeaPulse SRL](https://www.subseapulse.com/)  
8. [Woods Hole Oceanographic Institution](https://www.whoi.edu/)  
9. [The Goby Underwater Autonomy Project](https://gobysoft.com/)  
10. [Develogic Subsea Systems](https://www.develogic.de/)  
11. W. Liang, H. Yu, L. Liu, B. Li and C. Che, *“Information-Carrying Based Routing Protocol for Underwater Acoustic Sensor Network,”* Proc. ICMA, Takamatsu, Japan, Aug. 2007.  
12. N. Abramson, *“Development of the ALOHANET,”* IEEE Trans. on Information Theory, vol. 31, no. 2, pp. 119–123, 1985.  
13. X. Guo, M. R. Frater and M. J. Ryan, *“A propagation-delay-tolerant collision avoidance protocol for underwater acoustic sensor networks,”* Proc. OES/IEEE OCEANS, Singapore, May 2006.  
14. B. Peleato and M. Stojanovic, *“Distance aware collision avoidance protocol for ad-hoc underwater acoustic sensor networks,”* IEEE Communications Letters, vol. 11, no. 12, pp. 1025–1027, 2007.  
15. F. Favaro, P. Casari, F. Guerra, M. Zorzi, *“Data Upload from a Static Underwater Network to an AUV: Polling or Random Access?”* Proc. MTS/IEEE OCEANS, Yeosu, South Korea, May 2012.  
16. A. Syed, W. Ye and J. Heidemann, *“T-Lohi: A new class of MAC protocols for underwater acoustic sensor networks,”* IEEE INFOCOM, Phoenix, AZ, Apr. 2008.  
17. S. Azad, P. Casari, M. Zorzi, *“The Underwater Selective Repeat Error Control Protocol for Multiuser Acoustic Networks: Design and Parameter Optimization,”* IEEE Trans. Wireless Communications, vol. 12, no. 10, pp. 4866–4877, Oct. 2013.  
18. F. Favaro, L. Brolo, G. Toso, P. Casari, M. Zorzi, *“A Study on Data Retrieval Strategies in Underwater Acoustic Networks,”* Proc. MTS/IEEE OCEANS, San Diego, CA, Sep. 2013.  
19. C. Tapparello, P. Casari, G. Toso, I. Calabrese, R. Otnes, P. van Walree, M. Goetz, I. Nissen, M. Zorzi, *“Performance Evaluation of Forwarding Protocols for the RACUN Network,”* Proc. ACM WUWNet, Kaohsiung, Taiwan, Nov. 2013.  
20. B. Liang and Z. Haas, *“Predictive distance-based mobility management for PCS networks,”* Proc. IEEE INFOCOM, New York, NY, Mar. 1999.  
21. F. Campagnaro, R. Francescon, F. Guerra, F. Favaro, P. Casari, R. Diamant, M. Zorzi, *“The DESERT underwater framework v2: Improved capabilities and extension tools,”* Proc. IEEE UCOMMS, Lerici, Italy, Oct. 2016.  
22. R. Francescon, F. Favaro, F. Guerra, M. Zorzi, *“Software Defined Underwater Communications: an Experimental Platform for Research,”* OCEANS 2023 - Limerick, Ireland, 2023.
