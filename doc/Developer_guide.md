# Developer guide

## DESERT Structure
The DESERT Underwater Framework is an extension of [ns-miracle](https://signet.dei.unipd.it/dgt/ns/miracle/doxygen/index.html). It inherits the modular structure and allows the use of all the modules of ns-miracle.
Each DESERT module extends the `Module` class, which in turn extends the `PlugIn` class.
These classes provide basic APIs to (i) send packets to upper/lower layers, (ii) communicate via cross-layer messages with other layers of the stack, and (iii) configure the layer via Tcl script.

The folder structure is composed of two main folders: 
- **DESERT_Framework**: contains the DESERT folder, which groups all the core modules organized according to the ISO/OSI protocol stack and the installer folder. For brevity, we refer to the path `DESERT_Underwater/DESERT_Framework/DESERT` as `DES_CORE`.
- **DESERT_Addons**: contains external modules that do not necessarily fit within the ISO/OSI stack and/or the DESERT core.

## How to create a new tcl script
In this section we describe how to write a basic Tcl script, following the structure of the script found in `DES_CORE/samples/desert_samples/test_uwcbr.tcl`.

It is a good practice to start your script with a brief description and a simple draw of the simulated network topology:
```tcl
# ----------------------------------------------------------------------------------
# This script depicts a very simple but complete stack in which two nodes send data
# to a common sink. The second node is used by the first one as a relay to send data to the sink.
# The routes are configured by using UW/STATICROUTING.
# The application used to generate data is UW/CBR.
# ----------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------
# Stack
#             Node 1                         Node 2                        Sink
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  7. UW/CBR               |   |  7. UW/CBR               |   |  7. UW/CBR  | UW/CBR     |
#   +--------------------------+   +--------------------------+   +-------------+------------+
#   |  6. UW/UDP               |   |  6. UW/UDP               |   |  6. UW/UDP               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |   |  5. UW/STATICROUTING     |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  4. UW/IP                |   |  4. UW/IP                |   |  4. UW/IP                |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  3. UW/MLL               |   |  3. UW/MLL               |   |  3. UW/MLL               |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |   |  2. UW/CSMA_ALOHA        |
#   +--------------------------+   +--------------------------+   +--------------------------+
#   |  1. Module/MPhy/BPSK     |   |  1. Module/MPhy/BPSK     |   |  1. Module/MPhy/BPSK     |
#   +--------------------------+   +--------------------------+   +--------------------------+
#            |         |                    |         |                   |         |       
#   +----------------------------------------------------------------------------------------+
#   |                                     UnderwaterChannel                                  |
#   +----------------------------------------------------------------------------------------+
```

The first thing to do in order to have a working script, is to import the libraries. Note that the order matters and is often a cause of errors.
You first need to import the ns-miracle libraries in the following order:
```tcl
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
```
Afterwards you can import DESERT libraries you need (in order).
```tcl
load libuwcsmaaloha.so
load libuwip.so
load libuwstaticrouting.so
load libuwmll.so
load libuwudp.so
load libuwcbr.so
```

In order to configure your simulation, some options and variables may be needed.
A common approach is to use an associative array called. These options may be hardcoded or given to the simulation as bash parameters.
```tcl
set opt(bash_parameters) 0; # Set to 1 to activate bash parameters
if {$opt(bash_parameters)} {
	if {$argc != 2} {
		puts "The script requires two inputs"
	} else {
		set opt(pktsize)	[lindex $argv 0]
		set opt(rngstream)	[lindex $argv 1]
	} else {
		set opt(pktsize)	125;	# Bytes
		set opt(rngstream)	1;		# Needed by the RNG	
	}
}
```

Common simulation options are listed below
```tcl
set opt(nn) 2.0 ;# Number of Nodes
set opt(starttime) 1
set opt(stoptime) 1000
set opt(duration) [expr $opt(stoptime) - $opt(starttime)]
```

Once you have all the needed libraries and options, you need to initialize the ns-miracle simulator and set the random number generator.
```tcl
set ns [new Simulator]
$ns use-Miracle

# Set the default random number generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}
```

Next there are the binding of the modules. Here you can set globally the initialization values of the binded variables.
```tcl
Module/UW/CBR set packetSize_		$opt(pktsize)
# ...
```

Now you are ready to define a tcl procedure (function) that allows you to create as many different kind of nodes in the network.
Inside this procedure, you need to create a ns-miracle node and add the modules you need (the whole protocol stack) and configure them.
Optionally you can initialize and set the node position.
```tcl
proc createNode { id } {
	# include global variables to use in the procedure
	global <variables list>

    # build the NS-Miracle node
	set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]

    # define the module(s) you want to put in the node
	set cbr($id)  [new Module/UW/CBR]
	set udp($id)  [new Module/UW/UDP]
	set ipif($id) [new Module/UW/StaticRouting]
	set ipr($id)  [new Module/UW/IP]
	set mll($id)  [new Module/UW/MLL]
	# ...

    # insert the module(s) into the node 
	# $node addModule <layer> <module> <cl_trace> <tag>
	$node($id) addModule 7 $cbr($id) 0 "CBR"
	# ...

    # intra-node module connections
	# $node setConnection <upper> <lower> <trace>
	$node($id) setConnection $cbr($id) $udp($id) 0
	# ...

	# socket configuration
    set portnum($id) [$udp($id) assignPort $cbr($id) ]
    $ipif($id) addr  [expr ($id) + 1]

    # add node positions (optional)  
	set position($id) [new "Position/BM"]
	$node($id) addPosition $position($id)
	$position($id) setX_ 0
	$position($id) setY_ 0
	$position($id) setZ_ -100

	# ...
}
```

You can have as many procedures you want. Typically there is a specific procedure for the sink node (a node which only receives packets).
Finally, to actually create the nodes you need to call the procedure. Suppose to have `$opt(nn)` nodes in the network, you can do:
```tcl
for {set id 0} {$id < $opt(nn)} {incr id}
{
	createNode $id
}
createSink; # Suppose to have a procedure to create the sink node
```

Once you created the nodes you need to connect them, setup the routes and fill the arp tables.
```tcl
proc connectNodes {id1} {
    global ipif ipr portnum cbr cbr_sink ipif_sink portnum_sink ipr_sink

    $cbr($id1) set destAddr_ [$ipif_sink addr]
    $cbr($id1) set destPort_ $portnum_sink($id1)
    $cbr_sink($id1) set destAddr_ [$ipif($id1) addr]
    $cbr_sink($id1) set destPort_ $portnum($id1)
}

# Setup flows
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    connectNodes $id1
}

# Fill ARP tables
for {set id1 0} {$id1 < $opt(nn)} {incr id1}  {
    for {set id2 0} {$id2 < $opt(nn)} {incr id2}  {
      $mll($id1) addentry [$ipif($id2) addr] [$mac($id2) addr]
    }   
    $mll($id1) addentry [$ipif_sink addr] [ $mac_sink addr]
    $mll_sink addentry [$ipif($id1) addr] [ $mac($id1) addr]
}

# Setup routing table
$ipr(0) addRoute [$ipif_sink addr] [$ipif(1) addr]
$ipr(1) addRoute [$ipif_sink addr] [$ipif_sink addr]
```

Finally you need to start and stop the modules and the simulation.
```tcl
for {set id1 0} {$id1 < $opt(nn)} {incr id1} {
	$ns at $opt(starttime) "$cbr($id1) start"
	$ns at $opt(stoptime) "$cbr($id1) stop"

	# ...

	$ns at $opt(stoptime)  "finish; $ns halt"; # stop simulation and run finish procedure
	$ns run
}
```

Optionally, it is good practice to add a finish procedure that displays the simulation settings and computes relevant statistics.

## How to add a new module
In this section, we describe how to add a new module to the DESERT core (i.e., into the DES_CORE directory). 
As an example, we present the creation of a new module (a library containing a new DESERT module) named `uwcool_phy`.

According to the ISO/OSI structure, our new protocol belongs to the physical layer. 
First, change directory to `DES_CORE/physical` and create a folder named `uwcool_phy`, which will contain your new module.
Then, change directory to the just created folder and create two files called `uwcoolphy.h` and `uwcoolphy.cpp`, which will contain the class definition `UwCoolPhy`.

You will also need to create the following files:

- `initlib.cpp`,
- `uwcoolphy-defaults.tcl`.
- `Makefile.am`,

For this class to work, some types of bindings are necessary.

Inside `uwcoolphy.cpp`, you must always create a **static** class variable that derives from `TclClass` and defines the name that will be used in Tcl to create and bind an object of this module.

In our example, this is done with the following code:

```cpp 
static class UwCoolPhyClass : public TclClass
{
public:
    UwCoolPhyClass()
        : TclClass("Module/UW/CoolPhy")
    {
    }
    TclObject *
    create(int, const char *const *)
    {
        return (new UwCoolPhy);
    }
} class_module_uwcoolphy;
```

Then, we need to link the class parameters to Tcl variables. This is usually done in the `.cpp` constructor using the `bind()` function.
This function takes as input a string representing a Tcl variable, which is bound to the second argument, a cpp class parameter.

```cpp
bind("debug", (int *) &debug_);			// inside uwcoolphy.cpp
```
```tcl
Module/UW/CoolPhy set debug 1			# inside a tcl script
```

The `initlib.cpp` file creates the connection between the cpp library of your module and the Tcl representing your simulation/emulation scenario.
To do so, you need to define an **extern** variable representing an **EmbeddedTcl** object, which folllows the naming convention: `<ModuleNameInCamelCase><TclCode>`.

You also need to define an **extern** function that translates Tcl modules into cpp code to speed up the simulation.
This function must be named after the module, starting with an uppercase letter, followed by the `_Init` suffix.

Following our example, your `initlib.cc` should look like this:

```cpp
#include <tclcl.h>

extern EmbeddedTcl UwCoolPhyTclCode;

extern "C" int
Uwcoolphy_Init()
{
	UwCoolPhyTclCode.load();
	return 0;
}
```

All bound variables need to be initialized to a default value in the `uwcoolphy-defaults.tcl` file.

For instance:

```tcl
Module/UW/CoolPhy set BitRate					32500
Module/UW/CoolPhy set AcquisitionThreshold_dB_	10.0
Module/UW/CoolPhy set debug						0
```

To compile your module, you need to fill the `Makefile.am` file:
- There should be the definition of your library. The name of your library should always contain *lib* prefix, i.e., `libuwcoolphy`.
- Specify the list of sources (`libuwcool_phy_la_SOURCES`), which should contain only the cpp files located in the `uwcool_phy` folder.
- Remember to define the `CPPFLAGS`, `LDFLAGS` and `LIBADD` where needed.
- Define the tcl files variables defaults which are specified inside `uwcool-phy-defaults.tcl`.

Your `Makefile.am` should look like this:

```am
AM_CXXFLAGS = -Wall -ggdb3

lib_LTLIBRARIES = libuwcoolphy.la
libuwcoolphy_la_SOURCES = initlib.cpp uwcoolphy.cpp

libuwcoolphy_la_CPPFLAGS = @NS_CPPFLAGS@ @NSMIRACLE_CPPFLAGS@ @DESERT_CPPFLAGS@
libuwcoolphy_la_LDFLAGS =  @NS_LDFLAGS@ @NSMIRACLE_LDFLAGS@ @DESERT_LDFLAGS@
libuwcoolphy_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@

nodist_libuwcoolphy_la_SOURCES = InitTcl.cc
BUILT_SOURCES = InitTcl.cc
CLEANFILES = InitTcl.cc

TCL_FILES =  uwcoolphy-default.tcl

InitTcl.cc: Makefile $(TCL_FILES)
                cat $(VPATH)/$(TCL_FILES) | @TCL2CPP@ UwCoolPhyTclCode > InitTcl.cc

EXTRA_DIST = $(TCL_FILES)
```

DESERT compilation and linking is based on autotools. 
Specifically, to add a new library, you need to modify the top-level `DES_CORE/Makefile.am` and `DES_CORE/configure.ac` files as follows:
- `Makefile.am`: add your folder to the list of subdirectories (SUBDIRS), using \ as a separator. The order of the list also determines the compilation order; if your module depends on another module (i.e., it includes headers from another module), it should be listed after that. 
- `configure.ac`: add your new folder to the CPPFLAGS, and tell autoconf (AC_CONFIG_FILES) where the new internal Makefile is.

In our example:

`DES_CORE/Makefile.am`
```am

[...]
SUBDIRS = m4 \
		... \
		data_link/uwcool_mac \
        physical/uwcool_phy

```

`DES_CORE/configure.ac`
```ac
[...]
DESERT_CPPFLAGS="$DESERT_CPPFLAGS "'-I$(top_srcdir)/physical/uwcool_phy'
[...]
AC_CONFIG_FILES=([
[...]
physical/uwcool_phy
[...]
])
```

Suppose now, that your module depends on anoter one called `UwCoolMac`.
You also have to modify the `libuwcoolphy_la_LIBADD` flag of the module `Makefile.am` as follows.
```am
libuwcoolphy_la_LIBADD = @NS_LIBADD@ @NSMIRACLE_LIBADD@ @DESERT_LIBADD@ \
                         @DESERT_UWCOOLMAC_LIBADD@
```

Where the variable `@DESERT_UWCOOLMAC_LIBADD@` is defined inside the top-level `configure.ac` (if not, you need to manually define it):
```ac
DESERT_UWCOOLMAC_LIBADD='$(top_builddir)/data_link/uwcoolmac/libuwcoolmac.la'
[...]
AC_SUBST(DESERT_UWCOOLMAC_LIBADD)
```

Once everything is ready, we need to reinstall DESERT to properly add the new module.

> Note: After the module is installed, we can recompile it without reinstalling everything. In our case, we would need to do the following:
```console
$ cd DESERT_buildCopy_LOCAL/.buildHost/DESERT/physical/uwcoolphy/
$ make && make install
```

### The command method
Finally, inside a tcl script we can use our module by:

1. Setting its parameter, 
2. instantiating an object, 
3. connecting or calling the necessary methods on it.

To set the parameters and instantiate the object we can write

```tcl
Module/UW/CoolPhy set BitRate_				   32500
Module/UW/CoolPhy set AcquisitionThreshold_dB_ 10.0

set phy [new Module/UW/CoolPhy]
```

To enable the communication between the cpp class and the tcl script we need to overwrite the command method, for example to get and set parameters of a module object.
It must be declared in this way, in your module class (`uwcoolphy.h`)

```cpp
virtual int command(int argc, const char *const *argv);
```
And then it should be defined in this way (inside `uwcoolphy.cpp`)

```cpp
UwCoolPhy::command(int argc, const char *const *argv) {
	if (argc == 2) {
		if (strcmp(argv[1], "Method1") == 0) {
			method1();
			return TCL_OK;
		}
	}
	else if (argc == 3) { .... }
	
	return Module::command(argc, argv)
}
```

### The printOnLog method
> NOTE: This is a new feature (version >= 3.6). Only the following modules implement it:

- UwCbrModule
- UwApplicationModule
- UwUdp
- UWIPModule
- UwStaticRoutingModule
- CsmaAloha
- UnderwaterPhysical

All the other modules use the ns-way of logging: 
You need to define a `debug_` parameter on your module and bind it to tcl inside the constructor (see the previous section).
Every time you want to print a log message, then you need to check if debug parameter is set and print to console your message.
</blockquote>
Each DESERT module inherits a static logger parameter, shared among all the modules in a simulation istance.
It lets you specify the level of logging messages you want. There are three possible values:

```
ERROR: only error messages will be generated.
INFO : general info about the algorithms running and also ERROR messages.
DEBUG: details that allow to understand the execution flow plus both ERROR and INFO messages are shown.
```


In particular, it gives you the method `printOnLog`, defined as follows:

```cpp
virtual void printOnLog(Logger::LogLevel level, const std::string &module, const std::string &message) const;
```

This method takes three parameter: (i) the log level of the message, (ii) the name of the module printing the message and (iii) a string representing the log message.

As an example, suppose you have a method to handle packets reception that you would like to track, you can do it as follows:

```cpp
void
UwCoolPhy::startRx(Packet *p)
{
	...
	printOnLog(Logger:LogLevel:DEBUG, "UWCOOLPHY", "startRX(Packet *)::Received new packet");
	...
}
```

To enable and configure the log messages in your simulation, you can use the following commands:

```tcl
	setLogLevel <log_level>				;# Set the logger level (1, 2 or 3)
	setLogFile  <file_name>				;# Set the logger file name
	setLog		<log_level> <file_name>	;# Set both the logger level and file name
	enableLog							;# Enable log on a specific module
	disableLog							;# Disable log on a specific module
```

Remember that all the modules in a given simulation, share the same instance of the logger, meaning, that the `log_level` and `log_file` can be set just once.
For instance, suppose, you have a tcl file, with one transmitting node and one sink node, you can set the logger for the sink physical layer only as follows:

```tcl
proc createSink { } {
	...
	set phy_data_sink [new Module/UW/CoolPhy]
	....
	$phy_data_sink setLog 3 "log_output.txt"
	...
}
```

It will print all the `coolphy` log messages inside a file called `log_output.txt`.
Each message has an `id` associated with it, which is the ns id of the node and is incremental set, e.g., the first node created will have `id` 0, the second `id` 1 and so on.

Note: You can find a complete example on how to use the logging system inside `DES_CORE/samples/desert_samples/logger/` folder.

## How to add a new Packet headers

In the case your module needs its own **packet header**, a few things need to be performed:

Define a header *struct*, for example `hdr_uwmyprotocol` inside a file `myproto-hdr.h` with your parameters plus the offset field required by the *packet header manager*:

```cpp
typedef struct hdr_uwmyprotocol 
{
	uint8_t param;
	static int offset_;

	inline uint8_t & param() { return param_ }
	inline static int& offset() {return offset_;}

	inline static struct hdr_uwmyprotocol *
	access(const Packet *p)
	{ 
		return (struct hdr_uwmyprotocol *) p->access(offset_); 
	}
} hdr_uwmyprotocol;
```

To simplify the access of your *header* from the *packet* class, at the beginning of the source file where your header is defined you can define the following macro

```cpp
#define HDR_UWMYPROTOCOL(p) (hdr_uwmyprotocol::access(p))
```

Just after the macro, at the beginning of the file, you need to define a new *packet* type with

```cpp
extern packet_t PT_MYPACKET;
```

Inside initlib.cpp you need to 
- Include your header file and define the offset and the packet type
```cpp
#include<myproto-hdr.h>
...

int hdr_uwmyprotocol::ofset_;
packet_t PT_MYPACKET;
```

- Add a static class

```cpp
static class MyProtoHeaderClass : public PacketHeaderClass
{
public:
    MyProtoHeaderClass()
        : PacketHeaderClass("PacketHeader/MYPROTO", sizeof(hdr_uwmyprotocol))
    {
        this->bind();
        bind_offset(&hdr_uwmyprotocol::offset_);
    }
}
```

- Add the packet inside the `init` function (before the `load` function call), using the same name of the extern

```cpp
PT_MYPACKET = p_info::addPacket("MYPROTO");
```

Finally, inside the *-default.tcl you need to add the packet to the packet manager, with

```tcl
PacketHeaderManager set tab_(PacketHeader/MYPROTO) 1
```

This last step is really important to have the right memory allocation done. Missing it could lead to unpredictable behavior, usually really hard to troubleshoot.


## How to add a new DESERT Add-on

In this section we describe how to add a new module as a DESERT addon.
An addon, is an external autotools project that depends on DESERT, ns-miracle and, in some cases, WOSS or even other addons, so we don’t need to modify the DESERT toplevel `Makefile.am` and `configure.ac`.
We need to create just the addon internal `Makefile.am` and a dedicated `configure.ac` with `m4` folder to resolve the dependencies. 
The complete procedure is described in the following.

Inside the `DESERT_Addons` folder, create your addon folder, say `cool-addon`.
Inside that folder, create the basic files for a properly cpp module, i.e., `cool-addon.h` and `cool-addon.cpp` and the `initlib.cpp` as well.

Similarly to what you did for the adding a module, your `initlib.cpp` will look like

```cpp
#include <tclcl.h>

extern EmbeddedTcl CoolAddonTclCode;

extern "C" int
Cooladdon_Init()
{
	CoolAddonTclCode.load();
	return 0;
}
```

To make the addon compilable and complete, we also need:
- A `Makefile.am`,
- a `configure.ac` config file,
- an initialization file for the tcl, i.e., `cool-addon-init.tcl`,
- the `autogen.sh` script,
- and a `m4` folder

The `Makefile.am` can be taken from another addon or module and modified according to your addond needs, similarly to what what we did for a module.
Just note that in case of a *WOSS* dependent addon, it is better to take the `Makefile.am` from another *WOSS*-based addon, such as `wossgmmob3D` or `wossgroupmob3D`.
Similarly, in case your addon is based on another existing addon, take it from `uwswarm_control` or `ms2c_evologics_lowlev` addons.

Also for the `configure.ac` you take one from another addon and copy into your folder.
You just need to modify the `AC_INIT` with the name of your addon.
Also in this case if you need WOSS functionalities take it from a WOSS-based addon.

The `autogen.sh` file is needed to compile your addon and can be copied from any other addon and doesn't need any modification.

Add all the variables that you have *binded* in your addon class inside the `cool-addon-init.tcl` files as youd did for the module.

The `m4` folder contains the `desert.m4` file and the `desertAddon.m4` and `woss.m4` files if the addon depend from another addon or woss.
Also in this case, it is easier to copy from another addon and modify the dependencies wee ned for our addon.
Those are directories listed in `dir` and `lib` portion of the given `m4` file.

Finally, to have your addon visible during installation, modify the installation file `DESERT_Framework/.addon.list` adding the name of your addon folder.

## How to add a new custom installer
> NOTE: When you see a pathname preceded by three dots, this refers to a well-known but unspecified top-level directory. The top-level directory is context-dependent, but almost always refers to top-level DESERT source directory.
For example, `.../DESERT_Underwater/DESERT_Framework/Installer/installDESERT_LOCAL.sh` refers to the `installDESERT_LOCAL.sh` file located in the architecture branch of a DESERT framework source tree.
The actual path might be something like `/home/foo/DESERT_Underwater/DESERT_Framework/Installer/installDESERT_LOCAL.sh`.

First of all, enter the installation folder:

```console
$ cd .../DESERT_Underwater/DESERT_Framework/Installation/
```

Now, create a new script.
We suggest to copy an existing installation script (e.g. `installDESERT_LOCAL.sh`):

```console
$ cp installDESERT_LOCAL.sh installDESERT_<your-custom-installation-target>.sh
```

Please note the `<your-custom-installation-target>` label:
this will be the same parameter you will pas later as an option to the installation script when indicating which installation-target to choose.
We remark that these steps are very easy but also very important, since the only way to call the correct installation script for `<your-custom-installation-target>` is that the script `installDESERT_<your-custom-installation-target>.sh` exists in the folder `.../DESERT_Underwater/DESERT_Framework/Installation/`

Let us now have a deeper look at the script. We focus first on the main block:

```sh
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

Notice the `host` directive.
This tells the installation script to compile the libraries for the current host.
If you need to modify this script to cross-compile for a different target, substitute all lines with the following ones:

```sh
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
host, some for both the host and the target and the remaining ones only for the target (`target` directive).
This step is crucial, because some libraries must be compiled for the host, in order to correctly cross-compile DESERT and its addons for `<your-custom-installation-target>`.

> WARNING: be careful NOT TO CONFUSE the directive "target" with `<your-custom-installation-target>`
The former is just a directive to the `handle_package` function of the installation script.


We remark that you need the **TOOLCHAIN** of `<your-custom-installation-target>` in order to correctly complete the installation.
