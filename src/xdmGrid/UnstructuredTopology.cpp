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
#include <xdmGrid/UnstructuredTopology.hpp>

#include <sstream>

//namespace {
//
//class TrivialCellRefImpl : public xdmGrid::CellSharedImp {
//public:
//  TrivialCellRefImpl(
//    const xdmGrid::CellType::Type& type,
//    xdm::RefPtr< xdmGrid::Geometry > geometry,
//    std::size_t* nodeIndexArray ) :
//    mType( type ), mGeometry( geometry ), mNodeIds( nodeIndexArray ) {
//  }
//
//  virtual xdmGrid::ConstNode node( std::size_t cellIndex, std::size_t nodeIndex ) const {
//    std::size_t nodeID = mNodeIds[ cellIndex * mType.nodesPerCell() + nodeIndex ];
//    return mGeometry->node( nodeID );
//  }
//
//  virtual xdmGrid::CellType::Type cellType( std::size_t cellIndex ) const {
//    return mType;
//  }
//
//private:
//  xdmGrid::CellType::Type mType;
//  xdm::RefPtr< xdmGrid::Geometry > mGeometry;
//  std::size_t* mNodeIds;
//};
//
//} // anon namespace

XDM_GRID_NAMESPACE_BEGIN

UnstructuredTopology::UnstructuredTopology() :
  Topology(),
  mConnectivity(),
  mCellType( CellType::Default ),
  mOrdering() {
}

UnstructuredTopology::~UnstructuredTopology() {
}

void UnstructuredTopology::setCellType( const CellType::Type& type ) {
  mCellType = type;
}

const CellType::Type& UnstructuredTopology::cellType( std::size_t ) const {
  return mCellType;
}

void UnstructuredTopology::setNodeOrdering( const NodeOrderingConvention::Type& order )
{
  mOrdering = order;
}

NodeOrderingConvention::Type UnstructuredTopology::nodeOrdering() const
{
  return mOrdering;
}

void UnstructuredTopology::setConnectivity( xdm::RefPtr< xdm::UniformDataItem > connectivity ) {
  mConnectivity = connectivity;
}

void UnstructuredTopology::traverse( xdm::ItemVisitor& iv ) {
  if ( mConnectivity.valid() ) {
    mConnectivity->accept( iv );
  }
}

void UnstructuredTopology::writeMetadata( xdm::XmlMetadataWrapper& xml ) {
  Topology::writeMetadata( xml );

  // Write the number of cells
  xml.setAttribute( "NumberOfElements", numberOfCells() );

  // Write the cell type
  xml.setAttribute( "ElementType", mCellType.shapeName() );

  // Write the nodes per cell
  xml.setAttribute( "NodesPerElement", mCellType.nodesPerCell() );
}

xdm::RefPtr< xdm::VectorRefImp< std::size_t > > UnstructuredTopology::createVectorImp() {
  return xdm::RefPtr< xdm::VectorRefImp< std::size_t > >(
    new xdm::SingleArrayOfVectorsImp< std::size_t >(
        mConnectivity->typedArray< std::size_t >()->begin(), mCellType.nodesPerCell() ) );
}

XDM_GRID_NAMESPACE_END

