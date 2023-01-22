//
// Copyright (c) 2022 Regents of the SIGNET lab, University of Padova.
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
//


#ifndef UWTOKENBUS_RANGH
#define UWTOKENBUS_RANGH

#include <vector>

namespace LSSQ
{
    enum LeastSqResult {OK = 0, TIMEOUT = 1, ERROR = 2}; /**< enumerates the possible outputs of nnLeastSquares() */

    /** 
    * 	@brief Least Squares Linear Regressor solves the least squares problem A * X = B, X>=0 
    *	@param a NxM matrix A: the first index returns the vector of samples related to a single unknown, the second index individuates the sample
    *	@param b vector of known terms of size M 
    *	@param x vector of size N to hold the solution output
    *	@param resid (optional) outputs the squared norm of the residual vector
    *	@return 0 = OK, 1 = TIMEOUT, 2 = ERROR
    */
    LeastSqResult nnLeastSquares(std::vector<std::vector<double>> a,std::vector<double> b,std::vector<double> &x,double* resid = nullptr);

}
#endif