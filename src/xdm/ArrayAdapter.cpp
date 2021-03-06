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
#include <xdm/ArrayAdapter.hpp>

#include <xdm/DataSelection.hpp>
#include <xdm/Dataset.hpp>
#include <xdm/StructuredArray.hpp>

#include <algorithm>
#include <numeric>

namespace xdm {

ArrayAdapter::ArrayAdapter( RefPtr< StructuredArray > array, bool isDynamic ) :
  MemoryAdapter( isDynamic ),
  mArray( array ),
  mSelectionMap()
{
}

ArrayAdapter::~ArrayAdapter()
{
}

RefPtr< StructuredArray > ArrayAdapter::array() {
  return mArray;
}

RefPtr< const StructuredArray > ArrayAdapter::array() const {
  return mArray;
}

void ArrayAdapter::setArray( RefPtr< StructuredArray > array ) {
  mArray = array;
}

const DataSelectionMap& ArrayAdapter::selectionMap() const {
  return mSelectionMap;
}

void ArrayAdapter::setSelectionMap( const DataSelectionMap& selectionMap ) {
  mSelectionMap = selectionMap;
}

void ArrayAdapter::writeImplementation( Dataset* dataset ) {
  dataset->serialize( mArray.get(), mSelectionMap );
}

void ArrayAdapter::readImplementation( Dataset* dataset ) {
  DataShape<> shape = dataset->shape();
  size_t totalSize = std::accumulate( shape.begin(), shape.end(), 1,
    std::multiplies< size_t >() );
  mArray->resize( totalSize );
  dataset->deserialize( mArray.get(), mSelectionMap );
}

} // namespace xdm

