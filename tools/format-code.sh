#!/bin/bash

# This is a tool to "clang-format" the code of a DESERT module
# Put the path (absolute or relative) of the module you want to format.
# If clang-formaat is not installed it will try to install it.
# Note: only valid on 64 bits linux systems.

if [ $# -lt 1 ]; then
    echo "Insert relative path of module you want to format using clang."
    exit 1
fi
CODE_PATH=$1

CLANG_COMMAND="clang-format"

install_clang() {
	uname -a | grep 'Linux.*x86_64' &> /dev/null
	if [ $? != 0 ]; then
	   echo "The auto-install works only on 64bit linux systems, sorry."
	   exit 1
	fi

	sudo wget -qO /usr/local/bin/clang-format https://github.com/cpp-linter/clang-tools-static-binaries/releases/latest/download/clang-format-20_linux-amd64
	sudo chmod a+x /usr/local/bin/clang-format
	echo "Successfully installed $(${CLANG_COMMAND} --version)"
}

check_clang_installed() {
	if ! command -v ${CLANG_COMMAND} >/dev/null 2>&1
	then
		install=''
	    read -p "${CLANG_COMMAND} is not installed, would you like to installl it? [Y/n]" install
	    case "${install}" in
	        [Yy]|[Yy]es)
				install_clang
	            ;;
	        [Nn]|[Nn]o)
				echo "Exiting..."
				exit 0
	            ;;
	        "")
				install_clang
	            ;;
	        *)
				check_clang_installed
	            ;;
		esac
	fi
}

check_clang_installed

$CLANG_COMMAND -i $CODE_PATH/*.h > /dev/null  2>&1 
$CLANG_COMMAND -i $CODE_PATH/*.cc > /dev/null  2>&1 
$CLANG_COMMAND -i $CODE_PATH/*.cpp > /dev/null  2>&1 
echo "Module formatted correctly."
