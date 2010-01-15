#ifndef xdm_HyperslabDataSelection_hpp
#define xdm_HyperslabDataSelection_hpp

#include <xdm/DataSelection.hpp>
#include <xdm/DataSelectionVisitor.hpp>
#include <xdm/HyperSlab.hpp>

XDM_NAMESPACE_BEGIN

/// DataSelection consisting sampling data with a given start, stride, and
/// count.
class HyperslabDataSelection : public DataSelection {
private:
  HyperSlab<> mHyperslab;
public:
  /// Constructor takes a hyperslab to select.  Defaults to an empty hyperslab.
  HyperslabDataSelection( const HyperSlab<>& hyperslab = HyperSlab<>() ) :
    mHyperslab( hyperslab ) {}

  virtual ~HyperslabDataSelection() {}

  /// Set the hyperslab for the selection.
  void setHyperslab( const HyperSlab<>& hyperslab ) { 
    mHyperslab = hyperslab; 
  }
  /// Get the selection's hyperslab.
  const HyperSlab<>& hyperslab() const {
    return mHyperslab;
  }

  /// Visitor accept interface.
  virtual void accept( DataSelectionVisitor& v ) const {
    v.apply( *this );
  }
};

XDM_NAMESPACE_END

#endif // xdm_HyperslabDataSelection_hpp
