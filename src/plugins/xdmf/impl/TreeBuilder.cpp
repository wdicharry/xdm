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

#include <xdmf/impl/TreeBuilder.hpp>

#include <xdmf/impl/XPathQuery.hpp>

#include <xdm/Algorithm.hpp>
#include <xdm/ArrayAdapter.hpp>
#include <xdm/DataShape.hpp>
#include <xdm/Item.hpp>
#include <xdm/UniformDataItem.hpp>
#include <xdm/VectorStructuredArray.hpp>

#include <xdmFormat/Reader.hpp>

#include <xdmGrid/Attribute.hpp>
#include <xdmGrid/CollectionGrid.hpp>
#include <xdmGrid/Geometry.hpp>
#include <xdmGrid/Grid.hpp>
#include <xdmGrid/InterlacedGeometry.hpp>
#include <xdmGrid/MultiArrayGeometry.hpp>
#include <xdmGrid/RectilinearMesh.hpp>
#include <xdmGrid/TensorProductGeometry.hpp>
#include <xdmGrid/Topology.hpp>
#include <xdmGrid/UniformGrid.hpp>

#include <xdmHdf/HdfDataset.hpp>

#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <algorithm>
#include <map>
#include <sstream>

#include <cassert>
#include <cstring>

namespace xdmf {
namespace impl {

namespace {

//-- Tags in the XDMF Specification --//
char const * const kAttributeTag   = "Attribute";
char const * const kDataItemTag    = "DataItem";
char const * const kDomainTag      = "Domain";
char const * const kGeometryTag    = "Geometry";
char const * const kGridTag        = "Grid";
char const * const kInformationTag = "Information";
char const * const kXdmfTag        = "Xdmf";
char const * const kTopologyTag    = "Topology";
char const * const kTimeTag        = "Time";

// Assuming input has been validated, determine type from a string/precision
// pair.
xdm::primitiveType::Value type( const std::string& typeStr, size_t precision ) {
  switch ( precision ) {
  case 1:
    if ( typeStr == "Float" ) return xdm::primitiveType::kFloat;
    if ( typeStr == "Int" ) return xdm::primitiveType::kChar;
    if ( typeStr == "UInt" ) return xdm::primitiveType::kUnsignedChar;
    if ( typeStr == "Char" ) return xdm::primitiveType::kChar;
    break;
  case 4:
    if ( typeStr == "Float" ) return xdm::primitiveType::kFloat;
    if ( typeStr == "Int" ) return xdm::primitiveType::kInt;
    if ( typeStr == "UInt" ) return xdm::primitiveType::kUnsignedInt;
    if ( typeStr == "Char" ) return xdm::primitiveType::kInt;
    break;
  case 8:
    if ( typeStr == "Float" ) return xdm::primitiveType::kDouble;
    if ( typeStr == "Int" ) return xdm::primitiveType::kLongInt;
    if ( typeStr == "UInt" ) return xdm::primitiveType::kLongUnsignedInt;
    if ( typeStr == "Char" ) return xdm::primitiveType::kLongInt;
    break;
  default:
    return xdm::primitiveType::kFloat;
    break;
  }
}

} // namespace

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
TreeBuilder::TreeBuilder( xmlDocPtr document ) :
  mDocument( document ) {
  mXPathContext = xmlXPathNewContext( document );
}

TreeBuilder::~TreeBuilder() {
  xmlXPathFreeContext( mXPathContext );
}

xdm::RefPtr< xdm::Item > TreeBuilder::buildTree() {
  xdm::RefPtr< xdm::Item > result;
  xmlNode * rootNode = xmlDocGetRootElement( mDocument );

  // Search for Grid children of the Domain element.
  XPathQuery gridQuery( mXPathContext, rootNode, "Domain/Grid" );

  if ( gridQuery.size() == 0 ) {
    // If there are no grids, return a NULL item.
    result.reset();
  } else if ( gridQuery.size() == 1 ) {
    // There is one grid.
    result = buildGrid( gridQuery.node( 0 ) );
  } else {
    // There are multiple grids, build a spatial collection grid from them.
    xdm::RefPtr< xdmGrid::CollectionGrid > collection( new xdmGrid::CollectionGrid );
    for ( size_t i = 0; i < gridQuery.size(); i++ ) {
      collection->appendChild( buildGrid( gridQuery.node( i ) ) );
    }
    result = collection;
  }

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdm::UniformDataItem > 
TreeBuilder::buildUniformDataItem( xmlNode * node ) {
  xdm::RefPtr< xdm::UniformDataItem > result( new xdm::UniformDataItem );

  // Get the number type from the NumberType attribute.
  XPathQuery typeQuery( mXPathContext, node, "@NumberType" );
  std::string typeString;
  if ( typeQuery.size() > 0 ) {
    typeString = typeQuery.textValue( 0 );
  } else {
    typeString = "Float";
  }
  XPathQuery precisionQuery( mXPathContext, node, "@Precision" );
  size_t precision;
  if ( precisionQuery.size() > 0 ) {
    precision = precisionQuery.getValue( 0, 4 );
  } else {
    precision = 4;
  }
  xdm::primitiveType::Value dataType = type( typeString, precision );
  result->setDataType( dataType );

  // Get the shape from the Dimensions attribute.
  XPathQuery dimensionsQuery( mXPathContext, node, "@Dimensions" );
  if ( dimensionsQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "No dimensions for a UniformDataItem." ) );
  }
  result->setDataspace( xdm::makeShape( dimensionsQuery.textValue( 0 ) ) );

