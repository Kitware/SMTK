/*
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 * 
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 */

/**\file Matrix3.hpp
 *\author Jason Kraftcheck (kraftche@cae.wisc.edu)
 *\date 2006-07-18
 *\date 2012-08-2 Updated by rhl to be more generic. less code that does more!
 * TODO: Remove all 'inline' keywords as it is only a suggestion to the compiler
 * anyways, and it will ignore it or add it when it thinks its necessary.
 *\date 2016-08-03 Updated to use Eigen3 support underneath to improve performance
 */

#ifndef MOAB_MATRIX3_HPP
#define MOAB_MATRIX3_HPP

#include <iostream>
#include <iosfwd>
#include <limits>
#include <cmath>
#include <cassert>

#include "moab/Util.hpp"
#include "moab/Types.hpp"
#include "moab/CartVect.hpp"

#ifdef __GNUC__
// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wshadow"
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#define EIGEN_DEFAULT_TO_ROW_MAJOR
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
// #define EIGEN_NO_STATIC_ASSERT
#include "moab/Eigen/Dense"

#ifdef __GNUC__
// turn the warnings back on
#pragma GCC diagnostic pop
#endif

namespace moab {

namespace Matrix{
	template< typename Matrix>
	Matrix inverse( const Matrix & d, const double det){
		Matrix m( d);
		m(0) = det * (d(4) * d(8) - d(5) * d(7));
    m(1) = det * (d(2) * d(7) - d(8) * d(1));
    m(2) = det * (d(1) * d(5) - d(4) * d(2));
    m(3) = det * (d(5) * d(6) - d(8) * d(3));
    m(4) = det * (d(0) * d(8) - d(6) * d(2));
    m(5) = det * (d(2) * d(3) - d(5) * d(0));
    m(6) = det * (d(3) * d(7) - d(6) * d(4));
    m(7) = det * (d(1) * d(6) - d(7) * d(0));
    m(8) = det * (d(0) * d(4) - d(3) * d(1));
		return m;
	}

	template< typename Matrix>
	inline bool positive_definite( const Matrix & d, 
				       double& det ){
	        double subdet6 = d(1)*d(5)-d(2)*d(4);
	        double subdet7 = d(2)*d(3)-d(0)*d(5);
	        double subdet8 = d(0)*d(4)-d(1)*d(3);
	        det = d(6)*subdet6 + d(7)*subdet7 + d(8)*subdet8;
	        return d(0) > 0 && subdet8 > 0 && det > 0;
	}

	template< typename Matrix>
	inline Matrix mmult3( const Matrix& a, const Matrix& b ) {
	  return Matrix( a(0,0) * b(0,0) + a(0,1) * b(1,0) + a(0,2) * b(2,0),
	                 a(0,0) * b(0,1) + a(0,1) * b(1,1) + a(0,2) * b(2,1),
	                 a(0,0) * b(0,2) + a(0,1) * b(1,2) + a(0,2) * b(2,2),
	                 a(1,0) * b(0,0) + a(1,1) * b(1,0) + a(1,2) * b(2,0),
	                 a(1,0) * b(0,1) + a(1,1) * b(1,1) + a(1,2) * b(2,1),
	                 a(1,0) * b(0,2) + a(1,1) * b(1,2) + a(1,2) * b(2,2),
	                 a(2,0) * b(0,0) + a(2,1) * b(1,0) + a(2,2) * b(2,0),
	                 a(2,0) * b(0,1) + a(2,1) * b(1,1) + a(2,2) * b(2,1),
	                 a(2,0) * b(0,2) + a(2,1) * b(1,2) + a(2,2) * b(2,2) );
	}

	template< typename Vector, typename Matrix>
	inline Matrix outer_product( const Vector & u,
	                              const Vector & v,
				      Matrix & m ) {
	  	m = Matrix( u[0] * v[0], u[0] * v[1], u[0] * v[2],
	                    u[1] * v[0], u[1] * v[1], u[1] * v[2],
	                    u[2] * v[0], u[2] * v[1], u[2] * v[2] );
		return m;
	}
	template< typename Matrix>
	inline double determinant3( const Matrix & d){
		return (d(0) * d(4) * d(8) 
		     + d(1) * d(5) * d(6)
		     + d(2) * d(3) * d(7)
		     - d(0) * d(5) * d(7)
		     - d(1) * d(3) * d(8)
		     - d(2) * d(4) * d(6)); 
	}
	
	template< typename Matrix>
	inline const Matrix inverse( const Matrix & d){
		const double det = 1.0/determinant3( d);
		return inverse( d, det);
	}
	
	template< typename Vector, typename Matrix>
	inline Vector vector_matrix( const Vector& v, const Matrix& m ) {
	  return Vector( v[0] * m(0,0) + v[1] * m(1,0) + v[2] * m(2,0),
	                 v[0] * m(0,1) + v[1] * m(1,1) + v[2] * m(2,1),
	                 v[0] * m(0,2) + v[1] * m(1,2) + v[2] * m(2,2) );
	}
	
