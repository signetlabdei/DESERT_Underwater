//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file uwminterpreter.h
 * @author Riccardo Masiero, Matteo Petrani
 * \version 1.0.0
 * \brief Header of the class that is in charge of building/parsing the necessary messages to make the UWMdriver able to communicate with the modem.
 */

#ifndef UWMINTERPRETER_H
#define UWMINTERPRETER_H

// Forward declaration to avoid dependence from UWMdriver.h.
class UWMdriver; 


/** 
 * This class is in charge of building/parsing the necessary messages to make the UWMdriver able to communicate with the modem. This class provides a basic interface to implement an interpreter for an UWMdriver object. NOTE: any class which inherits from UWMinterpreter should implements its own methods to build and parse messages between host and modem; therefore, such derived class should know the reception variables of the UWMdriver object to which it points, along with all the methods to update them (since the UWMdriver object can be extended through inheritance, any new class that extends UWMdriver must provide all the necessary methods to allow the update of its reception variables).
 */
class UWMinterpreter
{
	public:

	/** 
	 * Class constructor.
	 * 
	 * @param pmDriver_  pointer to the UWMdriver object to link with this UWMinterpreter object.
	 */
	UWMinterpreter(UWMdriver*);
		
	/**
	 * Class destructor.
	 */
	~UWMinterpreter();
	
	// METHODS to BUILD MESSAGES
	
	// METHODS to PARSE MESSAGES
	// NOTE: These methods must know and use the reception variable of the linked UWdriver object and the corresponding methods to update them

	protected:
	 
	UWMdriver* pmDriver; /**< Link to the UWMdriver object which contains this UWMinterpreter object. */
	int debug_; /**< Flag to enable debug mode (i.e., printing of debug messages) if set to 1. */
};
#endif	/*UWMINTERPRETER_H */
