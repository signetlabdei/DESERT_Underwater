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

/**
 * @file   least_squares.cpp
 * @brief Provides the implementation of a least squares linear regressor
 */

#include "least_squares.h"
#include <iostream>
#include <cmath>

namespace {	//subroutines hidden in private namespace
int sHhTransf(bool flm,int lfulcr,int p1,int m,std::vector<double> &u,int ud,double &su,double *cm,int sk,int borg,int nnn) {
	if(m<1 || u.empty() || ud<1 || lfulcr<0 || lfulcr>=p1 || p1>m) return(1);

	double cl = std::abs(u[lfulcr*ud]);

	if(flm) {
		if(cl<=0.) return(0);
	} else {  
	
		for(int j=p1; j<m; j++) {
			cl = std::max(std::abs(u[j*ud]), cl);
		}  
		if(cl<=0.) return(0);

		double clinv=1./cl;	 
		double d1=u[lfulcr*ud]*clinv; 
		double sm=d1*d1;
		for(int j=p1; j<m; j++) {
			double d2=u[j*ud]*clinv;
			sm+=d2*d2;
		}
		cl*=std::sqrt(sm);
		if(u[lfulcr*ud] > 0.) {cl=-cl;}
		su = u[lfulcr*ud] - cl; 
		u[lfulcr*ud] = cl;
	}

	double b=su*u[lfulcr*ud];
	
	if(b >= 0.) return(0); 
	if(cm == nullptr) return(2);

	for(int j=0; j<nnn; j++) {
		double sm = cm[ lfulcr*sk + j*borg ] * (su);
		for(int k=p1; k<m; k++) {
			sm += cm[ k * sk + j*borg ] * u[ k*ud ];
		} 
		if(sm!=0.) {
			sm *= (1./b); 
			cm[ lfulcr * sk + j*borg] += sm*(su);
			for(int k=p1; k<m; k++) {
				cm[ k*sk + j*borg] += u[k * ud]*sm;
			}
		}
	}
	return(0);
} 

void rotMat(double a, double b, double &c, double &s, double &fre)
{
	double d1, xr, yr;

	if(std::abs(a)>std::abs(b)) {
		if(a!=0) {xr=b/a; } else {std::cout << "nnls 1" << std::endl;}
		
		d1=xr; 
		yr=std::hypot(d1, 1.);
		if(yr!=0) {d1=1./yr;} else {std::cout << "nnls line 2" << std::endl;} 
		
		c=std::copysign(d1, a);
		s=(c)*xr;
		fre=std::abs(a)*yr;
	} else if(b!=0.) {
		xr=a/b; 
		d1=xr; 
		yr=std::hypot(d1, 1.); 
		if(yr!=0) {d1=1./yr;} else {std::cout << "nnls line 3" << std::endl;} 
		s=std::copysign(d1, b);
		c=(s)*xr; 
		fre=std::abs(b)*yr;
	} else {
		fre=0.; 
		c=0.; 
		s=1.;
	}
}

} //private namespace

