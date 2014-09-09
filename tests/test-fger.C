/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s

/*
 * Copyright (C) FFLAS-FFPACK
 * Written by JG Dumas
 * This file is Free Software and part of FFLAS-FFPACK.
 *
 * ========LICENCE========
 * This file is part of the library FFLAS-FFPACK.
 *
 * FFLAS-FFPACK is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */


//--------------------------------------------------------------------------
//                        Test for fger : 1 computation
//
//--------------------------------------------------------------------------
// Clement Pernet
//-------------------------------------------------------------------------

//#define DEBUG 1
#define TIME 1

#include <iomanip>
#include <iostream>
#include "fflas-ffpack/field/modular-balanced.h"
#include "fflas-ffpack/utils/timer.h"
#include "Matio.h"
#include "fflas-ffpack/fflas/fflas.h"
//#include "fflas-ffpack/field/modular-positive.h"
//#include "fflas-ffpack/field/modular-balanced.h"
#include "fflas-ffpack/field/modular-int32.h"

#include "fflas-ffpack/utils/args-parser.h"
#include "test-utils.h"
#include "givaro/givintprime.h"


// checks that D = alpha . x . y^T + C

// WARNING
template<class Field>
bool check_fger(const Field                   & F,
	      const typename Field::Element_ptr  Cd, // c0
	      const size_t                    m,
	      const size_t                    n,
	      const typename Field::Element & alpha,
	      const typename Field::Element_ptr  x,
	      const size_t                    incx,
	      const typename Field::Element_ptr  y,
	      const size_t                    incy,
	      const typename Field::Element_ptr  C, // res
	      const size_t                    ldc
	      ) {
	bool wrong = false;

	typedef typename Field::Element Element;
	typedef typename Field::Element_ptr Element_ptr;

// 	std::cerr << "with(LinearAlgebra):" << std::endl;
//         write_field(F,std::cerr <<"X:=",x, m, 1, incx, true) << ';' << std::endl;
//         write_field(F,std::cerr <<"Y:=Transpose(",y, n, 1, incy, true) << ");" << std::endl;
//         write_field(F,std::cerr <<"A:=",Cd, m, n, ldc, true) << ';' << std::endl;
// 	F.write(std::cerr << "a:=", alpha) << ';' << std::endl;
// 	std::cerr << "q:=" << F.characteristic() << ';' << std::endl;

	Element_ptr D  = FFLAS::fflas_new (F,m,n);
	FFLAS::fcopy(F,m,n,Cd,n,D,n);
	for(size_t i=0; i<m; ++i) {
		Element tmp; F.init(tmp);
		F.mul(tmp, alpha, *(x+i*incx) );
		for(size_t j=0; j<n; j+=incy) {
			F.axpyin(*(D+i*n+j), tmp, *(y+j) );
			if ( !F.areEqual( *(D+i*n+j), *(C+i*ldc+j) ) ) {
				wrong = true;
			}
		}
	}
//     write_field(F,std::cerr <<"d:=",D, m, n, ldc, true) << ';' << std::endl;
// 	F.write(std::cerr, alpha) << "*X.Y+A,d;";
// 	F.write(std::cerr, alpha) << "*X.Y+A-d mod q;" << std::endl;
	if ( wrong ){
		size_t ici = 20 ;
		std::cout<<"FAIL"<<std::endl;
		std::cout << "a   :" << alpha<<std::endl;
		std::cout << "m   :" << m   << ", n   : " <<  n  << std::endl;
		std::cout << "incx :" << incx << ", incy : " << incy << ", ldC : " << ldc << std::endl;
		for (size_t i=0; i<m && ici; ++i){
			for (size_t j =0; j<n && ici; ++j)
				if (!F.areEqual( *(C+i*ldc+j), *(D+i*n+j) ) ) {
					std::cout<<"Error C["<<i<<","<<j<<"]="
					<<(*(C+i*ldc+j))<<" D["<<i<<","<<j<<"]="
					<<(*(D+i*n+j))<<std::endl;
					ici--;
				}
		}
		if (m<80 && n<80) {
			for (size_t i=0; i<m ; ++i){
				for (size_t j =0; j<n ; ++j) {
					if ( !F.areEqual( *(C+i*ldc+j), *(D+i*n+j) ) )
						std::cout << 'X' ;
					else
						std::cout << '.' ;
				}
				std::cout << std::endl;
			}
		}
	}
	FFLAS::fflas_delete (D);

	return !wrong ;
}


template<class Field>
bool launch_fger(const Field & F,
	       const size_t   m,
	       const size_t   n,
	       const typename Field::Element alpha,
	       const size_t ldc,
	       const size_t inca,
	       const size_t incb,
	       size_t iters)
{
	bool ok = true;

	typedef typename Field::Element_ptr Element_ptr;
	Element_ptr A ;
	FFLASFFPACK_check(inca >= 1);
	Element_ptr B ;
	FFLASFFPACK_check(incb >= 1);
	Element_ptr C = FFLAS::fflas_new (F,m,ldc);
	FFLASFFPACK_check(ldc >= n);
	FFLAS::fzero(F,m,n,C,ldc);
	Element_ptr D = FFLAS::fflas_new (F, m, n);
	for(size_t i = 0;i<iters;++i){
		A = FFLAS::fflas_new (F, m*inca, 1);
		RandomMatrix(F,A,m*inca,1,1);
		B = FFLAS::fflas_new (F, n*incb, 1);
		RandomMatrix(F,B,n*incb,1,1);
		RandomMatrix(F,C,m,n,ldc);
		FFLAS::fcopy(F,m,n,C,ldc,D,n);
		FFLAS::fger (F,m,n,alpha, A, inca, B, incb, C,ldc);
		ok &= check_fger(F, D, m,n,alpha, A, inca, B, incb, C,ldc);

		FFLAS::fflas_delete(A);
		FFLAS::fflas_delete(B);

		if (!ok)
			break;


	}
	FFLAS::fflas_delete (C);
	FFLAS::fflas_delete (D);

	return ok ;
}