	template< typename Vector, typename Matrix>
	inline Vector matrix_vector( const Matrix& m, const Vector& v ){
	   Vector res = v;
	   res[ 0] = v[0] * m(0,0) + v[1] * m(0,1) + v[2] * m(0,2);
	   res[ 1] = v[0] * m(1,0) + v[1] * m(1,1) + v[2] * m(1,2);
	   res[ 2] = v[0] * m(2,0) + v[1] * m(2,1) + v[2] * m(2,2);
	   return res;
	} 

} //namespace Matrix

class Matrix3  {
  Eigen::Matrix3d _mat;

public:
  //Default Constructor
  inline Matrix3() {
    _mat.fill(0.0);
  }
  inline Matrix3(Eigen::Matrix3d mat) : _mat(mat) {
  }
  //TODO: Deprecate this.
  //Then we can go from three Constructors to one. 
  inline Matrix3( double diagonal ) {
    _mat << diagonal, 0.0, 0.0,
            0.0, diagonal, 0.0,
            0.0, 0.0, diagonal;
  }
  inline Matrix3( const CartVect & diagonal ) {
    _mat << diagonal[0], 0.0, 0.0,
            0.0, diagonal[1], 0.0,
            0.0, 0.0, diagonal[2];
  }
  //TODO: not strictly correct as the Matrix3 object
  //is a double d[ 9] so the only valid model of T is
  //double, or any refinement (int, float) 
  //*but* it doesn't really matter anything else
  //will fail to compile.
  inline Matrix3( const std::vector<double> & diagonal ) { 
    _mat << diagonal[0], 0.0, 0.0,
            0.0, diagonal[1], 0.0,
            0.0, 0.0, diagonal[2];
  }

  inline Matrix3( double v00, double v01, double v02,
                double v10, double v11, double v12,
                double v20, double v21, double v22 ) {
    _mat << v00, v01, v02,
            v10, v11, v12,
            v20, v21, v22;
  }

  //Copy constructor 
  Matrix3 ( const Matrix3 & f) : _mat(f._mat) {}

  //Weird constructors 
  template< typename Vector> 
  inline Matrix3(   const Vector & row0,
                    const Vector & row1,
                    const Vector & row2,
                    const bool isRow) {
    if (isRow) {
      _mat << row0[0], row0[1], row0[2],
              row1[0], row1[1], row1[2],
              row2[0], row2[1], row2[2];
    }
    else {
      _mat << row0[0], row1[0], row2[0],
              row0[1], row1[1], row2[1],
              row0[2], row1[2], row2[2];
    }
  }
#ifndef DEPRECATED
  #ifdef __GNUC__
    #define DEPRECATED __attribute__((deprecated))
  #else
    #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
    #define DEPRECATED
  #endif
#endif
  /*
   * \deprecated { Use instead the constructor with explicit fourth argument, bool isRow, above }
   *
   */
  template< typename Vector>
  inline DEPRECATED Matrix3(   const Vector & row0,
                    const Vector & row1,
                    const Vector & row2)
  {
    _mat << row0[0], row0[1], row0[2],
            row1[0], row1[1], row1[2],
            row2[0], row2[1], row2[2];
  }


  inline Matrix3( const double v[9] ){ 
    _mat << v[0], v[1], v[2],
            v[3], v[4], v[5],
            v[6], v[7], v[8];
  }

  
  inline Matrix3& operator=( const Matrix3& m ){
    _mat = m._mat;
    return *this;
  }
  
  inline Matrix3& operator=( const double v[9] ){ 
    _mat << v[0], v[1], v[2],
            v[3], v[4], v[5],
            v[6], v[7], v[8];
    return *this;
 }

  inline double* operator[]( unsigned i ){ return _mat.row(i).data(); }
  inline const double* operator[]( unsigned i ) const{ return _mat.row(i).data(); }
  inline double& operator()(unsigned r, unsigned c) { return _mat(r,c); }
  inline double operator()(unsigned r, unsigned c) const { return _mat(r,c); }
  inline double& operator()(unsigned i) { return _mat(i); }
  inline double operator()(unsigned i) const { return _mat(i); }
  
    // get pointer to array of nine doubles
  inline double* array()
      { return _mat.data(); }
  inline const double* array() const
      { return _mat.data(); }

  inline Matrix3& operator+=( const Matrix3& m ){
    _mat += m._mat;
    return *this;
  }
  
  inline Matrix3& operator-=( const Matrix3& m ){
    _mat -= m._mat;
    return *this;
  }
  
  inline Matrix3& operator*=( double s ){
    _mat *= s;
    return *this;
  }

  inline Matrix3& operator/=( double s ){
    _mat /= s;
    return *this;
  }

  inline Matrix3& operator*=( const Matrix3& m ){
	  _mat *= m._mat;
	  return *this;
  }

  inline ErrorCode eigen_decomposition(Eigen::Vector3d& evals, Matrix3& evecs)
  {
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigensolver(this->_mat);
    if (eigensolver.info() != Eigen::Success)
      return MB_FAILURE;
    evals = eigensolver.eigenvalues();
    evecs._mat = eigensolver.eigenvectors(); //.col(1)
    return MB_SUCCESS;
  }

