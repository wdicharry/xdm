//==============================================================================
// This software developed by Stellar Science Ltd Co and the U.S. Government.
// Copyright (C) 2010 Stellar Science. Government-purpose rights granted.
//
// This file is part of XDM
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#ifndef xdm_VectorRef_hpp
#define xdm_VectorRef_hpp

#include <xdm/RefPtr.hpp>
#include <xdm/ReferencedObject.hpp>

#include <vector>

#include <cassert>

#include <xdm/NamespaceMacro.hpp>

XDM_NAMESPACE_BEGIN

/// The base implementation class held by VectorRefs.
template< typename ValueType >
class VectorRefImpl : public ReferencedObject
{
public:
  /// @param baseIndex The index of the vector in the underlying container of vectors.
  /// @param i The index of an element of the vector at @arg baseIndex, e.g. for an xyz vector,
  ///        i == 1 refers to the y value.
  virtual ValueType& at( std::size_t baseIndex, std::size_t i ) = 0;

  /// @returns The number of elements in this vector.
  virtual std::size_t size() const = 0;
};

/// A VectorRef is a thin object that refers to a set of data and provides vector-like
/// data access semantics with operator[]. It provides a vector-like "view" of a set of
/// data that may or may not be contiguous in memory. This class was designed so that vector
/// operations could be performed on e.g. nodal xyz coordinates without resorting to dimension-
/// by-dimension access in the case where x, y, and z are stored in separate arrays. The class
/// provides a common interface for that case as well as arrays where node coordinates are
/// stored as xyzxyzxyzxyz, and so on.
///
/// Currently, VectorRef ALWAYS refers to persistent data, and there are no implementations
/// of VectorRefImpl that are self-contained. The copy constructor and assignment operator
/// preserve these references so that the reference semantics of the class are preserved. Thus,
/// if a self-contained vector is necessary, the user should copy the data out of the
/// VectorRef into a more suitable vector.
template< typename ValueType >
class VectorRef
{
public:
  /// @param imp The implementation instance that will be used for this vector.
  /// @param index The index of this particular vector in the underlying collection of vectors.
  VectorRef( RefPtr< VectorRefImpl< ValueType > > imp, std::size_t index );

  /// Copy constructor copies reference, so the new vector will keep the reference intact.
  VectorRef( const VectorRef< ValueType >& copyMe );

  /// Operator= copies the reference, so the lhs vector references the same data that the
  /// rhs vector references.
  VectorRef< ValueType >& operator=( const VectorRef< ValueType >& rhs );

  ValueType& operator[]( std::size_t i );

    /// @returns The number of elements in the vector.
    std::size_t size() const;

private:
  RefPtr< VectorRefImpl< ValueType > > mImp;
  std::size_t mIndex;
};

/// The implementation for the trivial case where data is stored in a single array with the xyz
/// index cycling faster than the vector index.
/// Intrinsic state: single contiguous array of data.
/// Extrinsic state: index of specific location.
/// Interlaced arrays xyzxyzxyzxyz...
template< typename ValueType >
class SingleArrayOfVectorsImpl : public VectorRefImpl< ValueType >
{
public:
  SingleArrayOfVectorsImpl( ValueType* xyzArray, std::size_t elementsPerVector );

  /// Assumes the array is laid out contiguously in dimension order.
  virtual ValueType& at( std::size_t baseIndex, std::size_t i );

  virtual std::size_t size() const;

private:
  ValueType* mData;
  std::size_t mSize;
};

/// The implementation for the case where data is stored in multiple arrays where the vector index
/// cycles faster than the xyz index.
/// One array for each coordinate value of the nodes.
/// x1x2x3x4.... y1y2y3y4.... z1z2z3z4....
template< typename ValueType >
class MultipleArraysOfVectorElementsImpl : public VectorRefImpl< ValueType >
{
public:
  MultipleArraysOfVectorElementsImpl( const std::vector< ValueType* >& arrays );

  virtual ValueType& at( std::size_t baseIndex, std::size_t i );

  virtual std::size_t size() const;

private:
  std::vector< ValueType* > mArrays;
  std::size_t mSize;
};