  // Get the format string for the dataset.
  XPathQuery formatQuery( mXPathContext, node, "@Format" );
  std::string format( "HDF" );
  if ( formatQuery.size() == 0 ) {
    format = formatQuery.textValue( 0 );
  }
  
  // Build the dataset.
  if ( format == "HDF" ) {
    xdm::RefPtr< xdmHdf::HdfDataset > dataset( new xdmHdf::HdfDataset );
    XPathQuery datasetInfoQuery( mXPathContext, node, "text()" );
    if ( datasetInfoQuery.size() == 0 ) {
      XDM_THROW( "No information about requested HDF dataset." );
    }
    std::string datasetInfo = datasetInfoQuery.textValue( 0 );
    std::string fileString;
    xdmHdf::GroupPath groupPath;
    std::string datasetString;
    if ( xdmHdf::parseDatasetInfo( datasetInfo, fileString, groupPath, datasetString ) )
    {
      dataset->setFile( fileString );
      dataset->setGroupPath( groupPath );
      dataset->setDataset( datasetString );
      result->setDataset( dataset );
    } else {
      XDM_THROW( xdmFormat::ReadError( "Invalid HDF dataset specification" ) );
    }
  } else {
    XDM_THROW( xdmFormat::ReadError(
      "Only HDF datasets are supported." ) );
  }