  inline ErrorCode eigen_decomposition(CartVect& evals, Matrix3& evecs)
  {
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigensolver(this->_mat);
    if (eigensolver.info() != Eigen::Success)
      return MB_FAILURE;
    Eigen::Vector3d _evals = eigensolver.eigenvalues();
    evals[0] = _evals[0];
    evals[1] = _evals[1];
    evals[2] = _evals[2];
    evecs._mat = eigensolver.eigenvectors(); //.col(1)
    return MB_SUCCESS;
  }

  inline void transpose_inplace()
  {
    return _mat.transposeInPlace();
  }

  inline Matrix3 transpose() const
  {
    return Matrix3( _mat.transpose() );
  }

  inline void swapcol(int index, Eigen::Vector3d& vol) {
  	_mat.col(index).swap(vol);
  }

  inline void swapcol(int srcindex, Matrix3& vol, int destindex) {
  	_mat.col(srcindex).swap(vol._mat.col(destindex));
  }

  inline void swapcol(int srcindex, int destindex) {
  	_mat.col(srcindex).swap(_mat.col(destindex));
  }

 inline Eigen::Vector3d vcol(int index) const {
  	return _mat.col(index);
  }

 inline void colscale(int index, double scale) {
  	_mat.col(index) *= scale;
  }

 inline void rowscale(int index, double scale) {
  	_mat.row(index) *= scale;
  }

  inline CartVect col(int index) const{
    Eigen::Vector3d mvec = _mat.col(index);
  	return CartVect(mvec[0], mvec[1], mvec[2]);
  }

  inline CartVect row(int index) const{
    Eigen::Vector3d mvec = _mat.row(index);
  	return CartVect(mvec[0], mvec[1], mvec[2]);
  }

  friend Matrix3 operator+( const Matrix3& a, const Matrix3& b );
  friend Matrix3 operator-( const Matrix3& a, const Matrix3& b );
  friend Matrix3 operator*( const Matrix3& a, const Matrix3& b );

  inline double determinant() const{
  	return _mat.determinant();
  }
 
  inline Matrix3 inverse() const {
    return Matrix3(_mat.inverse());
  }
 
  inline bool invert() {
    Eigen::Matrix3d invMat;
    bool invertible;
    double d_determinant;
    _mat.computeInverseAndDetWithCheck(invMat, d_determinant, invertible);
    if (!Util::is_finite(d_determinant))
      return false;
    _mat = invMat;
    return invertible;
  }
    // Calculate determinant of 2x2 submatrix composed of the
    // elements not in the passed row or column.
  inline double subdet( int r, int c ) const{
    const int r1 = (r+1)%3, r2 = (r+2)%3;
    const int c1 = (c+1)%3, c2 = (c+2)%3;
    assert(r >= 0 && c >= 0);
    if (r < 0 || c < 0) return DBL_MAX;
    return _mat(r1,c1)*_mat(r2,c2) - _mat(r1,c2)*_mat(r2,c1);
  }
}; //class Matrix3



template< typename Vector>
inline Matrix3 outer_product( const Vector & u,
                              const Vector & v ) {
  return Matrix3( u[0] * v[0], u[0] * v[1], u[0] * v[2],
                  u[1] * v[0], u[1] * v[1], u[1] * v[2],
                  u[2] * v[0], u[2] * v[1], u[2] * v[2] );
}

inline Matrix3 operator+( const Matrix3& a, const Matrix3& b ) {
  return Matrix3(a._mat + b._mat);
}

inline Matrix3 operator-( const Matrix3& a, const Matrix3& b ){ 
  return Matrix3(a._mat - b._mat);
}

inline Matrix3 operator*( const Matrix3& a, const Matrix3& b ) {
  return Matrix3(a._mat * b._mat);
}

template< typename T>
inline std::vector< T> operator*( const Matrix3&m, const std::vector< T> & v){
		return moab::Matrix::matrix_vector( m, v);
}

template< typename T>
inline std::vector< T> operator*( const std::vector< T>& v, const Matrix3&m){
		return moab::Matrix::vector_matrix( v, m);
}

inline CartVect operator*( const Matrix3&m,  const CartVect& v){
		return moab::Matrix::matrix_vector( m, v);
}

inline CartVect operator*( const CartVect& v, const Matrix3& m){
		return moab::Matrix::vector_matrix( v, m);
}

} // namespace moab

#ifndef MOAB_MATRIX3_OPERATORLESS
#define MOAB_MATRIX3_OPERATORLESS
inline std::ostream& operator<<( std::ostream& s, const moab::Matrix3& m ){
  return s <<  "| " << m(0,0) << " " << m(0,1) << " " << m(0,2) 
           << " | " << m(1,0) << " " << m(1,1) << " " << m(1,2) 
           << " | " << m(2,0) << " " << m(2,1) << " " << m(2,2) 
           << " |" ;
}
#endif//MOAB_MATRIX3_OPERATORLESS
#endif //MOAB_MATRIX3_HPP
