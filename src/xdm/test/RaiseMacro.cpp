#include <gtest/gtest.h>

#include <xdm/RaiseMacro.hpp>

#include <stdexcept>

TEST( RaiseMacro, throws ) {
  ASSERT_THROW( XDM_RAISE( std::runtime_error, "Test exception" ), 
    std::runtime_error );
}

int main( int argc, char* argv[] ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
