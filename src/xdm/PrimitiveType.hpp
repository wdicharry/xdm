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
#ifndef xdm_PrimitiveType_hpp
#define xdm_PrimitiveType_hpp

#include <map>



namespace xdm {

namespace primitiveType {
  /// Enumeration of type names.
  enum Value {
    kChar = 0,
    kShort,
    kInt,
    kLongInt,
    kUnsignedChar,
    kUnsignedShort,
    kUnsignedInt,
    kLongUnsignedInt,
    kFloat,
    kDouble  
  };
} // namespace primitiveType

/// Function to provide size information for a primitive type.
std::size_t typeSize( primitiveType::Value type );

/// Structure to provide static type information for values of the
/// primitiveType::Value enumeration.
template< typename T > struct PrimitiveTypeInfo {
  /// A compile error on this line means a specialization of PrimitiveTypeInfo
  /// has not been defined for the type T.  A specialization must be defined
  /// that provides values for PrimitiveTypeInfo::kSize and
  /// PrimitiveTypeInfo::kValue.
  typedef typename T::NoSpecializationForRequestedType StaticFailure;
};

#define XDM_PRIMITIVE_TYPE_DEFINE_INFO( type, value ) \
  template<> struct PrimitiveTypeInfo< type > { \
    static const primitiveType::Value kValue = value; \
    static const std::size_t kSize = sizeof( type ); \
  };

XDM_PRIMITIVE_TYPE_DEFINE_INFO( char, primitiveType::kChar )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( short, primitiveType::kShort )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( int, primitiveType::kInt )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( long int, primitiveType::kLongInt )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( unsigned char, primitiveType::kUnsignedChar )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( unsigned short, primitiveType::kUnsignedShort )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( unsigned int, primitiveType::kUnsignedInt )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( long unsigned int, primitiveType::kLongUnsignedInt )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( float, primitiveType::kFloat )
XDM_PRIMITIVE_TYPE_DEFINE_INFO( double, primitiveType::kDouble )

#undef XDM_PRIMITIVE_TYPE_DEFINE_INFO

} // namespace xdm

#endif // xdm_PrimitiveType_hpp

