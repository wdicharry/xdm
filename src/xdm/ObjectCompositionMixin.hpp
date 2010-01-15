#ifndef xdm_ObjectCompositionMixin_hpp
#define xdm_ObjectCompositionMixin_hpp

#include <xdm/ReferencedObject.hpp>
#include <xdm/RefPtr.hpp>

#include <vector>

#include <cassert>

#include <xdm/NamespaceMacro.hpp>

XDM_NAMESPACE_BEGIN

/// Mixin that provides ReferencedObject composition through inheritance.
///
/// Inheritors of this class are imbued with an interface and implementation of
/// ReferencedObject composition.  This is useful for avoiding code duplication
/// when some class needs to contain a collection of ReferencedObjects.
///
/// When multiple inheritance is involved, it is probably easier to access the
/// class methods with the Non-member templates defined below.
template< typename T >
class ObjectCompositionMixin {
public:
  typedef typename std::vector< xdm::RefPtr< T > >::iterator Iterator;
  typedef typename std::vector< xdm::RefPtr< T > >::const_iterator ConstIterator;

  ~ObjectCompositionMixin() {}

  /// Get an iterator to the beginning of the child objects.
  Iterator begin() { return mChildObjects.begin(); }
  /// Get a const iterator pointing to the beginning of the child objects.
  ConstIterator begin() const { return mChildObjects.begin(); }
  /// Get an iterator pointint to the end of the child objects.
  Iterator end() { return mChildObjects.end(); }
  /// Get a const iterator pointing to the end of the child objects.
  ConstIterator end() const { return mChildObjects.end(); }

  /// Get the number of children.
  unsigned int numberOfChildren() const {
    return mChildObjects.size();
  }

  /// Set the number of child objects.  This will resize the storage
  /// constructing default objects or unreferencing objects past the end.
  void setNumberOfChildren( unsigned int n ) {
    mChildObjects.resize( n );
  }

  /// Set the ith child, possibly overwriting (and therefore unreferencing) an
  /// existing child.
  void setChild( unsigned int i, T* child ) {
    assert( i < mChildObjects.size() );
    mChildObjects[i] = child;
  }

  /// Append a child to the end of the list.
  void appendChild( T* object ) {
    mChildObjects.push_back( object );
  }

  /// Get the i'th child of the object.
  T* child( unsigned int i ) {
  	assert( i < mChildObjects.size() );
  	return mChildObjects[i];
  }

  /// Get the const i'th child of the object.
  const T* child( unsigned int i ) const {
  	assert( i < mChildObjects.size() );
  	return mChildObjects[i];
  }

protected:
  
  /// Only sublclasses can construct.  Takes an optional argument to initialize
  /// the number of contained objects.
  ObjectCompositionMixin( unsigned int size = 0 ) : mChildObjects( size ) {}

private:
  
  std::vector< xdm::RefPtr< T > > mChildObjects;

};

/// Get the beginning of an object's children.
template< typename T >
typename ObjectCompositionMixin< T >::Iterator 
begin( ObjectCompositionMixin< T >& obj ) {
  return obj.begin();
}

/// Get the beginning of an object's children.
template< typename T >
typename ObjectCompositionMixin< T >::ConstIterator
begin( const ObjectCompositionMixin< T >& obj ) {
  return obj.begin();
}

/// Get the end of an object's children.
template< typename T >
typename ObjectCompositionMixin< T >::Iterator
end( ObjectCompositionMixin< T >& obj ) {
  return obj.end();
}

/// Get the end of an object's children.
template< typename T >
typename ObjectCompositionMixin< T >::ConstIterator
end( const ObjectCompositionMixin< T >& obj ) {
  return obj.end();
}

/// Get the i'th child of an object.
template< typename T >
T* child( ObjectCompositionMixin< T >& obj, unsigned int i ) {
	return obj.child( i );
}

/// Get the const i'th child of an object.
template< typename T >
const T* child( const ObjectCompositionMixin< T >& obj, unsigned int i ) {
	return obj.child( i );
}

/// Set the i'th child of the given object.
template< typename T > 
void setChildObject( ObjectCompositionMixin< T >& obj, unsigned int i, T* child ) {
	obj.setChildObject( i, child );
}

/// Append a child to the given object.
template< typename T >
void appendChild( ObjectCompositionMixin< T >& obj, T* child ) {
  obj.appendChild( child );
} 

XDM_NAMESPACE_END

#endif // xdm_ObjectCompositionMixin_hpp