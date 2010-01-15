//=============================================================================
// This software developed by Stellar Science Ltd Co and the U.S. Government.
// Copyright (C) 2009 Stellar Science. Government-purpose rights granted.
//-----------------------------------------------------------------------------
#ifndef xdmGrid_RectilinearMesh_hpp
#define xdmGrid_RectilinearMesh_hpp

#include <xdmGrid/StructuredTopology.hpp>

#include <xdmGrid/NamespaceMacro.hpp>

XDM_GRID_NAMESPACE_BEGIN

/// Mesh in which axes are mutually orthogonal with variable spacing between
/// nodes.
class RectilinearMesh : public StructuredTopology {
public:
  RectilinearMesh();
  virtual ~RectilinearMesh();

  XDM_META_ITEM( RectilinearMesh );

  virtual void writeMetadata( xdm::XmlMetadataWrapper& xml );
};

XDM_GRID_NAMESPACE_END

#endif // xdmGrid_RectilinearMesh_hpp
