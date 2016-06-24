/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* checkers/Checker_ftrsm.inl
 * Copyright (C) 2016 Ashley Lesdalons
 *
 * Written by Ashley Lesdalons <Ashley.Lesdalons@e.ujf-grenoble.fr>
 *
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

#ifndef __FFLASFFPACK_checker_ftrsm_INL
#define __FFLASFFPACK_checker_ftrsm_INL

#ifdef ENABLE_CHECKER_ftrsm

template <class Field> 
class Checker_ftrsm {

	const Field& F;	
	typename Field::Element_ptr v,w;

public:
	Checker_ftrsm(const Field& F_, const size_t m, const size_t n,
				  const typename Field::Element alpha,
				  const typename Field::ConstElement_ptr B, const size_t ldb) 
			: F(F_), v(FFLAS::fflas_new(F_,n,1)), w(FFLAS::fflas_new(F_,m,1))
	{
		typename Field::RandIter G(F);
		init(G,m,n,B,ldb,alpha);
	}

	Checker_ftrsm(typename Field::RandIter &G, const size_t m, const size_t n,
				  const typename Field::Element alpha,
				  const typename Field::ConstElement_ptr B, const size_t ldb)
			: F(G.ring()), v(FFLAS::fflas_new(F,n,1)), w(FFLAS::fflas_new(F,m,1))
	{
		init(G,m,n,B,ldb,alpha);
	}

	~Checker_ftrsm() {
		FFLAS::fflas_delete(v,w);
	}

	inline bool check(const FFLAS::FFLAS_SIDE side,
					  const FFLAS::FFLAS_UPLO uplo,
					  const FFLAS::FFLAS_TRANSPOSE trans,
					  const FFLAS::FFLAS_DIAG diag,
                      const size_t m, const size_t n,
#ifdef __FFLAS__TRSM_READONLY
	       typename Field::ConstElement_ptr
#else
	       typename Field::Element_ptr
#endif
                      A, size_t lda,
					  const typename Field::ConstElement_ptr X, size_t ldx) {
		size_t k = (side==FFLAS::FflasLeft?m:n);

		typename Field::Element_ptr v1 = FFLAS::fflas_new(F,k,1);
//         std::cerr << side << ' ' << uplo << ' ' << trans << ' ' << diag << std::endl;
//         std::cerr << "lda: " << lda << ", ldx: " << ldx << std::endl;
        
        
//         write_field(F,std::cerr<<"chec A : ",A,k,k,lda,true)<<std::endl;
//         write_field(F,std::cerr<<"chec X : ",X,m,n,ldx,true)<<std::endl;
//         write_field(F,std::cerr<<"chec v : ",v,n,1,1,true)<<std::endl;
        
		if (side==FFLAS::FflasLeft) {
            // (Left) v1 <- X.v 
            // (Left) v1 <- A.v1
            // (Left) w <- w - v1
			FFLAS::fgemv(F, FFLAS::FflasNoTrans, m, n, F.one, X, ldx, v, 1, F.zero, v1, 1);
// write_field(F,std::cerr<<"left v1: ",v1,k,1,1,true)<<std::endl;
            FFLAS::ftrmm(F, FFLAS::FflasLeft, uplo, trans, diag, k, 1, F.one, A, lda, v1, 1);
// write_field(F,std::cerr<<"trsv v1: ",v1,k,1,1,true)<<std::endl;
            FFLAS::fsubin(F, m, v1, 1, w, 1);
        } else {
            // (Right) v <- A.v
            // (Right) w <- X.v - w
			FFLAS::ftrmm(F, FFLAS::FflasLeft, uplo, trans, diag, k, 1, F.one, A, lda, v, 1);
// write_field(F,std::cerr<<"right v : ",v,k,1,1,true)<<std::endl;
            FFLAS::fgemv(F, FFLAS::FflasNoTrans, m, n, F.one, X, ldx, v, 1, F.mOne, w, 1); 
        }
        

//         write_field(F,std::cerr<<"chec w : ",w,m,1,1,true)<<std::endl;
		FFLAS::fflas_delete(v1);
        
		bool pass = FFLAS::fiszero(F,m,1,w,1);
		if (!pass) throw FailureTrsmCheck();
		return pass;
	}

private:	
	inline void init(typename Field::RandIter &G, const size_t m, const size_t n, const typename Field::ConstElement_ptr B, size_t ldb, const typename Field::Element alpha) {
		FFLAS::frand(F,G,n,v,1);
        
// write_field(F,std::cerr<<"init B : ",B,m,n,ldb,true)<<std::endl;
// write_field(F,std::cerr<<"init v : ",v,n,1,1,true)<<std::endl;

		// w <- alpha.B.v
		FFLAS::fgemv(F, FFLAS::FflasNoTrans, m, n, alpha, B, ldb, v, 1, F.zero, w, 1);
// write_field(F,std::cerr<<"init w : ",w,m,1,1,true)<<std::endl;
	}
};

#endif // ENABLE_CHECKER_ftrsm

#endif // __FFLASFFPACK_checker_ftrsm_INL
