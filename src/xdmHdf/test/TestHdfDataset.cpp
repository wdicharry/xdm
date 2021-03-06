//==============================================================================
// This software developed by Stellar Science Ltd Co and the U.S. Government.  
// Copyright (C) 2009 Stellar Science. Government-purpose rights granted.      
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
#define BOOST_TEST_MODULE TestHdfDataset
#include <boost/test/unit_test.hpp>

#include <xdm/DataSelection.hpp>
#include <xdm/FileSystem.hpp>
#include <xdm/StructuredArray.hpp>
#include <xdm/VectorStructuredArray.hpp>
#include <xdm/RefPtr.hpp>

#include <xdmHdf/FileIdentifierRegistry.hpp>
#include <xdmHdf/HdfDataset.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <cstdlib>

namespace {

BOOST_AUTO_TEST_CASE( roundtrip ) {
  const char * kDatasetFile = "HdfDataset.h5";

  xdm::remove( xdm::FileSystemPath( kDatasetFile ) );
  
  // set up the input data
  xdm::VectorStructuredArray< int > data(16);
  srand( 42 );
  std::generate( data.begin(), data.end(), &rand );
  
  // put the write code in its own scope so we can ensure that the file is
  // actually closed and reopened later.
  {
    
    // initialize the dataset on disk
    xdm::DataShape<> fileshape( 2 );
    fileshape[0] = 4;
    fileshape[1] = 4;

    // create the dataset
    xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset() );
    dataset->setFile( kDatasetFile );
    dataset->setDataset( "testdata" );

    // write the data to disk
    dataset->initialize( xdm::primitiveType::kInt, fileshape,
      xdm::Dataset::kCreate );
    dataset->serialize( &data, xdm::DataSelectionMap() );
    dataset->finalize();
  
  } // end of writing
  
  // make sure that all files are closed
  xdmHdf::FileIdentifierRegistry::instance()->closeAllIdentifiers();

  xdm::VectorStructuredArray< int > result( 16 );
  { // begin reading

    // specify the expected size of the dataset on disk
    xdm::DataShape<> fileshape( 2 );
    fileshape[0] = 4;
    fileshape[1] = 4;

    // open the dataset
    xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset() );
    dataset->setFile( kDatasetFile );
    dataset->setDataset( "testdata" );

    // read the data from disk into the array
    dataset->initialize( xdm::primitiveType::kInt, fileshape,
      xdm::Dataset::kRead );
    dataset->deserialize( &result, xdm::DataSelectionMap() );
    dataset->finalize();

  } // end of reading

  // make sure the result data matches the input data
  BOOST_CHECK_EQUAL_COLLECTIONS( 
    result.begin(), result.end(), 
    data.begin(), data.end() );
}

BOOST_AUTO_TEST_CASE( compression ) {
  const char * kUncompressedFile = "Uncompressed.h5";
  const char * kCompressedFile = "Iscompressed.h5";

  xdm::remove( xdm::FileSystemPath( kUncompressedFile ) );
  xdm::remove( xdm::FileSystemPath( kCompressedFile ) );

  const size_t kLength = 1 << 20;
  xdm::VectorStructuredArray< int > data( kLength );
  std::fill( data.begin(), data.end(), 0 );

  // write the data uncompressed with chunking.
  {
    xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset(
      kUncompressedFile, xdmHdf::GroupPath(), "Data" ) );
    dataset->setUseChunkedIo( true );
    dataset->initialize(
      xdm::primitiveType::kInt,
      xdm::makeShape( kLength ),
      xdm::Dataset::kCreate );
    dataset->serialize( &data, xdm::DataSelectionMap() );
    dataset->finalize();
  }

  // write the same data compressed with chunking.
  {
    xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset(
      kCompressedFile, xdmHdf::GroupPath(), "Data" ) );
    dataset->setUseChunkedIo( true );
    dataset->setUseCompression( true );
    dataset->initialize(
      xdm::primitiveType::kInt,
      xdm::makeShape( kLength ),
      xdm::Dataset::kCreate );
    dataset->serialize( &data, xdm::DataSelectionMap() );
    dataset->finalize();
  }

  size_t uncompressedSize = xdm::size( xdm::FileSystemPath( kUncompressedFile ) );
  size_t compressedSize = xdm::size( xdm::FileSystemPath( kCompressedFile ) );
  // it's all zeroes, so compression should be at least factor of 2.
  BOOST_CHECK_LT( compressedSize, uncompressedSize / 2 );
}

BOOST_AUTO_TEST_CASE( typeConversion ) {
  // Make sure a dataset written out as one type can be read in as another.
  char const * const kFile = "typeConversion.h5";
  const size_t kLength = 1;

  xdm::remove( xdm::FileSystemPath( kFile ) );

  // Write out floats.
  {
    xdm::VectorStructuredArray< float > floatData( kLength );
    std::fill( floatData.begin(), floatData.end(), 42.0f );
    xdmHdf::HdfDataset dataset;
    dataset.setFile( kFile );
    dataset.setDataset( "Data" );
    dataset.initialize( 
      xdm::primitiveType::kFloat,
      xdm::makeShape( kLength ),
      xdm::Dataset::kCreate );
    dataset.serialize( &floatData, xdm::DataSelectionMap() );
    dataset.finalize();
  }

  // Read in doubles.
  {
    xdm::VectorStructuredArray< double > doubleData( kLength );
    xdmHdf::HdfDataset dataset;
    dataset.setFile( kFile );
    dataset.setDataset( "Data" );
    dataset.initialize(
      xdm::primitiveType::kFloat,
      xdm::makeShape( kLength ),
      xdm::Dataset::kRead );
    dataset.deserialize( &doubleData, xdm::DataSelectionMap() );
    dataset.finalize();

    BOOST_CHECK_EQUAL( doubleData.dataType(), xdm::primitiveType::kDouble );

    const double answer = 42.0; // double precision.
    double result = doubleData[0];
    BOOST_CHECK_EQUAL( result, answer );
  }
}

} // namespace

