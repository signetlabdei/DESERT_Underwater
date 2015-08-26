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
 * @file   uwrandomlib.c
 * @version 1.0.0
 * 
 * \brief Library of random variable functions
 *
 * Library of random variable functions (e.g. Pareto distribuction) 
 */


#include "uwrandomlib.h"

Uwrandomlib::Uwrandomlib()
 {
  for(int i = 0; i < GENER; i ++)
  {
    s1[i] = 1;
    s2[i] = 123456789L;
    iy[i] = 0;
  }
}
void Uwrandomlib::SetSeed(long int s,int type){
  int j;
  long int k;

  if(s==0)s=1;
  s1[type]=s;
  s2[type]=s;
  for(j=NTAB+7;j>=0;j--){
    k=s1[type]/IQ1;
    s1[type]=IA1*(s1[type]-k*IQ1)-k*IR1;
    if(s1[type]<0)s1[type]+=IM1;
    if(j<NTAB)iv[type][j]=s1[type];
  }
  iy[type]=iv[type][0];
}

double Uwrandomlib::Rand01(int type){
  int j;
  long int k;
  double t;

  k=s1[type]/IQ1;
  s1[type]=IA1*(s1[type]-k*IQ1)-k*IR1;
  if(s1[type]<0)s1[type]+=IM1;

  k=s2[type]/IQ2;
  s2[type]=IA2*(s2[type]-k*IQ2)-k*IR2;
  if(s2[type]<0)s2[type]+=IM2;

  j=iy[type]/NDIV;
  iy[type]=iv[type][j]-s2[type];
  iv[type][j]=s1[type];
  if(iy[type]<1)iy[type]+=IMM1;
  if((t=AM*iy[type])>RNMX)
    return RNMX;
  else
    return t;
}

// Generate gaussian distributed numbers with mean m and stdev sigma
double Uwrandomlib::Gauss(double m,double sigma,int type){
  static int cache=0;
  static float v1;
  float v2,w,y;

  if(cache){
    cache=0;
    return v1*sigma+m;
  }
  else{
    do{
      v1=2.0*Rand01(type)-1.0;
      v2=2.0*Rand01(type)-1.0;
      w=(v1*v1)+(v2*v2);
    }while(w>1.0);
    y=sqrt((-2.0*log(w))/w);
    v1=v1*y;
    cache=1;
    return v2*y*sigma+m;
  }
}

// Generate Pareto distributed numbers
// FX(x)=Prob(X<=x)
// FX(x)=1-((beta+x)/beta)^-alpha	x>=0
// FX(x)=0				otherwise
double Uwrandomlib::Pareto(double alpha, double beta, int type)
{	
	return( beta*(pow( (1.-Rand01(type)),alpha)-1.) );
}
