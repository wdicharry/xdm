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
#define BOOST_TEST_MODULE ImplTreeBuilder
#include <boost/test/unit_test.hpp>

#include <xdmf/impl/TreeBuilder.hpp>
#include <xdmf/impl/XmlDocumentManager.hpp>
#include <xdmf/impl/XPathQuery.hpp>

#include <xdmf/impl/UniformDataItem.hpp>

#include <xdm/AllDataSelection.hpp>
#include <xdm/DataSelectionMap.hpp>
#include <xdm/Dataset.hpp>
#include <xdm/TypedStructuredArray.hpp>
#include <xdm/VectorStructuredArray.hpp>
#include <xdm/UniformDataItem.hpp>

#include <xdmGrid/Attribute.hpp>
#include <xdmGrid/StructuredTopology.hpp>
#include <xdmGrid/TensorProductGeometry.hpp>
#include <xdmGrid/Time.hpp>
#include <xdmGrid/UniformGrid.hpp>
#include <xdmGrid/UnstructuredTopology.hpp>

#include <xdmHdf/HdfDataset.hpp>

#include <libxml/parser.h>
#include <libxml/tree.h>

namespace {

using xdm::RefPtr;
using xdmf::impl::XmlDocumentManager;
using xdmf::impl::SharedNodeVector;

char const * const kTestDatasetFilename = "BuildTreeTest.h5";

// Create a simple test HDF5 file.
void createHdfFile() {
  xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset );
  dataset->setFile( kTestDatasetFilename );
  xdmHdf::GroupPath path;
  path.push_back( "group1" );
  path.push_back( "group2" );
  dataset->setGroupPath( path );
  dataset->setDataset( "dataset" );
  xdm::RefPtr< xdm::VectorStructuredArray< double > > array(
    new xdm::VectorStructuredArray< double >( 9 ) );
  for ( int i = 1; i <= 9; i++ ) {
    (*array)[i-1] = i;
  }

  // write the array
  dataset->initialize(
    xdm::primitiveType::kDouble,
    xdm::makeShape( 3, 3),
    xdm::Dataset::kCreate );
  dataset->serialize( array.get(), xdm::DataSelectionMap() );
  dataset->finalize();
}

RefPtr< XmlDocumentManager > loadXml( const char * xml ) {
  RefPtr< XmlDocumentManager > result( new XmlDocumentManager(
    xmlParseDoc( reinterpret_cast< const xmlChar *>(xml) ) ) );
  return result;
}

BOOST_AUTO_TEST_CASE( findPathToAncestorTest ) {
  char const * const kXml =
    "<grid>"
    "  <topology/>"
    "  <geometry>"
    "    <dataitem/>"
    "    <dataitem/>"
    "  </geometry>"
    "  <attribute name='jim'>"
    "    <dataitem/>"
    "  </attribute>"
    "  <attribute name='jeff'>"
    "    <dataitem/>"
    "  </attribute>"
    "</grid>";

  xmlDocPtr document = xmlParseDoc( (xmlChar*)kXml );
  xmlNode * rootNode = xmlDocGetRootElement( document );

  xdmf::impl::XPathQuery query( document, rootNode, "//dataitem" );
  BOOST_CHECK_EQUAL( query.size(), 4 );

  const char * answer[4] = {
    "geometry[1]/dataitem[1]",
    "geometry[1]/dataitem[2]",
    "attribute[1]/dataitem[1]",
    "attribute[2]/dataitem[1]"
  };

  for ( size_t i = 0; i < query.size(); ++i ) {
    using xdmf::impl::NodePath;
    using xdmf::impl::findPathToAncestor;
    NodePath path = findPathToAncestor( document, query.node(i), rootNode );
    std::reverse( path.begin(), path.end() );
    std::string result = xdmf::impl::makeXPathQuery( path );
    BOOST_CHECK_EQUAL( result, answer[i] );
  }

  xmlFreeDoc( document );
}

