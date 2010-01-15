#ifndef xdmIntegrationTest_FunctionData_hpp
#define xdmIntegrationTest_FunctionData_hpp

#include <xdm/DataShape.hpp>
#include <xdm/HyperSlab.hpp>
#include <xdm/RefPtr.hpp>
#include <xdm/WritableData.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace xdm {
  class StructuredArray;
}

namespace xdmGrid {
  class Attribute;
  class Grid;
}

typedef std::pair< double, double > Bounds;

class GridBounds {
private:
  Bounds mBounds[3];
  xdm::DataShape<> mSize;

public:
  GridBounds( 
    const Bounds& xbounds,
    const Bounds& ybounds,
    const Bounds& zbounds,
    const xdm::DataShape<>& shape ) :
    mSize( shape ) {
      mBounds[0] = xbounds;
      mBounds[1] = ybounds;
      mBounds[2] = zbounds;
    }
  ~GridBounds() {}

  const Bounds& bounds( int dimension ) const { return mBounds[dimension]; }
  xdm::DataShape<>::size_type size( int dimension ) const { 
    return mSize[dimension]; 
  }
  
  const xdm::DataShape<>& shape() const { return mSize; }
  xdm::DataShape<>& shape() { return mSize; }

  double nodeCoordinate( int dimension, int index ) const {
    return mBounds[dimension].first +
      index * ( mBounds[dimension].second - mBounds[dimension].first ) /
      mSize[dimension];
  }

  double cellCoordinate( int dimension, int index ) const {
    return 0.5 * ( 
      nodeCoordinate( dimension, index ) + 
      nodeCoordinate( dimension, index + 1 ) );
  }

  void reverseDimensionOrder() {
    std::reverse( mBounds, mBounds + 3 );
    xdm::reverseDimensionOrder( mSize );
  }
};

inline void reverseDimensionOrder( GridBounds& bounds ) {
  bounds.reverseDimensionOrder();
}

inline const GridBounds& testCaseBounds() {
  static GridBounds result(
    std::make_pair( 0.0, 6.28 ),
    std::make_pair( 0.0, 6.28 ),
    std::make_pair( 0.0, 6.28 ),
    xdm::makeShape( 50, 50, 50 ) );
  return result;
}

typedef std::pair< 
  xdm::RefPtr< xdmGrid::Grid >, 
  xdm::RefPtr< xdmGrid::Attribute >
> ProblemInfo;

ProblemInfo
constructFunctionGrid( const GridBounds& bounds, const std::string& hdfFile );

/// Function object to provide the function to be called in the below
/// FunctionData class.
class Function : public xdm::ReferencedObject {
public:
  virtual ~Function() {}
  virtual double operator()( double x, double y, double z ) = 0;
};

// class that computes the function values given the overall grid bounds and a
// region of interest.
class FunctionData : public xdm::WritableData {
private:
  GridBounds mGrid;
  xdm::HyperSlab<> mRegionOfInterest;
  std::vector< double > mStorage;
  xdm::RefPtr< xdm::StructuredArray > mStructuredArray;
  xdm::RefPtr< Function > mFunction;
  xdm::DataShape<> mBlockSize;

public:
  FunctionData( 
    const GridBounds& grid, 
    const xdm::HyperSlab<>& region,
    Function* function, 
    const xdm::DataShape<>& blockSize = xdm::makeShape( 14, 14, 14 ) );
  virtual ~FunctionData();

  virtual void writeImplementation( xdm::Dataset* dataset );
};

/// Constant function returns a constant value over the grid.
class ConstantFunction : public Function {
private:
  double mValue;

public:
  ConstantFunction( double value ) : mValue( value ) {}
  virtual ~ConstantFunction() {}
  virtual double operator()( double, double, double ) { return mValue; }
};

/// Test case computes sin(x*y*z) on the grid.
class TestCaseFunction : public Function {
public:
  virtual ~TestCaseFunction() {}
  virtual double operator()( double x, double y, double z );
};

#endif // xdmIntegrationTest_FunctionData_hpp
