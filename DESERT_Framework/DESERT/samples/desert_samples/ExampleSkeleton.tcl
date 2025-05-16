#  Brief description of the script
# 
#  Simple draw of the simulated network topology
#
#

#####################
# Library Loading   #
#####################
# Load here all the NS-Miracle libraries you need
# e.g.,
load libMiracle.so
load libMiracleBasicMovement.so
load libmphy.so
load libmmac.so
load libUwmStd.so
# Then add the DESERT libraries

########################################
# Simulation Options and tcl variables #
########################################
# Put here flags to enable or disable high level options for the simulation setup (optional)
# Put here all the tcl variables you need for simulation management (optional), namely, values for the binded variables, location parameters, module configuration's parameters, ...
# e.g., 
set opt(bash_parameters) 0; # Set to 1 to activate bash parameters
if {$opt(bash_parameters)} {
	if {$argc != 2} {
		puts "The script requires two inputs"
	} else {
		set opt(par1)	[lindex $argv 0]
		set opt(par2)	[lindex $argv 1]
	} else {
		set opt(par1) 125 ; # Bytes
		set opt(par1) 60  ; # Seconds
	}
}

#############################
# NS-Miracle initialization #
#############################
# You always need the following two lines to use the NS-Miracle simulator
set ns [new Simulator]
$ns use-Miracle

# Set the default random number generator
global defaultRNG
for {set k 0} {$k < $opt(rngstream)} {incr k} {
	$defaultRNG next-substream
}


#########################
# Module Configuration  #
#########################
# Put here all the commands to set globally the initialization values of the binded variables (optional)
# e.g., 
Module/UW/Module_Name set module_variable  variable_value

################################
# Procedure(s) to create nodes #
################################
# Define here one or more procedures that allow you to create as many different kind of nodes
proc createNode { id } {
    
    # include all the global variable you are going to use inside this procedure
	global <variables>
    
    # build the NS-Miracle node
	set node($id) [$ns create-M_Node $opt(tracefile) $opt(cltracefile)]
    
    # define the module(s) you want to put in the node
	set module_name($id) [new Module/UW/Module_Name]
    
    # insert the module(s) into the node 
	$node addModule <layer> <module> <cl_trace> <tag>
    
    # intra-node module connections (if needed)
	$node setConnection <upper> <lower> <trace>
    
    # set module and node parameters (optional)
   
    # initialize node's modules (if needed)
    
    # add node positions (optional)  
 
}

#################
# Node Creation #
#################
# Create here all the nodes you want to network together
# e.g., 
createNode 1

################################
# Inter-node module connection #
################################
# Put here all the commands required to connect nodes in the network (optional), namely, specify end to end connections, fill ARP tables, define routing settings

#####################
# Start/Stop Timers #
#####################
# Set here the timers to start and/or stop modules (optional)
# e.g., 
$ns at time_start "$module_name start"
$ns at time_stop "$module_name stop"

###################
# Final Procedure #
###################
# Define here the procedure to call at the end of the simulation
proc finish {} {
	
   # computation of the statics

   # display messages

   # save traces

   # close files

}


##################
# Run simulation #
##################
# Specify the time at which to call the finish procedure and halt ns
# e.g.,
$ns at time_value  "finish; $ns halt" 

# You always need the following line to run the NS-Miracle simulator
$ns run
