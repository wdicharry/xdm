#ifndef xdm_SelectableDataMixin_hpp
#define xdm_SelectableDataMixin_hpp

#include <xdm/RefPtr.hpp>

#include <xdm/NamespaceMacro.hpp>

XDM_NAMESPACE_BEGIN

class AllDataSelection;
class DataSelection;

/// Mixin that provides data selection services to inheritors.  Classes that
/// require the ability to select data should inherit this interface and
/// implementation.
class SelectableDataMixin {
public:

  /// Set the selection for the data.
  void setSelection( DataSelection* selection );
  /// Get the const data selection.
  const DataSelection* selection() const;
  /// Get the data selection.
  DataSelection* selection();

  /// Convenience method to select an entire dataset.
  void selectAll();
  
protected:
  
  /// Default constructor defaults to selecting all data.
  SelectableDataMixin();
  ~SelectableDataMixin();

private:
  RefPtr< DataSelection > mSelection;
  static RefPtr< AllDataSelection > sAllSelection;
};

XDM_NAMESPACE_END

#endif // xdm_SelectableDataMixin_hpp
