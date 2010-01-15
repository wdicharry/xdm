//=============================================================================
// This software developed by Stellar Science Ltd Co and the U.S. Government.
// Copyright (C) 2009 Stellar Science. Government-purpose rights granted.
//-----------------------------------------------------------------------------
#ifndef xdm_DataSelectionVisitor_hpp
#define xdm_DataSelectionVisitor_hpp

#include <xdm/DataSelection.hpp>
#include <xdm/ReferencedObject.hpp>
#include <xdm/RefPtr.hpp>

#include <xdm/NamespaceMacro.hpp>

XDM_NAMESPACE_BEGIN

class AllDataSelection;
class CoordinateDataSelection;
class HyperslabDataSelection;

class DataSelectionVisitor : public virtual ReferencedObject {
public:
  DataSelectionVisitor();
  virtual ~DataSelectionVisitor();

  //-- apply methods for known subclasses of DataSelection --//
  virtual void apply( const DataSelection& selection );
  virtual void apply( const AllDataSelection& selection );
  virtual void apply( const HyperslabDataSelection& selection );
  virtual void apply( const CoordinateDataSelection& selection );

};

XDM_NAMESPACE_END

#endif // xdm_DataSelectionVisitor_hpp