  // Give the item an array to hold the data from the dataset should it be
  // loaded into memory.
  xdm::RefPtr< xdm::StructuredArray > array(
    xdm::makeVectorStructuredArray( dataType ) );
  result->setData( xdm::makeRefPtr( new xdm::ArrayAdapter( array ) ) );

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Geometry > TreeBuilder::buildGeometry( xmlNode * node ) {
  enum {
    XYZ = 0,
    XY,
    X_Y_Z,
    X_Y,
    VXVYVZ,
    VXVY,
    ORIGIN_DXDYDZ,
    ORIGIN_DXDY
  } geometryType( XYZ );

  static const char * strings[] = {
    "XYZ",
    "XY",
    "X_Y_Z",
    "X_Y",
    "VXVYVZ",
    "VXVY",
    "ORIGIN_DXDYDZ",
    "ORIGIN_DXDY"
  };

  // Get the geometry type from the GeometryType attribute.
  XPathQuery geometryTypeQuery( mXPathContext, node, "@GeometryType" );
  if ( geometryTypeQuery.size() > 0 ) {
    std::string typeString = geometryTypeQuery.textValue( 0 );
    xdm::toUpper( typeString );
    if ( typeString == strings[XYZ] ) geometryType = XYZ;
    else if ( typeString == strings[XY] ) geometryType = XY;
    else if ( typeString == strings[X_Y_Z] ) geometryType = X_Y_Z;
    else if ( typeString == strings[X_Y] ) geometryType = X_Y;
    else if ( typeString == strings[VXVYVZ] ) geometryType = VXVYVZ;
    else if ( typeString == strings[VXVY] ) geometryType = VXVY;
    else if ( typeString == strings[ORIGIN_DXDYDZ] ) geometryType = ORIGIN_DXDYDZ;
    else if ( typeString == strings[ORIGIN_DXDY] ) geometryType = ORIGIN_DXDY;
    else XDM_THROW( xdmFormat::ReadError( "Unrecognized XDMF geometry type." ) );
  }

  // Choose the Geometry subclass based on the geometry type flag.
  xdm::RefPtr< xdmGrid::Geometry > result;
  switch ( geometryType ) {
  case XYZ: result = new xdmGrid::InterlacedGeometry( 3 ); break;
  case XY: result = new xdmGrid::InterlacedGeometry( 2 ); break;
  case X_Y_Z: result = new xdmGrid::MultiArrayGeometry( 3 ); break;
  case X_Y: result = new xdmGrid::MultiArrayGeometry( 2 ); break;
  case VXVYVZ: result = new xdmGrid::TensorProductGeometry( 3 ); break;
  case VXVY: result = new xdmGrid::TensorProductGeometry( 2 ); break;
  default: XDM_THROW( "Unsupported XDMF Geometry type." ); break;
  }

  // Read the internal data items that make up the values for the geometry.
  XPathQuery dataQuery( mXPathContext, node, kDataItemTag );
  if ( dataQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Geometry values unspecified." ) );
  }
  for ( int i = 0; i < dataQuery.size(); i++ ) {
    result->appendChild( buildUniformDataItem( dataQuery.node( i ) ) );
  }

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Topology > TreeBuilder::buildTopology( xmlNode * node ) {
  enum {
    POLYVERTEX = 0,
    POLYLINE,
    POLYGON,
    TRIANGLE,
    QUADRILATERAL,
    TETRAHEDRON,
    PYRAMID,
    WEDGE,
    HEXAHEDRON,
    EDGE_3,
    TRIANGLE_6,
    QUADRILATERAL_8,
    TETRAHEDRON_10,
    PYRAMID_13,
    WEDGE_15,
    HEXAHEDRON_20,
    MIXED,
    SMESH2D,
    RECTMESH2D,
    CORECTMESH2D,
    SMESH3D,
    RECTMESH3D,
    CORECTMESH3D
  } topologyType;

  static const char * strings[] = {
    "POLYVERTEX",
    "POLYLINE",
    "POLYGON",
    "TRIANGLE",
    "QUADRILATERAL",
    "TETRAHEDRON",
    "PYRAMID",
    "WEDGE",
    "HEXAHEDRON",
    "EDGE_3",
    "TRIANGLE_6",
    "QUADRILATERAL_8",
    "TETRAHEDRON_10",
    "PYRAMID_13",
    "WEDGE_15",
    "HEXAHEDRON_20",
    "MIXED",
    "2DSMESH",
    "2DRECTMESH",
    "2DCORECTMESH",
    "3DSMESH",
    "3DRECTMESH",
    "3DCORECTMESH"
  };

  // Read the topology type attribute.
  XPathQuery topologyTypeQuery( mXPathContext, node, "@TopologyType" );
  if ( topologyTypeQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "No XDMF topology type specified." ) );
  }
  std::string typeString = topologyTypeQuery.textValue( 0 );
  xdm::toUpper( typeString );
  if ( typeString == strings[POLYVERTEX] ) topologyType = POLYVERTEX;
  else if ( typeString == strings[POLYLINE] ) topologyType = POLYLINE;
  else if ( typeString == strings[POLYGON] ) topologyType = POLYGON;
  else if ( typeString == strings[TRIANGLE] ) topologyType = TRIANGLE;
  else if ( typeString == strings[QUADRILATERAL] ) topologyType = QUADRILATERAL;
  else if ( typeString == strings[TETRAHEDRON] ) topologyType = TETRAHEDRON;
  else if ( typeString == strings[PYRAMID] ) topologyType = PYRAMID;
  else if ( typeString == strings[WEDGE] ) topologyType = WEDGE;
  else if ( typeString == strings[HEXAHEDRON] ) topologyType = HEXAHEDRON;
  else if ( typeString == strings[EDGE_3] ) topologyType = EDGE_3;
  else if ( typeString == strings[TRIANGLE_6] ) topologyType = TRIANGLE_6;
  else if ( typeString == strings[QUADRILATERAL_8] ) topologyType = QUADRILATERAL_8;
  else if ( typeString == strings[TETRAHEDRON_10] ) topologyType = TETRAHEDRON_10;
  else if ( typeString == strings[PYRAMID_13] ) topologyType = PYRAMID_13;
  else if ( typeString == strings[WEDGE_15] ) topologyType = WEDGE_15;
  else if ( typeString == strings[HEXAHEDRON_20] ) topologyType = HEXAHEDRON_20;
  else if ( typeString == strings[MIXED] ) topologyType = MIXED;
  else if ( typeString == strings[SMESH2D] ) topologyType = SMESH2D;
  else if ( typeString == strings[RECTMESH2D] ) topologyType = RECTMESH2D;
  else if ( typeString == strings[CORECTMESH2D] ) topologyType = CORECTMESH2D;
  else if ( typeString == strings[SMESH3D] ) topologyType = SMESH3D;
  else if ( typeString == strings[RECTMESH3D] ) topologyType = RECTMESH3D;
  else if ( typeString == strings[CORECTMESH3D] ) topologyType = CORECTMESH3D;
  else XDM_THROW( xdmFormat::ReadError( "Unrecognized XDMF topology type" ) );

