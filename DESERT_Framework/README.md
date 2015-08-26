```
  _____                      _     _    _           _                         _
 |  __ \                    | |   | |  | |         | |                       | |
 | |  | | ___  ___  ___ _ __| |_  | |  | |_ __   __| | ___ _ ____      ____ _| |_ ___ _ __
 | |  | |/ _ \/ __|/ _ \ '__| __| | |  | | '_ \ / _` |/ _ \ '__\ \ /\ / / _` | __/ _ \ '__|
 | |__| |  __/\__ \  __/ |  | |_  | |__| | | | | (_| |  __/ |   \ V  V / (_| | ||  __/ |
 |_____/ \___||___/\___|_|   \__|  \____/|_| |_|\__,_|\___|_|    \_/\_/ \__,_|\__\___|_|
______Copyright (c) 2015 Regents of the SIGNET lab, University of Padova  (ver. 2.0)_______
```

README
------

The following README explains how to use the Desert Underwater installation tools.
More details can be found in the Desert Underwater doxygen documentation at
http://telecom.dei.unipd.it/ns/desert/DESERT_HTML_doxygen_doc/


__AUTHORS__

SIGNET lab, University of Padova


__CONTACT__

If you have problems, questions, ideas or sugestions, please contact us at
desert-usergroup@mail.dei.unipd.it


__INSTALL__

To install the whole framework you can choose two different approaches:

- wizard mode
- command-line mode

The wizard mode implements an interactive installation procedure, and is easier than
the command-line mode. However, after completing an installation with the wizard,
you will be also provided a command-line string. You can enter this string at the
prompt to reproduce exactly the same installation (with the same options, packages
installed, etc.)

To launch the installation in wizard mode just run the following command:
```
./install.sh --wizard
```
The procedure will guide you through 6 steps:

- setting of the installation-target
- setting of the installation mode
- setting of the destination folder
- choosing whether or not to install the Woss libraries
- setting of possible custom parameters (OPTIONAL)
- choosing which addons to install (OPTIONAL)

To employ the command-line mode, you need to manually pass all installation settings
through the corresponding installation options. This mode makes it possible to embed
the DESERT Underwater installation tool into any other custom script.
For example, if you need to integrate the installation procedure into some more
general "installation project", with the "command-line" mode you can do this.
Here is a list of all options currently available:
```shell
    --help
        Print the help

    --wizard
        Start the installation procedure in wizard mode

    --with-woss
        Only for command-line. Enable the installation of the woss libraries

    --without-woss
        Only for command-line. Explicitly disable the installation of the woss libraries

    --target <installation-target-name>
        Only for command-line. This options sets the compilation target.
        From the point of view of the installation tool, passing the target option
        is equivalent to choosing the specific installation script to be used after
        the preliminary settings. The installation scripts are located in
        .../DESERT_Underwater/DESERT_Framework/Installation/.
        If you create a custom installation script, you must put it in this folder.
        For more information please refer to the doxygen documentation.
        Targets available:
            - PC
            - Gumstix-yocto-poky
            - Raspberry-Pi-yocto-poky

    --inst_mode <installation-mode>
        Only for command-line. This is option sets whether to install in release mode or
        in development mode. In release mode, after the compilation/cross-compilation
        has been completed, in destination folder you find only the files strictly
        required to run the DESERT libraries (namely, the bin/ and lib/ folders).
        Conversely, the development mode keeps all source files and build products.
        The source folders are part of an active working copy, which you can already use
        to develop modules, re-compile code without restarting the installation process
        from scratch, and commit updates to the source code.
        Installation-modes available:
           - development
           - release

    --dest_folder <destination-folder-path>
        Only for command-line. This option sets where the installation procedure will
        put all files generated during the compilation process.

    --addons <list-of-the-DESERT-addons>
        Only for command-line. This option specifies which addons to Desert Underwater
        should be installed. The available addons are usually found in
        .../DESERT_Underwater/DESERT_Addons/
        The list-of-the-DESERT-addons parameter is composed by the names of any addons
        separated by a white space and enclosed in quotes, e.g.,
        "ADDON1 ADDON2 ADDON3"

    --custom_par <list-of-parameters>
        Only for command-line. This option provides a useful means of passing a variable
        list of parameters to a custom installation script. The list-of-parameters is
        composed by all parameters, separated by a white space and enclosed in quotes,
        e.g.,
        "param1 param2 param3"
```
If you should encounter any problem while building this library, we set up a 
mailing list that you can refer to. Please point to:
    https://mail.dei.unipd.it/mailman/listinfo/desert-usergroup.

__WHAT TO DO AFTER THE INSTALLATION HAS BEEN COMPLETED__

When the installation of DESERT Underwater has been successfully completed, you need
to update your environment variables by exporting the paths to the bin/ and lib/
folders. This is necessary in order to correctly use DESERT and its dependencies (ns,
ns-miracle, etc.). This operation is made simpler via the environment file generated
at the end of the installation session in the ``<destination-folder-path>`` only for the
"development" installation-mode. So, by running the following command:
```
source <destination-folder-path>/environment
```
you will be able to call ns and all compiled libraries directly in the shell where
you ran the above command. You may want to check the environment script to see what
it exactly does.

With the "release" installation-mode, instead, you can set the environment via the
following command:
```
cd <destination-folder-path> && ./make_environment.sh
```
before updating your environment variables. The "make_environment.sh" script is
particularly useful after the DESERT framework has been cross-compiled and copied
into the target device: in this case, the script dynamically generates the
environment file in agreement with the destination folder path within the target
device.

__HOW TO ADD A CUSTOM INSTALLER__

First of all, enter the installation folder:
```
cd .../DESERT_Underwater/DESERT_Framework/Installation/
```
then create a new script. We suggest to copy an exiting installation script
(e.g. installDESERT_PC.sh):
```
cp installDESERT_PC.sh installDESERT_<your-custom-installation-target>.sh
```
The ``<your-custom-installation-target>`` label will be the same parameter you will write
later when indicating to the installation script which installation-target to choose.
These steps are very easy but also very important, since the only way to call the
correct installation script for your-custom-installation-target is that the script
``installDESERT_<your-custom-installation-target>.sh`` exists in the folder
``.../DESERT_Underwater/DESERT_Framework/Installation/``

Inside the script, in the first instance, we focus on the main block:
```
        main() {
            #******************************************************************************
            # MAIN
            #     e.g handle_package host/target <pkt-name>
            #     e.g addon_installation_list host/target <addon-list>

            ## only for the cross-compilation session
            export CROSS_ENV_DIR=""
            export CROSS_ENV_FILE=""
            #*

            handle_package host ZLIB
            handle_package host TCL
            export PATH=${BUILD_HOST}/bin:$PATH
            export LD_LIBRARY_PATH=${BUILD_HOST}/lib
            handle_package host OTCL
            handle_package host TCLCL
            handle_package host NS
            handle_package host NSMIRACLE
            handle_package host DESERT
            if [ ${WITHWOSS} -eq 1 ]; then
                handle_package host NETCDF
                handle_package host NETCDFCXX
                handle_package host BELLHOP
                handle_package host WOSS
            fi
            if [ ! -z "${ADDONS}" ]; then
                addon_installation_list host "${ADDONS}"
            fi
            #******************************************************************************
        }
```
Notice the "host" directive. This tells the installation script to compile the
libraries for the current host. If you need to modify this script to cross-compile
for a different target, substitute all lines with the following ones:
```
        main() {
            #******************************************************************************
            # MAIN
            #     e.g handle_package host/target <pkt-name>
            #     e.g addon_installation_list host/target <addon-list>

            ## only for the cross-compilation session
            export CROSS_ENV_DIR="<YOUR/ENVIRONMENT/FOLDER/PATH>"
            export CROSS_ENV_FILE="${CROSS_ENV_DIR}/environment"
            #*

            handle_package host/target ZLIB
            handle_package host/target TCL
            export PATH="${BUILD_HOST}/bin:$PATH"
            export LD_LIBRARY_PATH="${BUILD_HOST}/lib"
            handle_package host/target OTCL
            handle_package host/target TCLCL
            handle_package target NS
            handle_package target NSMIRACLE
            handle_package target DESERT
            if [ ${WITHWOSS} -eq 1 ]; then
                #handle_package target NETCDF
                #handle_package target NETCDFCXX
                #handle_package target BELLHOP
                #handle_package target WOSS
                warn_L1 "The WOSS libreries wont be installed!"
            fi
            if [ ! -z "${ADDONS}" ]; then
                addon_installation_list target "${ADDONS}"
            fi
            #******************************************************************************
        }
```
With these changes we instructed the installer to compile some modules only for the
host ("host" directive), some for both the host and the target ("host/target"
directive) and the remaining ones only for the target ("target" directive).
This step is needed as some libraries must be compiled for the host, in order to
correctly cross-compile DESERT and its addons for your-custom-installation-target.

__WARNING__: *be careful NOT TO CONFUSE the directive "target" with
             "your-custom-installation-target". The former is just a directive to the
             handle_package function of the installation script.*

We remark that you need the TOOLCHAIN of your-custom-installation-target in order to
correctly complete the installation.


COPYING / LICENSE
-----------------

__Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.__

All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the University of Padova (SIGNET lab) nor the names
   of its contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

**THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.**