template<class Field>
bool launch_fger_dispatch(const Field &F,
			const size_t nn,
			const typename Field::Element alpha,
			const size_t iters)
{
	bool ok = true;
	size_t m,n;
	size_t inca,incb,ldc;
		//!@bug test for incx equal
		//!@bug test for transpo
		//!@todo does nbw actually do nbw recursive calls and then call blas (check ?) ?
	// size_t ld = 13 ;
	{
		m = 1+(size_t)random()%nn;
		n = 1+(size_t)random()%nn;


// 		lda = m+(size_t)random()%ld;
// 		ldb = 1+(size_t)random()%ld;

		inca = 1;
		incb = 1;

// 		ldc = n+(size_t)random()%ld;
		ldc = n;


		std::cout <<"q = "<<F.characteristic()<<" m,n = "<<m<<", "<<n<<" C := "
			  <<alpha<<".x * y^T + C";
		ok &= launch_fger<Field>(F,m,n,
				       alpha,
				       ldc,
				       inca,
				       incb,
				       iters);
		std::cout<<(ok?" -> ok ":" -> KO")<<std::endl;
	}
	return ok ;
}
template <class Field>
bool run_with_field (int q, unsigned long b, size_t n, size_t iters){
	bool ok = true ;
	unsigned long p=q;
	int nbit=(int)iters;
	while (ok &&  nbit){
		typedef typename  Field::Element Element ;
		typedef typename Field::RandIter Randiter ;
		typedef typename Field::Element  Element ;
		if (q==-1){
			b = 2 + (rand() % (int)floor(log(Field::getMaxModulus())/log(2.)+1));
		}
		Givaro::IntPrimeDom IPD;
		Givaro::Integer tmp;
		if (b > 1){
			    // Choose characteristic as a random prime of b bits
			do{
				Givaro::Integer _p;
				Givaro::Integer::random_exact_2exp(_p,b);//max % (2<<30);
				IPD.prevprime( tmp, _p+1 );
				p =  tmp;
			}while( (p < 2) );
		}
		if (p > (unsigned long)Field::getMaxModulus()){
			IPD.prevprime( tmp, Field::getMaxModulus()+1 );
			p=tmp;
		}
		p = (int)std::max((unsigned long) Field::getMinModulus(),(unsigned long)p);
		Field F((int)p);

		Randiter R1(F);
		FFPACK::NonzeroRandIter<Field,Randiter> R(F,R1);

		    //size_t k = 0 ;
		    //std::cout << k << "/24" << std::endl; ++k;
		ok &= launch_fger_dispatch<Field>(F,n,F.one,iters);
		    //std::cout << k << "/24" << std::endl; ++k;
		ok &= launch_fger_dispatch<Field>(F,n,F.zero,iters);
		    //std::cout << k << "/24" << std::endl; ++k;
		ok &= launch_fger_dispatch<Field>(F,n,F.mOne,iters);
		    //std::cout << k << "/24" << std::endl; ++k;

		Element alpha ;
		R.random(alpha);

		ok &= launch_fger_dispatch<Field>(F,n,alpha,iters);

		    //std::cout<<std::endl;
		nbit--;
	}
	return ok;
}

using namespace std;
using namespace FFPACK;

typedef Modular<double> Field;

int main(int argc, char** argv)
{
	std::cout<<setprecision(17);
	std::cerr<<setprecision(17);
	srand((int)time(NULL));
	srand48(time(NULL));

	static size_t iters = 3 ;
	static int q = -1 ;
	static unsigned long b = 0 ;
	static size_t n = 50 ;
	static bool loop = false;
	static Argument as[] = {
		{ 'q', "-q Q", "Set the field characteristic (-1 for random).",         TYPE_INT , &q },
		{ 'b', "-b B", "Set the bitsize of the random characteristic.",         TYPE_INT , &b },
		{ 'n', "-n N", "Set the dimension of the matrix.",      TYPE_INT , &n },
		{ 'i', "-i R", "Set number of repetitions.",            TYPE_INT , &iters },
		{ 'l', "-loop Y/N", "run the test in an infinte loop.", TYPE_BOOL , &loop },
		END_OF_ARGUMENTS
	};

	FFLAS::parseArguments(argc,argv,as);


	bool ok = true;
	do{
		std::cout<<"Modular<double>"<<std::endl;
		ok &= run_with_field<Modular<double> >(q,b,n,iters);
		std::cout<<"ModularBalanced<double>"<<std::endl;
		ok &= run_with_field<ModularBalanced<double> >(q,b,n,iters);
		std::cout<<"Modular<float>"<<std::endl;
		ok &= run_with_field<Modular<float> >(q,b,n,iters);
		std::cout<<"ModularBalanced<float>"<<std::endl;
		ok &= run_with_field<ModularBalanced<float> >(q,b,n,iters);
		std::cout<<"Modular<int32_t>"<<std::endl;
		ok &= run_with_field<Modular<int32_t> >(q,b,n,iters);
		std::cout<<"ModularBalanced<int32_t>"<<std::endl;
		ok &= run_with_field<ModularBalanced<int32_t> >(q,b,n,iters);
		//std::cout<<"Modular<int64_t>"<<std::endl;
		//ok &= run_with_field<Modular<int64_t> >(q,b,n,iters);
		// std::cout<<"ModularBalanced<int64_t>"<<std::endl;
		// ok &= run_with_field<ModularBalanced<int64_t> >(q,b,n,iters);
	} while (loop && ok);

	return !ok ;
}