  xdm::RefPtr< xdmGrid::Topology > result;
  if ( topologyType <= MIXED ) {
    // Unstructured topologies.
    XDM_THROW( xdmFormat::ReadError( "Unstructured XDMF topologies are unsupported." ) );
  } else {
    xdm::RefPtr< xdmGrid::RectilinearMesh > structuredTopology(
      new xdmGrid::RectilinearMesh );
    // Get the attribute that specifies the shape of the structured topology
    xdm::DataShape<> shape;
    XPathQuery shapeQuery( mXPathContext, node, "@Dimensions|@NumberOfElements" );
    if ( shapeQuery.size() > 0 ) {
      structuredTopology->setShape( xdm::makeShape( shapeQuery.textValue( 0 ) ) );
    } else {
      XDM_THROW( xdmFormat::ReadError(
        "No dimensions specified for XDMF structured topology" ) );
    }
    result = structuredTopology;
  }

  // Attach connectivity information specified in DataItem children.
  XPathQuery connectivityQuery( mXPathContext, node, "DataItem" );
  for ( size_t i = 0; i < connectivityQuery.size(); i++ ) {
    result->appendChild( buildUniformDataItem( connectivityQuery.node( i ) ) );
  }

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Attribute > 
TreeBuilder::buildAttribute( xmlNode * node ) {
  // Mapping from XDMF XML attribute type values to enumeration of types.
  typedef std::map< std::string, xdmGrid::Attribute::Type > TypeMap;
  static TypeMap typeMap;
  if ( typeMap.empty() ) {
    typeMap["Scalar"] = xdmGrid::Attribute::kScalar;
    typeMap["Vector"] = xdmGrid::Attribute::kVector;
    typeMap["Tensor"] = xdmGrid::Attribute::kTensor;
    typeMap["Tensor6"] = xdmGrid::Attribute::kTensor6;
    typeMap["Matrix"] = xdmGrid::Attribute::kMatrix;
    typeMap["GlobalID"] = xdmGrid::Attribute::kGlobalId;
  }

  // Mapping from XDMF XML attribute center values to enumeration of centering.
  typedef std::map< std::string, xdmGrid::Attribute::Center > CenterMap;
  static CenterMap centerMap;
  if ( centerMap.empty() ) {
    centerMap["Node"] = xdmGrid::Attribute::kNode;
    centerMap["Cell"] = xdmGrid::Attribute::kCell;
    centerMap["Grid"] = xdmGrid::Attribute::kGrid;
    centerMap["Face"] = xdmGrid::Attribute::kFace;
    centerMap["Edge"] = xdmGrid::Attribute::kEdge;
  }

  // create the result attribute.
  xdm::RefPtr< xdmGrid::Attribute > result( new xdmGrid::Attribute );

  // Get the name of the attribute.
  XPathQuery nameQuery( mXPathContext, node, "@Name" );
  if ( nameQuery.size() > 0 ) {
    result->setName( nameQuery.textValue( 0 ) );
  }

  // Get the attribute type
  XPathQuery typeQuery( mXPathContext, node, "@AttributeType" );
  if ( typeQuery.size() > 0 ) {
    TypeMap::const_iterator type = typeMap.find( typeQuery.textValue( 0 ) );
    if ( type != typeMap.end() ) {
      result->setDataType( type->second );
    } else {
      XDM_THROW( xdmFormat::ReadError( "Unrecognized XDMF attribute type" ) );
    }
  } else {
    // default is scalar
    result->setDataType( xdmGrid::Attribute::kScalar );
  }

  // Get the attribute center
  XPathQuery centerQuery( mXPathContext, node, "@Center" );
  if ( centerQuery.size() > 0 ) {
    CenterMap::const_iterator center = centerMap.find( centerQuery.textValue( 0 ) );
    if ( center != centerMap.end() ) {
      result->setCentering( center->second );
    } else {
      XDM_THROW( xdmFormat::ReadError( "Unrecognized XDMF attribute center." ) );
    }
  } else {
    // default is node centered.
    result->setCentering( xdmGrid::Attribute::kNode );
  }

  // Create the attribute data items.
  XPathQuery dataQuery( mXPathContext, node, "DataItem" );
  if ( dataQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Attribute contains no data." ) );
  }
  result->setDataItem( buildUniformDataItem( dataQuery.node( 0 ) ) );

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::UniformGrid > 
TreeBuilder::buildUniformGrid( xmlNode * node ) {
  xdm::RefPtr< xdmGrid::UniformGrid > result( new xdmGrid::UniformGrid );

  // Get the topology.
  XPathQuery topologyQuery( mXPathContext, node, "Topology" );
  if ( topologyQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Grid contains no Topology" ) );
  }
  if ( topologyQuery.size() > 1 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Grid contains more than one Topology" ) );
  }
  result->setTopology( buildTopology( topologyQuery.node( 0 ) ) );

  // Get the geometry.
  XPathQuery geometryQuery( mXPathContext, node, "Geometry" );
  if ( geometryQuery.size() == 0 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Grid contains no geometry." ) );
  }
  if ( geometryQuery.size() > 1 ) {
    XDM_THROW( xdmFormat::ReadError( "XDMF Grid contains more than one Geometry" ) );
  }
  result->setGeometry( buildGeometry( geometryQuery.node( 0 ) ) );

  // Read all the attributes.
  XPathQuery attributeQuery( mXPathContext, node, "Attribute" );
  for ( size_t i = 0; i < attributeQuery.size(); i++ ) {
    result->addAttribute( buildAttribute( attributeQuery.node( i ) ) );
  }

  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Grid > TreeBuilder::buildGrid( xmlNode * node ) {
  // Determine the grid type.
  XPathQuery typeQuery( mXPathContext, node, "@GridType" );
  if ( typeQuery.size() > 0 ) {
    std::string gridType = typeQuery.textValue( 0 );
    if ( gridType == "Uniform" ) {
      return buildUniformGrid( node );
    } else if ( gridType == "Collection" ) {
      return buildCollectionGrid( node );
    } else {
      XDM_THROW( xdmFormat::ReadError( "Unsupported XDMF Grid type" ) );
    }
  }
  // UniformGrid is default
  return buildUniformGrid( node );
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Grid > TreeBuilder::buildCollectionGrid( xmlNode * node ) {
  // Determine the collection type.
  XPathQuery typeQuery( mXPathContext, node, "@CollectionType" );
  if ( typeQuery.size() == 0 ) {
    return buildSpatialCollectionGrid( node );
  } else if ( typeQuery.textValue( 0 ) == "Spatial" ) {
    return buildSpatialCollectionGrid( node );
  } else if ( typeQuery.textValue( 0 ) == "Temporal" ) {
    return buildTemporalCollectionGrid( node );
  } else {
    XDM_THROW( xdmFormat::ReadError( "Unrecognized XDMF Grid collection type" ) );
  }
}

// -----------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::CollectionGrid >
TreeBuilder::buildSpatialCollectionGrid( xmlNode * node ) {
  xdm::RefPtr< xdmGrid::CollectionGrid > result(
    new xdmGrid::CollectionGrid( xdmGrid::CollectionGrid::kSpatial ) );
  // Find all grid children of the node.
  XPathQuery childGridQuery( mXPathContext, node, "Grid" );
  for ( size_t i = 0; i < childGridQuery.size(); i++ ) {
    result->appendChild( buildGrid( childGridQuery.node( i ) ) );
  }
  return result;
}

//------------------------------------------------------------------------------
xdm::RefPtr< xdmGrid::Grid >
TreeBuilder::buildTemporalCollectionGrid( xmlNode * node ) {
  return xdm::RefPtr< xdmGrid::Grid >();
}


} // namespace impl
} // namespace xdmf
