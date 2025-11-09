# Coding rules

This page provides a set of rules to follow when contributing to the `DESERT_Underwater`.

## Indentation

* Tab, length 4, no spaces (do not mix tab and spaces).
* The maximum line length is 80 columns. A line may exceed the maximum length, only if it is:
    * A comment line which is not feasible to split without harming readability, ease of cut and paste or auto (e.g., a literal URL),
    * a string literal that cannot easily be wrapped at 80 columns,
    * an include statement,
    * a header guard,
    * a using-declaration.
* For multi-line commands (e.g. commands that goes over 80 columns), use double indentation.
```C
x = superMegaLongFunctionWithALotOfParams(int a,
		int b, int c, int d, int e, int f, int g, int as,
		int z);
```
## Naming

* Filenames should be all lowercase and can include underscores (_) or dashes (-). Specifically, C++ files should have a .cpp filename extension, and header files a .h extension. 
```
my-cool-module.cpp
my-cool-module.h
my_cool_hdr.h
```
* Type names start with a capital letter and have a capital letter for each new word (`PascalCase`). The only exception are packet header structs which should start with the word `hdr_` followed by lowercase words separated by underscores (`snake_case`).
```C
// Classes and structs
class MyCoolModule { ...
struct MyCoolStruct { ...
struct hdr_mycoolmodule { ...

// typedefs
typedef std::map<int , Status> StatusMap;

// using aliases
using StatusMap = std::map<int , Status>;

// enums
enum class LogLevel { ...
```
* Functions/Methods start with a lowercase letter and have a capital letter for each new word (`camelCase`). The only exception are packet header functions which should be `snake_case`.
* Variables and data members are `snake_case`. Additionally data members (of both classes and structs) have a trailing underscore.

## Functions/Methods

* On declaration return type and method modifiers shall be on the same line.
```C
static int FooClass::myMethod(int a, int b);
```
* On implementation return type and method modifiers shall be on a single line.
```C
static int
FooClass::myMethod(int a, int)
{
	/* code here */
}
```
* Curly brackets (opening and closing shall be on a separate line).
* Use empty lines to create logical separations between actions into methods.
```C
static int
FooClass::myOtherMethod(int c)
{
	/* Variable declarations*/
	
	/* core of the method */

	/* return types or error/failure paths*/
}
```
* Use parentheses in `return expr` only if needed.
```C
// No parentheses in the simple case.
return result;
// Parentheses OK to make a complex expression more readable.
return (some_long_condition && another_condition);
```
## Control flow

* Put a space before the parentheses in `if`, `switch`, `for` and `while`.
* Open curly brackets on the same line of loops and `if`.
* Put a space before and after binary operators.

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

* Doxygen shall be only on .h file.
* The file should have this header (Please note that copyright date should be updated regularly).
```C
//
// Copyright (c) 2025 Regents of the SIGNET lab, University of Padova.
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
* Method definitions should have this doxygen.
```C
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
* Data members should have this doxygen.
```C
Packet* curr_data_pkt_; /**< Pointer to the current DATA packet */
int pkts_lost; /**< Total number of lost packets, including packets received
                    out of sequence. */
```

## Clang-format

Inside the tools directory there is a script to auto-format a module using clang-format.
An example usage is the following:

```console
$ ./tools/format-code.sh <path-to-module>
```

If clang-format is already installed in your systems, it will format the module according to the coding rules, otherwise it will try to install clang-format.
