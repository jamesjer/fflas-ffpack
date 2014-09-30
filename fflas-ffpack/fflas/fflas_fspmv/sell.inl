/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/*
 * Copyright (C) 2014 the FFLAS-FFPACK group
 *
 * Written by   BB <bbboyer@ncsu.edu>
 *              Bastien Vialla <bastien.vialla@lirmm.fr>
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

/** @file fflas/fflas_fspmv_sell.inl
 * NO DOC
*/

#ifndef __FFLASFFPACK_fflas_fflas_spmv_sell_INL
#define __FFLASFFPACK_fflas_fflas_spmv_sell_INL

namespace FFLAS {

template<class _Element>
	struct SELL {
		size_t m = 0;
		size_t n = 0;
		size_t chunk = 0;
		size_t nChunks = 0;
		uint32_t * col = nullptr;
		uint32_t * perm = nullptr;
		uint64_t * ptr = nullptr;
		uint32_t * chs = nullptr;
		_Element * dat = nullptr;
	};

	template<class _Element>
	struct SELL_sub : public ELL<_Element, _Simd> {
	};

template<class _Element>
	struct SELL_ZO : public ELL<_Element, _Simd> {
		_Element cst = 1;
	};

	namespace details{

		// y = A x + b y ; (generic)
		template<class Field>
		inline void fspmv(
			      const Field& F,
			      const size_t m,
			      const size_t n,
			      const size_t chunk,
			      const uint32_t nChunks,
			      const uint32_t * col,
			      const uint64_t * ptr,
			      const uint32_t * chs,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      typename Field::Element * y,
			      FieldCategories::GenericTag
			     )
		{
			for(size_t i = 0 ; i < nChunks ; ++i){
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					for(size_t k = 0 ; k < chunk ; ++k){
						F.axpyin(y[i*chunk], dat[start+j*chunk+k], x[col[start+j*chunk+k]]);
					}
				}
			}
		}

		template<class Field>
		inline void fspmv(
			      const Field& F,
			      const size_t m,
			      const size_t n,
			      const size_t chunk,
			      const uint32_t nChunks,
			      const uint32_t * col,
			      const uint64_t * ptr,
			      const uint32_t * chs,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      typename Field::Element * y,
			      FieldCategories::FloatingPointTag
			     )
		{
#ifdef __FFLASFFPACK_USE_SIMD
			using simd = Simd<typename Field::Element>;
			using vect_t = typename simd::vect_t;

			vect_t vx, vy, vdat;

			for(size_t i = 0 ; i < nChunks ; ++i){
				vy = simd::zero();
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					vdat = simd::load(dat+start+j*chunk);
					vx = simd::gather(x, col+start+j*chunk);
					vy = madd(vy, vx, vdat);
				}
				simd::store(y+i*chunk, vy);
			}
#else
			for(size_t i = 0 ; i < nChunks ; ++i){
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					for(size_t k = 0 ; k < chunk ; ++k){
						y[i*chunk]+=dat[start+j*chunk+k]*x[col[start+j*chunk+k]];
					}
				}
			}
#endif
		}

		// y = A x + b y ; (generic)
		template<class Field>
		inline void fspmv_zo(
			      const Field& F,
			      const size_t m,
			      const size_t n,
			      const size_t chunk,
			      const uint32_t nChunks,
			      const uint32_t * col,
			      const uint64_t * ptr,
			      const uint32_t * chs,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      typename Field::Element * y,
			      FieldCategories::GenericTag
			     )
		{
			for(size_t i = 0 ; i < nChunks ; ++i){
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					for(size_t k = 0 ; k < chunk ; ++k){
						F.addin(y[i*chunk], x[col[start+j*chunk+k]]);
					}
				}
			}
		}

		template<class Field>
		inline void fspmv_zo(
			      const Field& F,
			      const size_t m,
			      const size_t n,
			      const size_t chunk,
			      const uint32_t nChunks,
			      const uint32_t * col,
			      const uint64_t * ptr,
			      const uint32_t * chs,
			      const typename Field::Element *  dat,
			      const typename Field::Element * x ,
			      typename Field::Element * y,
			      FieldCategories::FloatingPointTag
			     )
		{
#ifdef __FFLASFFPACK_USE_SIMD
			using simd = Simd<typename Field::Element>;
			using vect_t = typename simd::vect_t;

			vect_t vx, vy, vdat;

			for(size_t i = 0 ; i < nChunks ; ++i){
				vy = simd::zero();
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					vdat = simd::load(dat+start+j*chunk);
					vx = simd::gather(x, col+start+j*chunk);
					vy = add(vy, vx);
				}
				simd::store(y+i*chunk, vy);
			}
