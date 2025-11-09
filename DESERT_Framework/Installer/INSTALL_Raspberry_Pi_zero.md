# How to cross-compile DESERT for Raspberry Pi Zero (32bit)
## Cross-compilation on Debian/Ubuntu workstation
To download the appropriate toolchain, you can use the script provided inside

```console
$ <your-desert-path>/DESERT_Underwater/DESERT_Framework/Installer/make-rpi-toolchain.sh
```

> **NOTE**: this script requires sudo capabilities and workstation should be connected to the internet.

After script correctly finished, you can compile DESERT for Raspberry Pi Zero selecting the appropriate arch and selecting "release" mode when asked during wizard procedure in order not to copy unnecessary files.

Alternatively to the wizard, you can run the following command:


```console
$ ./install.sh --target Raspberry-Pi-zero-Raspbian --inst_mode release --dest_folder <your-desert-path>/DESERT_Underwater/DESERT_buildCopy_Raspberry-Pi-zero-Raspbian
```

After installation has been correctly completed, you can copy `_buildCopy` folder inside your Raspberry, set the environment files and run you tcl files inside the Raspberry.

```console
$ scp -r <your-desert-path>/DESERT_Underwater/DESERT_buildCopy_Raspberry-Pi-zero-Raspbian pi@<your-raspi-ip-address>:~
```

you can now get access via ssh to your Raspberry and source the DESERT env file in order to setup `PATH` and `LD_LIBRARY_PATH` variables

```console
$ ssh <user>@<your-modem-ip-address>
$ cd ~/DESERT_buildCopy_Raspberry-Pi-zero-Raspbian/ && ./make_environment.sh
$ source environment
```
  
---

## Use docker

In order to simplify the cross-compilation process, a Dockerfile is present in `DESERT_Framework/Docker/xcompile` folder, which prepares for you the right environment to cross-compile DESERT on Raspberry

> **NOTE**: The following section requires Docker to be installed and working on workstation.

You have to first build your docker image

```console
$ docker build --network=host -t desert/raspi-zero-cross:latest -f Dockerfile-Raspberry-Pi-Zero .
```

You can now use the just build the docker image to run a container and compile DESERT inside it. To run that container you can use the provided script inside `DESERT_Framework/Docker/xcompile`


```console
$ ./docker-init.sh desert/raspi-zero-cross:latest
```

You will have a shell inside the running container with DESERT framework mounted on its filesystem, and you can now proceed to compile DESERT as in the previous section. At the end of the installation,
you can exit the docker container shell (container will now stop) and copy the *_buildCopy* folder inside your Raspberry, as explained in the previous section.
