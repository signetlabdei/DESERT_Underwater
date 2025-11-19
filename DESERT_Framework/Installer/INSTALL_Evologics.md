# How to cross-compile DESERT for EvoLogics ARM Debian Sandbox
## Cross-compilation on Debian/Ubuntu workstation
To download and setup the appropriate toolchain, you can use the following commands (*validated on Debian 11 and Ubuntu 22.04*)

```console
# dpkg --add-architecture armhf
# apt-get update
# apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

> **NOTE**: These commands requires sudo capabilities and workstation should be connected to the internet.

After script correctly finished, you can compile DESERT for EvoLogics sandbox selecting the appropriate arch and selecting *"release"* modality when asked during wizard procedure in order not to copy unnecessary files.

```console
$ ./install.sh --target EvoLogics-sandbox-debian_armhf --inst_mode release --dest_folder <your-desert-path>/DESERT_Underwater/DESERT_buildCopy_EvoLogics-sandbox-debian_armhf
```

After installation has been correctly completed, you can copy `_buildCopy` folder inside your EvoLogics modem, source the environment files and run you tcl files inside the EvoLogics modem's sandbox

```
$ scp -r <your-desert-path>/DESERT_Underwater/DESERT_buildCopy_EvoLogics-sandbox-debian_armhf root@<your-modem-ip-address>:~
```

you can now get access via ssh to your modem sandbox and source the DESERT env file in order to setup PATH and `LD_LIBRARY_PATH` variables

```console
$ ssh root@<your-modem-ip-address>
$ cd ~/DESERT_buildCopy_EvoLogics-sandbox-debian_armhf/ && ./make_environment.sh
$ source ~/DESERT_buildCopy_EvoLogics-sandbox-debian_armhf/environment
```

## Use docker

In order to simplify the cross-compilation process, a Dockerfile is present in `DESERT_Framework/Docker/xcompile` folder, which prepares for you the right environment to cross-compile DESERT for Evologics modem sandbox

> **NOTE**: The following section requires Docker to be installed and working on workstation

You have to first build your docker image

```console
$ docker build --network=host -t evologics/desert-cross-compile:latest -f Dockerfile-EvoLogics-sandbox .
```

You can now use the just built docker image to run a container and build DESERT inside it. To run that container you can use the provided script inside `DESERT_Framework/Docker/xcompile`

```console
$ ./docker-init.sh evologics/desert-cross-compile:latest
```

You will have a shell inside the running container with DESERT framework mounted on its filesystem, and you can now proceed to compile DESERT as in the previous section. At the end of the installation,
you can exit the docker container shell and copy the `_buildCopy` folder inside your EvoLogics modem, as explained in the previous section
