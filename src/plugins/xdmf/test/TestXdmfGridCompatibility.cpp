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
//-----------------------------------------------------------------------------
// These are tests to verify format compatibility with ParaView's XDMF Library
//-----------------------------------------------------------------------------
#define BOOST_TEST_MODULE XdmfGridCompatibility
#include <boost/test/unit_test.hpp>

#include <xdm/CollectMetadataOperation.hpp>
#include <xdm/ContiguousArray.hpp>
#include <xdm/FileSystem.hpp>
#include <xdm/SerializeDataOperation.hpp>
#include <xdm/StructuredArray.hpp>
#include <xdm/UniformDataItem.hpp>
#include <xdm/ArrayAdapter.hpp>
#include <xdm/XmlObject.hpp>
#include <xdm/XmlOutputStream.hpp>

#include <xdmf/TemporalCollection.hpp>
#include <xdmf/TimeSeries.hpp>
#include <xdmf/VirtualDataset.hpp>
#include <xdmf/XdmfHelpers.hpp>

#include <xdmGrid/CollectionGrid.hpp>
#include <xdmGrid/Domain.hpp>
#include <xdmGrid/UniformGrid.hpp>
#include <xdmGrid/TensorProductGeometry.hpp>
#include <xdmGrid/RectilinearMesh.hpp>
#include <xdmGrid/Time.hpp>

#include <xdmHdf/AttachHdfDatasetOperation.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <cmath>

