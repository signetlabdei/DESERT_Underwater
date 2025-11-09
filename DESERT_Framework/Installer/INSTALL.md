# Installation Guidelines

## Installation Requirements

Before installing the **DESERT framework**, please check that your host device has the following packages installed:

- build-essential  
- autoconf  
- automake  
- libxmu-dev  
- libx11-dev  
- libxmu-dev  
- libxmu-headers  
- libxt-dev  
- libtool  
- gfortran  
- bison  
- flex  

### Debian

If you are using a Debian-based Linux distribution (e.g., Ubuntu, Linux Mint, etc...), you can type the following command in a terminal, in order to install all dependencies above:

```console
$ sudo apt-get install build-essential autoconf automake libxmu-dev libx11-dev libxmu-dev libxmu-headers libxt-dev libtool gfortran bison flex
```

### Arch Linux

If you are using an Arch Linux based distribution, you can type the following command in a terminal:

```console
$ sudo pacman -S base-devel make tk gcc-fortran bison flex autoconf automake libtool patch patchutils
```

### Fedora

Before installing the DESERT framework in Fedora, check that your host device has the following packages installed:

- C Development Tools and Libraries  
- gcc-gfortran  
- libXmu-devel  
- libX11-devel  
- patch  
- patchutils  

Note that main C development tools are installed by installing the package group **"C Development Tools and Libraries"**.  
To install all the software requirements you can run the following commands in a terminal:

```console
$ dnf groupinstall "C Development Tools and Libraries"
```

```console
$ dnf install gcc-gfortran libXmu-devel libX11-devel patch patchutils
```

## Download and install

The latest version of DESERT can be cloned from the github repository, as follows:

```console
$ git clone https://github.com/signetlabdei/DESERT_Underwater
```

Alternatively, you can download the source code of a specific version from: https://github.com/signetlabdei/DESERT_Underwater/releases

To install DESERT Underwater, you can choose between two different approaches:

- *wizard* mode
- *command-line* mode

The wizard mode implements an interactive installation procedure. As such, it is easier and more user-friendly than the command-line mode.
The command-line mode is recommended for more experienced users.
Note that, after completing an installation with the wizard, you will be also provided a command-line string. You can note down this string and enter it at the command prompt in order to reproduce exactly the same installation steps (with the same options, the same packages installed, etc.). When you become more comfortable with DESERT's installation options, you can also modify the automatically generated command-line string to reflect your desired configuration.

To launch the installation in wizard mode, once you have downloaded DESERT, you have to cd into the framework folder by using:

```console
$ cd DESERT_Underwater/DESERT_Framework
```

and run the installer:

```console
$ ./install.sh --wizard
```

The procedure will guide you through 6 steps:

- setting of the installation-target
- setting of the installation mode
- setting of the destination folder
- choose whether or not to install the WOSS libraries
- setting of possible custom parameters (OPTIONAL)
- selection of the addons to be installed (OPTIONAL)

To employ the command-line mode, you need to manually specify all installation settings through the corresponding command-line options. This mode makes it possible to embed the DESERT Underwater installation tool into other custom scripts. For example, if you need to integrate the installation procedure into some more general "installation project", with the "command-line" mode you will find the command-line mode useful.

Here is a list of all options currently available:

```console
--help
    Print the help and exit

--wizard
    Start the installation procedure in wizard mode

--with-woss
    Only for command-line. Enable the installation of the WOSS libraries

--without-woss
    Only for command-line. Explicitly disable the installation of the WOSS libraries

--target <installation-target-name>
    Only for command-line. This options sets the compilation target.
    From the point of view of the installation tool, passing the target option
    is equivalent to choosing the specific installation script to be used after
    the preliminary settings. The installation scripts are located in
    DESERT_Underwater/DESERT_Framework/Installation/.
    If you create a custom installation script, you must put it in this folder.
    Targets available:
        - LOCAL
        - Raspberry Pi zero
        - EvoLogics-sandbox-debian_armhf

--inst_mode <installation-mode>
    Only for command-line. This option sets whether to install in release mode or
    in development mode. In release mode, after the compilation/cross-compilation
    has been completed, the destination folder will contain only the files strictly
    required to run the DESERT libraries (namely, the bin/ and lib/ folders).
    Conversely, the development mode keeps all source files and build sub-products.
    Note that the source folders are part of an active working copy, which you can
    directly use to develop modules, re-compile code without restarting the
    installation process from scratch, and commit updates to the source code.
    Installation-modes available:
       - development
       - release

--dest_folder <destination-folder-path>
    Only for command-line. This option sets where the installation procedure will
    put all files generated during the compilation process.

--addons <list-of-the-DESERT-addons>
    Only for command-line. This option specifies which addons to Desert Underwater
    should be installed. The available addons are usually found in
    DESERT_Underwater/DESERT_Addons/
    The <list-of-the-DESERT-addons> parameter is composed by the names of any addons
    separated by a white space e.g.,
    ADDON1 ADDON2 ADDON3
    Or if you want to install all of them you can just type
    ALL

--custom_par <list-of-parameters>
    Only for command-line. This option provides a useful means of passing a variable
    list of parameters to a custom installation script. The list-of-parameters is
    composed by all parameters, separated by a white space and enclosed in quotes,
    e.g.,
    "param1 param2 param3"
```

For instance, the following command will install DESERT on your local machine in development mode with all the addons:

```console
$ ./install.sh --target LOCAL --inst_mode development --dest_folder <destination-folder-path> --addons ALL
```

If you encounter any problem while building this library, you can refer to github and open a dedicated issue, that we will investigate.


### After the installation has been completed

When the installation of DESERT Underwater has been successfully completed, you need to update your environment variables by exporting the paths to the bin/ and lib/ folders created in the path specified during the installation. This is required in order to correctly use DESERT and its dependencies (ns, ns-miracle, etc.).
This operation is made simpler via the *environment* file generated at the end of the installation session in the <destination-folder-path> only for the "development" installation-mode. So, by running the following command:

```console
$ source <destination-folder-path>/environment
```

you will be able to call *ns* and all compiled libraries directly in the shell where you ran the above command. You may want to check the *environment* script to see what it exactly does.

With the "release" installation-mode, instead, you can set the environment via the following command:

```console
$ cd <destination-folder-path> && ./make_environment.sh
```

Before updating your environment variables. The "make_environment.sh" script is particularly useful after the DESERT framework has been cross-compiled and copied into the target device: in this case, the script dynamically generates the environment file in agreement with the destination folder path within the target device.

In order to make the environment update persistent even after you close the terminal, you need to append the source command to your bash configuration file, as follows:

```console
$ echo "source <destination-folder-path>/environment" >> $HOME/.bashrc
```

## Use Docker
Alternatively to the manual installation, DESERT provide an automatic installation using docker.

You will need to cd to the dockerfile directory

```console
$ cd DESERT_Underwater/DESERT_Framework/Docker/ubuntu_dockerfile_with_desert/
```

And build it

```console
$ docker build -t docker_desert .
```

It will create a container with Ubuntu 22.04 LTS, download all the dependencies and install DESERT with WOSS and all the addons.
Finally run the just created container

```console
$ docker run -it docker_desert
```