BOOST_AUTO_TEST_CASE( buildUniformDataItem ) {
  char const * const kXml =
  "<DataItem Name='test' "
  "  ItemType='Uniform'"
  "  Dimensions='3 3'"
  "  NumberType='Float'"
  "  Precision='8'"
  "  Format='HDF'>"
  "  BuildTreeTest.h5:/group1/group2/dataset"
  "</DataItem>";
  double resultData[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 };

  createHdfFile();

  RefPtr< XmlDocumentManager > document = loadXml( kXml );
  RefPtr< SharedNodeVector > nodes( new SharedNodeVector );
  nodes->push_back( xmlDocGetRootElement( document->get() ) );

  xdmf::impl::TreeBuilder builder( document, nodes );
  xdm::RefPtr< xdm::UniformDataItem > item
    = builder.buildUniformDataItem( nodes->at( 0 ) );
  BOOST_REQUIRE( item );

  BOOST_CHECK_EQUAL( item->dataType(), xdm::primitiveType::kDouble );
  BOOST_CHECK_EQUAL( item->dataspace(), xdm::makeShape( 3, 3 ) );
  BOOST_REQUIRE( item->dataset() );
  BOOST_CHECK_EQUAL( item->dataset()->format(), "HDF" );

  xdm::RefPtr< xdmHdf::HdfDataset > dataset
    = xdm::dynamic_pointer_cast< xdmHdf::HdfDataset >( item->dataset() );
  BOOST_REQUIRE( dataset );

  BOOST_CHECK_EQUAL( dataset->file(), kTestDatasetFilename );
  xdmHdf::GroupPath path;
  path.push_back( "group1" );
  path.push_back( "group2" );
  BOOST_CHECK_EQUAL_COLLECTIONS(
    dataset->groupPath().begin(), dataset->groupPath().end(),
    path.begin(), path.end() );
  BOOST_CHECK_EQUAL( "dataset", dataset->dataset() );

  // check the data
  item->initializeDataset( xdm::Dataset::kRead );
  item->deserializeData();
  item->finalizeDataset();

  xdm::RefPtr< xdm::TypedStructuredArray< double > > array
    = item->typedArray< double >();
  BOOST_REQUIRE( array );
  BOOST_CHECK_EQUAL_COLLECTIONS(
    array->begin(), array->end(),
    resultData, resultData + 9 );
}

BOOST_AUTO_TEST_CASE( buildStructuredTopology ) {
  const char * kXml =
    "<Topology TopologyType='3DRectMesh' Dimensions='3 3 3'/>";

  RefPtr< XmlDocumentManager > document = loadXml( kXml );
  RefPtr< SharedNodeVector > nodes( new SharedNodeVector );
  nodes->push_back( xmlDocGetRootElement( document->get() ) );

  xdmf::impl::TreeBuilder builder( document, nodes );
  xdm::RefPtr< xdmGrid::Topology > result = builder.buildTopology( nodes->at(0) );

  BOOST_REQUIRE( result );

  xdm::RefPtr< xdmGrid::StructuredTopology > structured
    = xdm::dynamic_pointer_cast< xdmGrid::StructuredTopology >( result );
  BOOST_REQUIRE( structured );

  BOOST_CHECK_EQUAL( structured->shape(), xdm::makeShape( 2, 2, 2) );

}

BOOST_AUTO_TEST_CASE( buildUnstructuredTopology ) {
  const char * kXml = 
    "<Topology TopologyType='Tetrahedron' NumberOfElements='6'/>";

  RefPtr< XmlDocumentManager > document = loadXml( kXml );
  RefPtr< SharedNodeVector > nodes( new SharedNodeVector );
  nodes->push_back( xmlDocGetRootElement( document->get() ) );

  xdmf::impl::TreeBuilder builder( document, nodes );
  xdm::RefPtr< xdmGrid::Topology > result = builder.buildTopology( nodes->at(0) );

  BOOST_REQUIRE( result );

  xdm::RefPtr< xdmGrid::UnstructuredTopology > unstructured 
    = xdm::dynamic_pointer_cast< xdmGrid::UnstructuredTopology >( result );
  BOOST_REQUIRE( unstructured );

  xdm::RefPtr< const xdmGrid::ElementTopology > elementTopology =
    unstructured->elementTopology( 0 );
  BOOST_CHECK_EQUAL( elementTopology->numberOfFaces(), 4 );
  BOOST_CHECK_EQUAL( elementTopology->numberOfEdges(), 6 );
  BOOST_CHECK_EQUAL( elementTopology->numberOfNodes(), 4 );
}

