#include <gtest/gtest.h>

#include <xdm/TemplateStructuredArray.hpp>
#include <xdm/UniformDataItem.hpp>

#include <algorithm>

// dummy dataset implementation for testing.
class DummyDataset : public xdm::Dataset {
  const char* format() { return "Dummy"; }
  void initializeImplementation(
    xdm::primitiveType::Value,
    const xdm::DataShape<>&,
    std::iostream&  ) {}
  void serializeImplementation(
    const xdm::StructuredArray&,
    const xdm::HyperSlabMap<>&, 
    std::iostream& ) {}
  void finalizeImplementation() {}
};

// Fixture defines a common UniformDataItem for testing.
class UniformDataItem : public ::testing::Test {
public:
  float data[4];
  xdm::RefPtr< xdm::UniformDataItem > testItem;

  UniformDataItem() :
    data(),
    testItem() {
    std::fill( data, data + 4, 0.0 );
    xdm::DataShape<> arrayShape(1);
    arrayShape[0] = 4;
    xdm::RefPtr< xdm::StructuredArray > array( 
      createStructuredArray( data, arrayShape ) );
    xdm::RefPtr< xdm::Dataset > dataset( new DummyDataset );

    xdm::DataShape<> datasetShape(2);
    datasetShape[0] = datasetShape[1] = 2;
    testItem = new xdm::UniformDataItem( datasetShape );

    testItem->setArray( array );
    testItem->setDataset( dataset );
  }
};


TEST_F( UniformDataItem, writeMetadata ) {
  xdm::RefPtr< xdm::XmlObject > obj( new xdm::XmlObject );
  xdm::XmlMetadataWrapper xml( obj );
  testItem->writeMetadata( xml );

  ASSERT_EQ( xml.tag(), "DataItem" );
  ASSERT_EQ( xml.attribute( "ItemType" ), "Uniform" );
  ASSERT_EQ( xml.attribute( "Dimensions" ), "2 2" );
  ASSERT_EQ( xml.attribute( "NumberType" ), "Float" );
  ASSERT_EQ( xml.attribute( "Precision" ), "4" );
  ASSERT_EQ( xml.attribute( "Format" ), "Dummy" );
}

int main( int argc, char* argv[] ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

