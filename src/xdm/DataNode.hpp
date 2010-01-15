#ifndef xdm_DataNode_hpp
#define xdm_DataNode_hpp

#include <xdm/Dataset.hpp>
#include <xdm/HyperSlab.hpp>
#include <xdm/Node.hpp>
#include <xdm/StructuredArray.hpp>

#include <xdm/NamespaceMacro.hpp>

XDM_NAMESPACE_BEGIN

/// Terminal node containing the actual heavy data sets.  A DataNode contains
/// all of the information required to map a StructuredArray in memory to a
/// Dataset in a file. 
class DataNode : public Node {
public:
  explicit DataNode( const DataShape<>& dataspace );
  virtual ~DataNode();

  StructuredArray* array();
  const StructuredArray* array() const;
  void setArray( StructuredArray* array );

  Dataset* dataset();
  const Dataset* dataset() const;
  void setDataset( Dataset* ds );

  DataShape<>& dataspace();
  const DataShape<>& dataspace() const;
  void setDataspace( const DataShape<>& dataspace );

private:
  DataShape<> mDataspace;
  RefPtr< StructuredArray > mArray;
  RefPtr< Dataset > mDataset;
};

XDM_NAMESPACE_END

#endif // xdm_DataNode_hpp

