#define BOOST_TEST_MODULE HyperSlabBlockIterator 
#include <boost/test/unit_test.hpp>

#include <xdm/HyperSlabBlockIterator.hpp>

#include <algorithm>

namespace {

BOOST_AUTO_TEST_CASE( constructSmall ) {
  // construct a 5x5 slab
  xdm::HyperSlab<> total( xdm::makeShape( 5, 5 ) );
  std::fill( total.beginStart(), total.endStart(), 0 );
  std::fill( total.beginStride(), total.endStride(), 1 );
  std::fill( total.beginCount(), total.endCount(), 5 );

  // construct a block iterator with size 2x6
  xdm::DataShape<> blockSize = xdm::makeShape( 2, 6 );
  xdm::HyperSlabBlockIterator<> test( total, blockSize );

  BOOST_CHECK_EQUAL( 2, test->shape().rank() );
  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 0, test->start(1) );
  BOOST_CHECK_EQUAL( 1, test->stride(0) );
  BOOST_CHECK_EQUAL( 1, test->stride(1) );
  BOOST_CHECK_EQUAL( 2, test->count(0) );
  BOOST_CHECK_EQUAL( 5, test->count(1) );
}

BOOST_AUTO_TEST_CASE( iterate ) {
  // construct a 5x5 slab
  xdm::HyperSlab<> total( xdm::makeShape( 5, 5 ) );
  std::fill( total.beginStart(), total.endStart(), 0 );
  std::fill( total.beginStride(), total.endStride(), 1 );
  std::fill( total.beginCount(), total.endCount(), 5 );

  // construct a block iterator with size 3x2
  xdm::DataShape<> blockSize = xdm::makeShape( 3, 2 );
  xdm::HyperSlabBlockIterator<> test( total, blockSize );

  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 0, test->start(1) );
  BOOST_CHECK_EQUAL( 3, test->count(0) );
  BOOST_CHECK_EQUAL( 2, test->count(1) );

  ++test;
  BOOST_CHECK_EQUAL( 3, test->start(0) );
  BOOST_CHECK_EQUAL( 0, test->start(1) );
  BOOST_CHECK_EQUAL( 2, test->count(0) );
  BOOST_CHECK_EQUAL( 2, test->count(1) );
  
  ++test;
  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 2, test->start(1) );
  BOOST_CHECK_EQUAL( 3, test->count(0) );
  BOOST_CHECK_EQUAL( 2, test->count(1) );
  
  ++test;
  BOOST_CHECK_EQUAL( 3, test->start(0) );
  BOOST_CHECK_EQUAL( 2, test->start(1) );
  BOOST_CHECK_EQUAL( 2, test->count(0) );
  BOOST_CHECK_EQUAL( 2, test->count(1) );

  ++test;
  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 4, test->start(1) );
  BOOST_CHECK_EQUAL( 3, test->count(0) );
  BOOST_CHECK_EQUAL( 1, test->count(1) );
  
  ++test;
  BOOST_CHECK_EQUAL( 3, test->start(0) );
  BOOST_CHECK_EQUAL( 4, test->start(1) );
  BOOST_CHECK_EQUAL( 2, test->count(0) );
  BOOST_CHECK_EQUAL( 1, test->count(1) );

  ++test;
  BOOST_CHECK( test == xdm::HyperSlabBlockIterator<>() );
}

BOOST_AUTO_TEST_CASE( iterateStride ) {
  // construct a 5x5 slab with stride 2
  xdm::HyperSlab<> total( xdm::makeShape( 5, 5 ) );
  std::fill( total.beginStart(), total.endStart(), 0 );
  std::fill( total.beginStride(), total.endStride(), 2 );
  std::fill( total.beginCount(), total.endCount(), 3 );

  // construct a block iterator with size 3x2
  xdm::DataShape<> blockSize = xdm::makeShape( 3, 2 );
  xdm::HyperSlabBlockIterator<> test( total, blockSize );

  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 0, test->start(1) );
  BOOST_CHECK_EQUAL( 3, test->count(0) );
  BOOST_CHECK_EQUAL( 2, test->count(1) );
  
  ++test;
  BOOST_CHECK_EQUAL( 0, test->start(0) );
  BOOST_CHECK_EQUAL( 4, test->start(1) );
  BOOST_CHECK_EQUAL( 3, test->count(0) );
  BOOST_CHECK_EQUAL( 1, test->count(1) );

  ++test;
  BOOST_CHECK( test == xdm::HyperSlabBlockIterator<>() );
}

} // namespace

