//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file   uwrandomlib.h
 * @version 1.0.0
 * 
 * \brief Random function header
 *
 * Header of random variable function 
 */


#include <math.h>

#define GENER 3

#define IM1 2147483563L
#define IM2 2147483399L
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014L
#define IA2 40692L
#define IQ1 53668L
#define IQ2 52774L
#define IR1 12211L
#define IR2 3791L
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.0e-10
#define RNMX (1.0-EPS)


class Uwrandomlib
{
	public:

		Uwrandomlib();

		double Gauss(double m,double sigma,int type);

		double Pareto(double alpha, double beta, int type);

	private:

		long int s1[GENER];
		long int s2[GENER];
		long int iy[GENER];
		long int iv[GENER][NTAB];		

		void SetSeed(long int s,int type);

		double Rand01(int type);



};