BOOST_AUTO_TEST_CASE( buildStaticTree ) {
  typedef xdmf::impl::UniformDataItem InputUDI;

  xmlDocPtr document = xmlParseFile( "test_document1.xmf" );
  BOOST_REQUIRE( document );
  xmlNode * root = xmlDocGetRootElement( document );

  xdmf::impl::XPathQuery gridQuery( document, root, "/Xdmf/Domain/Grid" );
  BOOST_REQUIRE_EQUAL( gridQuery.size(), 1 );

  RefPtr< XmlDocumentManager > doc( new XmlDocumentManager( document ) );
  RefPtr< SharedNodeVector > nodes( new SharedNodeVector );
  nodes->push_back( gridQuery.node( 0 ) );
  xdmf::impl::TreeBuilder builder( doc, nodes );
  xdm::RefPtr< xdm::Item > result = builder.buildTree();
  BOOST_REQUIRE( result );

  xdm::RefPtr< xdmGrid::UniformGrid > grid
    = xdm::dynamic_pointer_cast< xdmGrid::UniformGrid >( result );
  BOOST_REQUIRE( grid );

  // Check the time.
  xdm::RefPtr< const xdmGrid::Time > time = grid->time();
  BOOST_REQUIRE( time );
  BOOST_CHECK_EQUAL( time->value(), 0.0 );

  // Check the topology.
  xdm::RefPtr< xdmGrid::Topology > topology = grid->topology();
  BOOST_REQUIRE( topology );
  xdm::RefPtr< xdmGrid::StructuredTopology > structured =
    xdm::dynamic_pointer_cast< xdmGrid::StructuredTopology >( topology );
  BOOST_REQUIRE( structured );
  // the topology shape should be the reverse of the shape specified in the xmf
  // file since XDMF stores the topology information in ZYX order, but the XDM
  // api chooses the XYZ convention for consistency.
  BOOST_CHECK_EQUAL( structured->shape(), xdm::makeShape( 128, 128, 382 ) );

  // Check the geometry.
  xdm::RefPtr< xdmGrid::Geometry > geometry = grid->geometry();
  BOOST_REQUIRE( geometry );
  xdm::RefPtr< xdmGrid::TensorProductGeometry > tpGeo =
    xdm::dynamic_pointer_cast< xdmGrid::TensorProductGeometry >( geometry );
  BOOST_REQUIRE( tpGeo );
  BOOST_REQUIRE_EQUAL( tpGeo->dimension(), 3 );
  size_t sizes[] = {129, 129, 383};
  const char * paths[3] = {
    "Geometry[1]/DataItem[1]",
    "Geometry[1]/DataItem[2]",
    "Geometry[1]/DataItem[3]"
  };
  for ( int i = 0; i < 3; i++ ) {
    xdm::RefPtr< xdm::UniformDataItem > geoData = tpGeo->child( i );
    BOOST_REQUIRE( geoData );
    BOOST_REQUIRE_EQUAL( geoData->dataspace().rank(), 1 );
    BOOST_CHECK_EQUAL( geoData->dataspace()[0], sizes[i] );
    xdm::RefPtr< InputUDI > inputItem =
      xdm::dynamic_pointer_cast< InputUDI >( geoData );
    BOOST_REQUIRE( inputItem );
    BOOST_CHECK_EQUAL( inputItem->xpathExpr(), paths[i] );
  }

  // Check the attributes.
  BOOST_REQUIRE_EQUAL( grid->numberOfAttributes(), 3 );
  char * names[3] = { "E", "B", "InternalCell" };
  const char * dataPaths[3] = {
    "Attribute[1]/DataItem[1]",
    "Attribute[2]/DataItem[1]",
    "Attribute[3]/DataItem[1]"
  };
  xdmGrid::Attribute::Type types[] = {
    xdmGrid::Attribute::kVector,
    xdmGrid::Attribute::kVector,
    xdmGrid::Attribute::kScalar
  };
  xdmGrid::Attribute::Center centers[] = {
    xdmGrid::Attribute::kElement,
    xdmGrid::Attribute::kElement,
    xdmGrid::Attribute::kElement,
  };
  for ( xdmGrid::Grid::ConstAttributeIterator attr = grid->beginAttributes();
    attr != grid->endAttributes(); ++attr ) {
    int i = attr - grid->beginAttributes();
    BOOST_CHECK_EQUAL( (*attr)->name(), names[i] );
    BOOST_CHECK_EQUAL( (*attr)->dataType(), types[i] );
    BOOST_CHECK_EQUAL( (*attr)->centering(), centers[i] );
    xdm::RefPtr< xdm::UniformDataItem > attrData = (*attr)->dataItem();
    BOOST_REQUIRE( attrData );
    xdm::RefPtr< InputUDI > inputItem =
      xdm::dynamic_pointer_cast< InputUDI >( attrData );
    BOOST_REQUIRE( inputItem );
    BOOST_CHECK_EQUAL( inputItem->xpathExpr(), dataPaths[i] );
  }
}

} // namespace

