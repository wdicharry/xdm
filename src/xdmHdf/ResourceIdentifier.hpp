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
#ifndef xdmHdf_ResourceIdentifier_hpp
#define xdmHdf_ResourceIdentifier_hpp

#include <xdm/ReferencedObject.hpp>

#include <hdf5.h>



namespace xdmHdf {

/// Reference counted class to manage HDF resources.  When the final reference
/// is destroyed, the resource is released according to the policy specified by
/// the template parameter.
template< typename ResourceReleaseFunctorT >
class ResourceIdentifier : public xdm::ReferencedObject {
private:
  // No copying!
  ResourceIdentifier( const ResourceIdentifier& other );
  ResourceIdentifier& operator=( const ResourceIdentifier& other );

  hid_t mIdentifier;
  ResourceReleaseFunctorT mReleaseFunctor;

public:
  /// Default constructor initializes the identifier to be invalid.
  ResourceIdentifier() :
    xdm::ReferencedObject(),
    mIdentifier( 0 ),
    mReleaseFunctor() {
  }

  /// Constructor takes an HDF identifier and manages it.
  explicit ResourceIdentifier( hid_t identifier ) :
    xdm::ReferencedObject(),
    mIdentifier( identifier ),
    mReleaseFunctor() {
  }

  /// Destructor releases the resource.
  virtual ~ResourceIdentifier() {
    release();
  }

  /// Access the identifier.
  hid_t get() const { return mIdentifier; }

  /// Release the identifier.
  void release() {
    if ( mIdentifier ) {
      mReleaseFunctor( mIdentifier );
      mIdentifier = 0;
    }
  }

  /// Reset the identifier.
  void reset( hid_t identifier ) {
    release();
    mIdentifier = identifier;
  }
};

} // namespace xdmHdf

#endif // xdmHdf_ResourceIdentifier_hpp