namespace {

std::vector< double > createGridPoints( int size ) {
  std::vector< double > result( size );
  for ( int i = 0; i < size; ++i ) {
    result[i] = i * ( 6.28 / size );
  }
  return result;
}

std::vector< float > createAttributeData(
  int sizex,
  int sizey,
  int sizez,
  float t = 0.0f) {
  std::vector< float > result( sizex * sizey * sizez );
  for ( int i = 0; i < sizex; ++i ) {
    for ( int j = 0; j < sizey; ++j ) {
      for ( int k = 0; k < sizez; ++k ) {
        int location = k * sizex * sizey + j * sizex + i;
        float x = i * ( 6.28 / sizex );
        float y = j * ( 6.28 / sizey );
        float z = k * ( 6.28 / sizez );
        result[location] = std::sin( x + 2 * y + 4 * z + t );
      }
    }
  }
  return result;
}

BOOST_AUTO_TEST_CASE( staticGrid ) {
  const char * hdfFile = "XdmfCompatibility.staticGrid.h5";

  xdm::remove( xdm::FileSystemPath( hdfFile ) );

  xdm::RefPtr< xdmGrid::Domain > domain( new xdmGrid::Domain );

  // build the grid for the domain
  xdm::RefPtr< xdmGrid::UniformGrid > grid( new xdmGrid::UniformGrid );
  domain->addGrid( grid );

  // build the topology for the grid.
  xdm::RefPtr< xdmGrid::RectilinearMesh > topology(
    new xdmGrid::RectilinearMesh );
  topology->setShape( xdm::makeShape("9 9 9") );
  grid->setTopology( topology );

  // build the geometry for the grid.
  xdm::RefPtr< xdmGrid::TensorProductGeometry > geometry(
    new xdmGrid::TensorProductGeometry( 3 ) );
  grid->setGeometry( geometry );

  // data in memory
  std::vector< double > vertices = createGridPoints( 10 );
  xdm::RefPtr< xdm::StructuredArray > array
    = xdm::createStructuredArray( &vertices[0], 10 );

  // the data item to map the array in memory to the file
  // since the coordinates are the same in x, y, and z we will share the
  // dataitem.
  xdm::RefPtr< xdm::UniformDataItem > sharedData(
    new xdm::UniformDataItem( xdm::primitiveType::kDouble, xdm::makeShape( 10 ) ) );
  sharedData->setData( xdm::makeRefPtr( new xdm::ArrayAdapter( array ) ) );
  for ( int i = 0; i < 3; ++i ) {
    geometry->setCoordinateValues( i, sharedData );
  }

  // add a node centered scalar attribute
  xdm::RefPtr< xdmGrid::Attribute > attribute(
    new xdmGrid::Attribute(
      xdmGrid::Attribute::kScalar,
      xdmGrid::Attribute::kNode ) );
  attribute->setName( "FunctionValues" );
  grid->addAttribute( attribute );

  // create the data for the attribute.
  std::vector< float > data = createAttributeData( 10, 10, 10 );
  xdm::RefPtr< xdm::UniformDataItem > attributeDataItem(
    new xdm::UniformDataItem( xdm::primitiveType::kFloat, xdm::makeShape( "10 10 10" ) ) );
  xdm::RefPtr< xdm::StructuredArray > attrvalues
    = xdm::createStructuredArray( &data[0], 1000 );
  attributeDataItem->setData( xdm::makeRefPtr(
    new xdm::ArrayAdapter( attrvalues ) ) );
  attribute->setDataItem( attributeDataItem );

  // attach an HDF dataset to all UniformDataItems
  xdmHdf::AttachHdfDatasetOperation attachHdfDataset(
    hdfFile, false );
  domain->accept( attachHdfDataset );

  // write the tree's data to disk
  xdm::SerializeDataOperation serializer;
  domain->accept( serializer );

  // collect the tree's metadata
  xdm::CollectMetadataOperation collector;
  domain->accept( collector );
  xdm::RefPtr< xdm::XmlObject > xdmf = xdmf::createXdmfRoot();
  xdmf->appendChild( collector.result() );

  std::ofstream xdmfFile("XdmfGridCompatibility.staticGrid.xmf");
  xdmfFile << *xdmf;
  xdmfFile.close();
}

BOOST_AUTO_TEST_CASE( timeSeries ) {
  const char * hdfFile = "XdmfGridCompatibility.timeSeries.h5";

  xdm::remove( xdm::FileSystemPath( hdfFile ) );

  // write this as both a temporal collection and as a virtual dataset
  xdm::RefPtr< xdmf::TimeSeries > temporalCollection (
    new xdmf::TemporalCollection(
      "XdmfGridCompatibility.temporalCollection.xmf",
      xdm::Dataset::kCreate ) );
  temporalCollection->open();

  // xdm::RefPtr< xdmf::TimeSeries > virtualDataset(
  //  new xdmf::VirtualDataset( "XdmfGridCompatibility.virtualDataset",
  //    xdm::Dataset::kCreate ) );
  // virtualDataset->open();

  // since the geometry and topology of the grid will remain constant throughout
  // the simulation, we make a single object here to share.
  xdm::RefPtr< xdmGrid::TensorProductGeometry > sharedGeometry(
    new xdmGrid::TensorProductGeometry( 3 ) );
  std::vector< double > vertexData = createGridPoints( 10 );
  xdm::RefPtr< xdm::StructuredArray > geometryArray =
    xdm::createStructuredArray( &vertexData[0], 10 );
  xdm::RefPtr< xdmHdf::HdfDataset > geometryDataset( new xdmHdf::HdfDataset );
  geometryDataset->setFile( "XdmfGridCompatibility.timeSeries.h5" );
  geometryDataset->setDataset( "gridValues" );
  xdm::RefPtr< xdm::UniformDataItem > geodata(
    new xdm::UniformDataItem( xdm::primitiveType::kFloat, xdm::makeShape( 10 ) ) );
  geodata->setData( xdm::makeRefPtr( new xdm::ArrayAdapter( geometryArray ) ) );
  geodata->setDataset( geometryDataset );
  for ( int i = 0; i < 3; ++i ) {
    sharedGeometry->setCoordinateValues( i, geodata );
  }

  xdm::RefPtr< xdmGrid::RectilinearMesh > sharedTopology(
    new xdmGrid::RectilinearMesh );
  sharedTopology->setShape( xdm::makeShape( 9, 9, 9 ) );

  double timeval = 0.0;
  for ( unsigned int step = 0; step <= 500; ++step ) {
    timeval += 0.1;

    // create a grid and set it up to use the shared geometry and topology
    xdm::RefPtr< xdmGrid::UniformGrid > grid( new xdmGrid::UniformGrid );
    grid->setTopology( sharedTopology );
    grid->setGeometry( sharedGeometry );

    // construct the time element for this time step.
    xdm::RefPtr< xdmGrid::Time > time( new xdmGrid::Time );
    time->setValue( timeval );
    grid->setTime( time );

    // construct a element centered scalar attribute for this time step.
    xdm::RefPtr< xdmGrid::Attribute > attribute(
      new xdmGrid::Attribute(
        xdmGrid::Attribute::kScalar,
        xdmGrid::Attribute::kElement ) );
    attribute->setName( "FunctionValues" );
    grid->addAttribute( attribute );

    // build the data for the attribute.
    std::vector< float > attrvalues = createAttributeData( 9, 9, 9, timeval );
    xdm::RefPtr< xdm::StructuredArray > attrArray =
      xdm::createStructuredArray(
        &attrvalues[0],
        9*9*9 );
    xdm::RefPtr< xdmHdf::HdfDataset > attrDataset( new xdmHdf::HdfDataset );
    attrDataset->setFile( hdfFile );
    attrDataset->setGroupPath( xdmHdf::GroupPath( 1, "FunctionValues" ) );
    std::stringstream stepStr;
    stepStr << std::setfill('0') << std::setw(5) << step;
    attrDataset->setDataset( stepStr.str() );
    xdm::RefPtr< xdm::UniformDataItem > attrData( new xdm::UniformDataItem(
      xdm::primitiveType::kFloat,
      xdm::makeShape( 9, 9, 9 ) ) );
    attrData->setData( xdm::makeRefPtr( new xdm::ArrayAdapter( attrArray ) ) );
    attrData->setDataset( attrDataset );
    attribute->setDataItem( attrData );

    // write the grid for this step to the TimeSeries files
    xdmf::writeTimestepGrid( temporalCollection, grid, step );
    // xdmf::writeTimestepGrid( virtualDataset, grid, step );
  }
  temporalCollection->close();
  // virtualDataset->close();
}

} // namespace