#else
			for(size_t i = 0 ; i < nChunks ; ++i){
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					for(size_t k = 0 ; k < chunk ; ++k){
						y[i*chunk]+=x[col[start+j*chunk+k]];
					}
				}
			}
#endif
		}


		// delayed by kmax
		//! @bug check field is M(B)<f|d>
		template<class Field >
		inline void fspmv(
			      const Field& F,
			      const size_t m,
			      const size_t n,
			      const size_t chunk,
			      const uint32_t nChunks,
			      const uint32_t * col,
			      const uint64_t * ptr,
			      const uint32_t * chs,
			      const typename Field::Element * dat,
			      const typename Field::Element * x ,
			      typename Field::Element * y,
			      const index_t & kmax,
			      FieldCategories::ModularFloatingPointTag
			      )
		{
#ifdef __FFLASFFPACK_USE_SIMD
			using simd = Simd<typename Field::Element>;
			using vect_t = typename simd::vect_t;
			vect_t X, Y, D, C, Q, TMP;
			double p = (typename Field::Element)F.characteristic();
			vect_t P = simd::set1(p);
			vect_t NEGP = simd::set1(-p);
			vect_t INVP = simd::set1(1./p);
			vect_t MIN = simd::set1(F.minElement());
			vect_t MAX = simd::set1(F.maxElement());

			size_t end = (m%chunk == 0)? m : m+m%chunk;

			for ( size_t i = 0; i < end/chunk ; ++i ) {
				index_t j = 0 ;
				index_t j_loc = 0 ;
				Y = simd::load(y+i*chunk);
				for (size_t l = 0 ; l < block ; ++l) {
					j_loc += kmax ;
					for ( ; j < j_loc ; ++j) {
						D = simd::load(dat+i*chunk*ld+j*chunk);
						X = simd::gather(x,col+i*chunk*ld+j*chunk);
						Y = simd::madd(Y,D,X);
					}
					vectorised::VEC_MOD(Y,Y,TMP, P, NEGP,INVP,MIN,MAX);
				}
				for ( ; j < ld ; ++j) {
					D = simd::load(dat+i*chunk*ld+j*chunk);
					X = simd::gather(x,col+i*chunk*ld+j*chunk);
					Y = simd::madd(Y,D,X);
				}
				vectorised::VEC_MOD(Y,Q,TMP, P, NEGP,INVP,MIN,MAX);
				simd::store(y+i*chunk,Y);
			}
#else

			for(size_t i = 0 ; i < nChunks ; ++i){
				for(size_t j = 0 ; j < chs[i] ; ++j){
					size_t start = ptr[i];
					for(size_t k = 0 ; k < chunk ; ++k){
						y[i*chunk]+=x[col[start+j*chunk+k]];
					}
				}
			}
			for(size_t i = 0 ; i < nChunks ; ++i){
				size_t j = 0, j_loc = 0;
				for(size_t l = 0 ; l < block ; ++l, j_loc=+kmax){
					size_t start = ptr[i];
					for(; j < j_loc ; ++j){
						for(size_t k = 0 ; k < chunk ; ++k){
							y[i*chunk+k] += dat[start+j*chunk+k]*x[col[start+j*chunk+k]];
						}
						for(size_t k = 0 ; k < chunk ; ++k){
							F.init(y[i*chunk+k], y[i*chunk+k]);
						}
					}
					for( ; j < ld ; ++j){
						for(size_t k = 0 ; k < chunk ; ++k){
							y[i*chunk+k] += dat[start+j*chunk+k]*x[col[start+j*chunk+k]];
						}
					}
					for(size_t k = 0 ; k < chunk ; ++k){
						F.init(y[i*chunk+k], y[i*chunk+k]);
					}
				}
			}
#endif
		}

		template<class It>
		struct rowInfos
		{
			It begin;
			uint32_t size = 0;
			uint32_t idx = 0;
			rowInfos(It b, uint32_t n, uint32_t i) : begin(b), size(n), idx(idx){}
			It begin() {return begin;}
			It end() {return begin+size;}
		};

	} // details

	/* ******* */
	/* SELL_sub */
	/* ******* */

	// y = A x + b y ; (generic)
	// it is supposed that no reduction is needed.
	template<class Field>
	inline void fspmv(
		      const Field& F,
		      const SELL_sub<typename Field::Element, simd_true> & A,
		      const VECT<typename Field::Element> & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element> & y
		     )
	{
		sp_spmv(F, A, x, b, y, FieldTraits<Field>::value);
	}

	template<class Field>
	inline void fspmv(const Field & F, const ELL_sub<typename Field::Element> & A, const VECT<typename Field::Element> & x, const typename Field::Element & b, VECT<typename Field::Element> & y, FieldCategories::ModularFloatingPointTag)
	{
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::ModularFloatingPointTag());
	}

	template<class Field>
	inline void fspmv(const Field & F, const ELL_sub<typename Field::Element> & A, const VECT<typename Field::Element> & x, const typename Field::Element & b, VECT<typename Field::Element> & y, FieldCategories::FloatingPointTag)
	{
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::FloatingPointTag());
		finit(F,A.m,y.dat,1);
	}

	/* ******* */
	/* SELL_ZO */
	/* ******* */

	template<class Field>
	inline void fspmv(
		      const Field& F,
		      const SELL_ZO<typename Field::Element, simd_true> & A,
		      const VECT<typename Field::Element> & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element> & y
		     )
	{
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldTraits<Field>::value);
	}

	template<class Field>
	inline void fspmv(const Field & F, const ELL_ZO<typename Field::Element> & A, const VECT<typename Field::Element> & x, const typename Field::Element & b, VECT<typename Field::Element> & y, FieldCategories::ModularFloatingPointTag)
	{
		details::fspmv_zo(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::ModularFloatingPointTag());
		fscalin(F, A.m, A.cst, y.dat, 1);
	}

	template<class Field>
	inline void fspmv(const Field & F, const ELL_ZO<typename Field::Element> & A, const VECT<typename Field::Element> & x, const typename Field::Element & b, VECT<typename Field::Element> & y, FieldCategories::FloatingPointTag)
	{
		details::fspmv_zo(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::FloatingPointTag());
		finit(F,A.m,y.dat,1);
		fscalin(F, A.m, A.cst, y.dat, 1);
	}

	/* *** */
	/* ELL */
	/* *** */

	// y = A x + b y ; (generic)
	// reductions are delayed.
	template<class Field>
	void fspmv(
		      const Field& F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<typename Field::Element> & A,
		      const VECT<typename Field::Element> & x,
		      const typename Field::Element & b,
		      VECT<typename Field::Element> & y
		     )
	{
		details::init_y(F, A.m, b, y, FieldTraits<Field>::value);
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldTraits<Field>::value);
	}

	void fspmv(
		      const DoubleDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		details::init_y(F, A.m, b, y, FieldCategories::FloatingPointTag());
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::FloatingPointTag());
	}

	void fspmv(
		      const FloatDomain & F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<float> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		details::init_y(F, A.m, b, y, FieldCategories::FloatingPointTag());
		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat, FieldCategories::FloatingPointTag());
	}

	void fspmv(
		      const FFPACK::Modular<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		// std::cout << "Modular Double ELL" << std::endl;
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one) ;

		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat,(index_t) kmax);
	}

	void fspmv(
		      const FFPACK::ModularBalanced<double>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<double> & A,
		      const VECT<double> & x,
		      const double & b,
		      VECT<double> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one) ;

		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat,(index_t) kmax);
	}

	void fspmv(
		      const FFPACK::Modular<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<float, simd_true> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one) ;

		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat,(index_t)kmax);
	}

	void fspmv(
		      const FFPACK::ModularBalanced<float>& F,
		      // const FFLAS_TRANSPOSE tA,
		      const SELL<float, simd_true> & A,
		      const VECT<float> & x,
		      const float & b,
		      VECT<float> & y
		     )
	{
		fscalin(F,A.m,b,y.dat,1);
		size_t kmax = Protected::DotProdBoundClassic(F,F.one) ;

		details::fspmv(F,A.m,A.n,A.chunk,A.nChunks,A.col,A.ptr,A.chs,A.dat,x.dat,y.dat,(index_t) kmax);
	}

	template<class Field, class ColT, class RowT>
	inline void sp_sell_from_csr()
	{
		std::vector<details::rowInfos<uint64_t>> infos;
		for(size_t i = 0 ; i < CSR_m ; ++i){
			infos.emplace_back(CSR_row[i], CSR_row[i+1]-CSR_row[i], i);
		}
		if(CSR_m%chunk != 0)
		{
			for(size_t i = 0 ; i < CSR_m%chunk ; ++i){
				infos.emplace_back(0, 0, 0);
			}
		}

		nChunks = infos.size()/chunk;

		std::sort(infos.begin(), infos.end(), [](rowInfos & a, rowInfos & b){
			return a.size < b.size;
		});
		for(size_t i = 0 ; i < CSR_m ; ++i){
			perm[i] = infos[i].idx;
		}
		std::vector<size_t> max(nChunks);
		for(size_t i = 0, end = infos.size() ; i < end ; i+=chunk){
			max[i] = *(std::max_element(infos.begin()+i, infos.begin()+i+chunk, [](rowInfos & a, rowInfos & b){
				return a.size < b.size;
			}));
		}
		size_t alloc_size = std::accumulate(max.begin(), max.end(),0)*chunk;
		if(!ZO){
			SELL_dat = fflas_new<typename Field::Element>(alloc_size, Alignment::CACHE_LINE);
		}
		SELL_col = = fflas_new<uint32_t>(alloc_size, Alignment::CACHE_LINE);
		size_t acc = 0;
		for(size_t i = 0 ; i < nChunks ; ++i){
			for(size_t j = 0 ; j < max[i] ; ++j){
				acc += max[i]*chunk;
				ptr[i] = acc;
				for(size_t k = 0 ; k < chunk ; ++k){
					if(i*chunk+k < CSR_m){
						if(j < CSR_row[i*chunk+k]){
							if(!ZO){
								SELL_dat[acc+j*chunk+k] = CSR_dat[infos[i*chunk+k].begin+j];
							}
							SELL_col[acc+j*chunk+k] = CSR_col[infos[i*chunk+k].begin+j];
						}
						else{
							if(!ZO){
								SELL_dat[acc+j*chunk+k] = 0;
							}
							SELL_col[acc+j*chunk+k] = 0;
						}
					}else{
						if(!ZO){
							SELL_dat[acc+j*chunk+k] = 0;
						}
						SELL_col[acc+j*chunk+k] = 0;
					}
				}
			}
		}
	}

} /* FFLAS */

#endif // __FFLASFFPACK_fflas_fflas_spmv_sell_INL