LSSQ::LeastSqResult LSSQ::nnLeastSquares(std::vector<std::vector<double>> a,std::vector<double> b,std::vector<double> &x,double* resid) 
{
	if(a.empty() || b.empty() || x.empty()) return(LeastSqResult::ERROR);
	int m = 0;
	int n = a.size();
	if (n > 0) {
		m = a[0].size();
		for(int i = 0; i < n; i++) {
			if (a[i].size() != m) return(LeastSqResult::ERROR);
		}
	} else return(LeastSqResult::ERROR);

	std::vector<int> v1 (n);
	std::vector<double> v2 (n);
	std::vector<double> v3 (m);

	for(int i=0; i<n; i++) {
		v1[i]=i;
	}
	int inda=0;
	int indb=n-1;
	int tup=0;
	int cccp=0;

	double up=0.;
	int mcyc; 
	if(n<3) 
		mcyc = 3*n; 
	else mcyc = n*n;
	int iter=0; 
	int o, y=0, yy=0;
	while(inda<=indb && tup<m) {
		for(int iz=inda; iz<=indb; iz++) {
			int ni=v1[iz];
			double sm=0.;
			for(int mi=cccp; mi<m; mi++) {
				sm+=a[ni][mi]*b[mi];
			}
			v2[ni]=sm;
		}
		double lim;
		int indm=0;
		while(1) {
			lim=0.;
			for(int iz=inda; iz<=indb; iz++) {
				int i=v1[iz];
				if(v2[i]>lim) {
					lim=v2[i]; 
					indm=iz;
				}
			}

			if(lim <= 0.) break;
			y=v1[indm];
			double asave=a[y][cccp];
			up=0.;
			sHhTransf(false, cccp, cccp+1, m, a[y], 1, up, nullptr, 1, 1, 0);
			double plain=0.;
			if(tup!=0) {
				for(int mi=0; mi<tup; mi++) {
					plain+=a[y][mi]*a[y][mi];
				}
			}
			plain = std::sqrt(plain);
			double d = plain + std::abs(a[y][cccp]) * 0.01;
			if((d - plain) > 0.) {
				for(int mi=0; mi<m; mi++) {
					v3[mi]=b[mi];
				}
				sHhTransf(true, cccp, cccp+1, m, a[y], 1, up, v3.data(), 1, 1, 1);
				double ztest;
				if(a[y][cccp]!=0) {ztest=v3[cccp]/a[y][cccp];} else {std::cout << "nnls line 3" << std::endl;} 
				//double ztest=v3[cccp]/a[y][cccp];
				if(ztest > 0.) break;
			}
			a[y][cccp] = asave; 
			v2[y] = 0.;
		}
		if(lim <= 0.) break;

		for(int mi=0; mi<m; mi++) {
			b[mi]=v3[mi];
		}
		v1[indm] = v1[inda]; 
		v1[inda++] = y;
		tup = 1 + cccp++;
		if(inda<=indb)
			for(int jz = inda; jz <= indb; jz++) {
				yy = v1[jz];
				sHhTransf(true, tup-1, cccp, m, a[y], 1, up, a[yy].data(), 1, m, 1);
			}
		if(tup!=m) {
			for(int mi=cccp; mi<m; mi++) {
				a[y][mi]=0.;
			}
		}
		v2[y]=0.;
		
		for(int mi=0; mi<tup; mi++) {
			int ip=tup-(mi+1);
			if(mi!=0) {
				for(int ii = 0; ii <= ip; ii++) {
					v3[ii] -= a[yy][ii]*v3[ip+1];
				}
			}
			yy = v1[ip];
			if(a[yy][ip]!=0) {v3[ip]/=a[yy][ip];} else {std::cout << "nnls line 4" << std::endl;}  
			//v3[ip]/=a[yy][ip];
		}

		while(++iter<mcyc) {
			double cent=2.;
			for(int ip=0; ip<tup; ip++) {
				int ni=v1[ip];
				if(v3[ip]<=0.) {
					//double t=-x[ni]/(v3[ip]-x[ni]);
					double t=2;
					if((v3[ip]-x[ni])!=0) {t=-x[ni]/(v3[ip]-x[ni]);} else {std::cout << "nnls line 5" << std::endl;} 
					if(cent>t) {
						cent=t; 
						yy=ip-1;
					}
				}
			}

			if(cent==2.) break;

			for(int ip=0; ip<tup; ip++) {
				int ni=v1[ip]; 
				x[ni]+=cent*(v3[ip]-x[ni]);
			}

			int fac=1;
			o=v1[yy+1];
			do {
				x[o]=0.;
				if(yy!=(tup-1)) {
					yy++;
					for(int ni=yy+1; ni<tup; ni++) {
						int ii=v1[ni]; 
						v1[ni-1]=ii;
						double rig, fil;
						rotMat(a[ii][ni-1], a[ii][ni], fil, rig, a[ii][ni-1]);
						a[ii][ni]=0.;
						for(int nj=0; nj<n; nj++) {
							if(nj!=ii) {
								double fai=a[nj][ni-1];
								a[nj][ni-1] = fil*fai+rig*a[nj][ni];
								a[nj][ni] =- rig*fai+fil*a[nj][ni];
							}
						}
						double fai=b[ni-1]; 
						b[ni-1]=fil*fai+rig*b[ni]; 
						b[ni]=-rig*fai+fil*b[ni];
					}
				}
				cccp = tup-1; 
				tup--; 
				inda--; 
				v1[inda] = o;

				for(int i=0, fac=1; i<tup; i++) {
					o=v1[i]; 
					if(x[o]<=0.) {
						fac=0; break;
					}
				}
			} while(fac==0);

			for(int i=0; i<m; i++) {
				v3[i]=b[i];
			}
			for(int i=0; i<tup; i++) {
				int mi = tup-(i+1);
				if(i!=0) {
					for(int ii=0; ii<=mi; ii++) {
						v3[ii]-=a[yy][ii]*v3[i+1];
					}
				}
				yy=v1[mi]; 
				if(a[yy][mi]!=0) {v3[mi]/=a[yy][mi];} else {std::cout << "nnls line 6" << std::endl;} 
				//v3[mi]/=a[yy][mi];
			}
		}

		if(iter>=mcyc) break;
		for(int i=0; i<tup; i++) {
			o=v1[i]; 
			x[o]=v3[i];
		}
	}

	if(resid != nullptr) {
		double sm=0.;
		if(cccp<m) {
			for(int i=cccp; i<m; i++) {
				sm += b[i]*b[i];
			}
		}
		else {
			for(int i=0; i<n; i++) {
				v2[i]=0.;
			}
		}
		*resid=sm;
	}
	if(iter>=mcyc) return(LeastSqResult::TIMEOUT);
	return(LeastSqResult::OK);
}