/// Tensor product implementation for structured mesh. This references vectors whose elements are
/// embedded in multiple arrays, like MultipleArraysOfVectorElementsImp, but with the distinction
/// that the underlying data is just a tensor product (it is sparse). The at() function allows
/// apparently dense access (e.g. by node index), but under the hood, the data is stored only on
/// coordinate axes.
template< typename ValueType >
class TensorProductArraysImpl : public VectorRefImpl< ValueType >
{
public:
  TensorProductArraysImpl(
    const std::vector< ValueType* >& coordinateAxisValues,
    const std::vector< std::size_t >& axisSizes );

  virtual ValueType& at( std::size_t baseIndex, std::size_t i );

  virtual std::size_t size() const;

private:
  std::vector< ValueType* > mCoordinateAxisValues;
  std::vector< std::size_t > mAxisSizes;
  std::size_t mSize;
};

//----------------------- Implementations --------------------------------------
template< typename ValueType >
VectorRef< ValueType >::VectorRef(
  RefPtr< VectorRefImpl< ValueType > > imp, std::size_t index ) :
    mImp( imp ),
    mIndex( index )
{
}

template< typename ValueType >
VectorRef< ValueType >::VectorRef( const VectorRef< ValueType >& copyMe ) :
    mImp( copyMe.mImp ),
    mIndex( copyMe.mIndex )
{
}

template< typename ValueType >
VectorRef< ValueType >& VectorRef< ValueType >::operator=(
  const VectorRef< ValueType >& rhs )
{
  if ( &rhs != this ) {
    mImp = rhs.mImp;
    mIndex = rhs.mIndex;
  }
  return *this; // virtual
}

template< typename ValueType >
ValueType& VectorRef< ValueType >::operator[]( std::size_t i )
{
  return mImp->at( mIndex, i ); // virtual
}

template< typename ValueType >
std::size_t VectorRef< ValueType >::size() const
{
  return mImp->size(); // virtual
}

template< typename ValueType >
SingleArrayOfVectorsImpl< ValueType >::SingleArrayOfVectorsImpl( ValueType* xyzArray, std::size_t elementsPerVector ) :
  mData( xyzArray ), mSize( elementsPerVector )
{
}

template< typename ValueType >
ValueType& SingleArrayOfVectorsImpl< ValueType >::at( std::size_t baseIndex, std::size_t i )
{
  return mData[ mSize * baseIndex + i ];
}

template< typename ValueType >
std::size_t SingleArrayOfVectorsImpl< ValueType >::size() const
{
  return mSize;
}

template< typename ValueType >
MultipleArraysOfVectorElementsImpl< ValueType >::MultipleArraysOfVectorElementsImpl(
  const std::vector< ValueType* >& arrays ) :
    mArrays( arrays ), mSize( arrays.size() )
{
}

template< typename ValueType >
ValueType& MultipleArraysOfVectorElementsImpl< ValueType >::at( std::size_t baseIndex, std::size_t i )
{
  return mArrays[i][baseIndex];
}

template< typename ValueType >
std::size_t MultipleArraysOfVectorElementsImpl< ValueType >::size() const
{
  return mSize;
}

template< typename ValueType >
TensorProductArraysImpl< ValueType >::TensorProductArraysImpl(
  const std::vector< ValueType* >& coordinateAxisValues,
  const std::vector< std::size_t >& axisSizes ) :
    mCoordinateAxisValues( coordinateAxisValues ),
    mAxisSizes( axisSizes ),
    mSize( axisSizes.size() )
{
  assert( axisSizes.size() == coordinateAxisValues.size() );
}

template< typename ValueType >
ValueType& TensorProductArraysImpl< ValueType >::at( std::size_t baseIndex, std::size_t i )
{
  // choose a convention for indexing multi-dimension structured data and
  // stick with it. Following XDMF, let's say z is always considered to be
  // the slowest varying dimension, y next, x fastest.
  std::size_t blockSize = mAxisSizes[0];
  std::size_t location[ mSize ];
  location[0] = baseIndex % blockSize;
  for ( int dimension = 1; dimension < mSize; ++dimension ) {
    location[ dimension ] = baseIndex / blockSize;
    blockSize *= mAxisSizes[ dimension ];
  }
  return mCoordinateAxisValues[i][location[i]];
}

template< typename ValueType >
std::size_t TensorProductArraysImpl< ValueType >::size() const
{
  return mSize;
}

XDM_NAMESPACE_END

#endif // xdm_VectorRef_hpp