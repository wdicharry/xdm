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
#include <xdmGrid/InterlacedGeometry.hpp>

#include <xdm/VectorRef.hpp>

#include <stdexcept>
#include <cassert>

#include <xdm/ThrowMacro.hpp>

namespace xdmGrid {

InterlacedGeometry::InterlacedGeometry( unsigned int dimension ) :
  Geometry( dimension ) {
  setNumberOfChildren( 1 );
}

InterlacedGeometry::~InterlacedGeometry() {
}

void InterlacedGeometry::setCoordinateValues( xdm::RefPtr< xdm::UniformDataItem > data ) {
  if ( child( 0 ) ) {
    XDM_THROW( std::logic_error( "setCoordinateValues was called after the values had"
      " already been set by a previous call to setCoordinateValues." ) );
  }
  setChild( 0, data );
}

void InterlacedGeometry::writeMetadata( xdm::XmlMetadataWrapper& xml ) {
  Geometry::writeMetadata( xml );

  unsigned int dim = dimension();
  switch ( dim ) {
  case 1:
    xml.setAttribute( "GeometryType", "X" );
    break;
  case 2:
    xml.setAttribute( "GeometryType", "XY" );
    break;
  case 3:
    xml.setAttribute( "GeometryType", "XYZ" );
    break;
  default:
    XDM_THROW( std::domain_error( "Unsupported number of dimensions" ) );
    break;
  }
}

xdm::RefPtr< xdm::VectorRefImp< double > > InterlacedGeometry::createVectorImp()
{
  return xdm::RefPtr< xdm::VectorRefImp< double > >(
    new xdm::SingleArrayOfVectorsImp< double >(
      child( 0 )->typedArray< double >()->begin(),
      dimension() ) );
}

void InterlacedGeometry::updateDimension() {
  // do nothing.
}

} // namespace xdmGrid

