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
#ifndef xdm_DataSelection_hpp
#define xdm_DataSelection_hpp

#include <xdm/ReferencedObject.hpp>



namespace xdm {

class DataSelectionVisitor;

/// Base class for selections of subsets of data.  This class provides the
/// abstraction of a selection of a subset of an array.  Subclasses of this type
/// should define a kind of selection.  Examples of selections are by hyperslab,
/// where a start stride and count in each dimension are defined, or by
/// coordinate where a set of coordinates is defined.
///
/// Dataset implementations should implement a subclass of DataSelectionVisitor
/// to operate on specific selection types and define the behavior of a
/// selection, which is generally dependent upon the dataset type.  For example,
/// the HDF dataset defines a DataSelectionVisitor that maps specific
/// DataSelection subclasses to the HDF5 API for selecting subsets of file data
/// and array data.
///
/// @see DataSelectionVisitor
/// @see AllDataSelection
/// @see HyperslabDataSelection
class DataSelection : public ReferencedObject {
public:
  DataSelection();
  virtual ~DataSelection();

  /// Visitor accept interface for typed operations on subclasses.  Inheritors
  /// should reimplement this as v.apply(*this) to pass their specific subclass
  /// into the visitor's apply method.
  virtual void accept( DataSelectionVisitor& v ) const;
};

} // namespace xdm

#endif // xdm_DataSelection_hpp

