# Coding rules

This file shows the coding rules of the project

## Indentation

* Tab, length 4, no spaces
* the maximum line length is 80 columns
* do not mix tab and spaces
* For multi-line commands (e.g. commands that goes over 80 columns, use double indentation)
```C
x = super_mega_long_function_with_a_lot_of_params(int a,
		int b, int c, int d, int e, int f, int g, int as,
		int z);
```
## Functions/Methods

* On declaration return type and method modifiers shall be on the same line
```C
static int FooClass::my_method(int a, int b);
```
* On implementation return type and method modifiers shall be on a single line
```C
static int
FooClass::my_method(int a, int)
{
	/* code here */
}
```
* Curly brackets (opening and closing shall be on a separate line
* Return values shall be around round brackets
```C
static int
FooClass::my_method(int a,int b)
{
	return (1);
}
```
* Use empty lines to create logical separations between actions into methods
```C
static int
FooClass::my_other_method(int c)
{
	/* Variable declarations*/
	
	/* core of the method */

	/* return types or error/failure paths*/
}
```
* Use lowercase letters for methods and CamelCase for Classes
## Control flow

* Put a space before the parentheses in `if`, `switch`, `for` and `while`
* Open curly brackets on the same line of loops and `if`
* Put a space before and after binary operators

```C
if (a == b) {
	/*do somehting*/
} else {
	/*do somehting else */
}

for (int i=0, i < 1000, i++) {
	/* do the same for 1000 times */
}

switch (a) {
	case A:
		/*A*/
		break;
	case B:
		/*B*/
		break;
}
```

## Doxygen

* Doxygen shall be only on .h file

* The file should have this header (Please note that copyright date should be updated regularly)
```C
//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the 
//    names of its contributors may be used to endorse or promote products 
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file   uwpolling_NODE.h
 * @author Federico Favaro
 * @version 1.0.0 
 *
 * \brief Class that represents a node of UWPOLLING 
 *
 *
 */
```
* Method definition should have this doxygen
```
    /**
     * TCL command interpreter. It implements the following OTcl methods:
     * 
     * @param argc Number of arguments in <i>argv</i>.
     * @param argv Array of strings which are the command parameters (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched successfully or not.
     * 
     **/
	 virtual int command(int argc, const char*const* argv);
```
* Variables should have this doxygen
```C
Packet* curr_data_pkt; /**< Pointer to the current DATA packet */
```
