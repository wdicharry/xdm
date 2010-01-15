#include <xdmComm/MpiDatasetProxy.hpp>

#include <xdmComm/BinaryIOStream.hpp>
#include <xdmComm/BinaryStreamOperations.hpp>
#include <xdmComm/CoalescingStreamBuffer.hpp>

#include <xdm/AllDataSelection.hpp>
#include <xdm/DataSelection.hpp>
#include <xdm/DataSelectionMap.hpp>
#include <xdm/DataSelectionVisitor.hpp>
#include <xdm/DataShape.hpp>
#include <xdm/HyperslabDataSelection.hpp>
#include <xdm/StructuredArray.hpp>

XDM_COMM_NAMESPACE_BEGIN

namespace {

} // namespace anon

MpiDatasetProxy::MpiDatasetProxy( 
  MPI_Comm communicator, 
  xdm::Dataset* dataset,
  size_t bufSizeHint ) :
  mCommunicator( communicator ),
  mDataset( dataset ),
  mCommBuffer( new CoalescingStreamBuffer( bufSizeHint, communicator ) ),
  mArrayBuffer( bufSizeHint ) {
}

MpiDatasetProxy::~MpiDatasetProxy() {
}

const char* MpiDatasetProxy::format() {
  return mDataset->format();
}

void MpiDatasetProxy::writeTextContent( xdm::XmlTextContent& text ) {
  mDataset->writeTextContent( text );
}

void MpiDatasetProxy::initializeImplementation(
  xdm::primitiveType::Value type,
  const xdm::DataShape<>& shape ) {
  int rank;
  MPI_Comm_rank( mCommunicator, &rank );
  if ( rank == 0 ) {
    mDataset->initialize( type, shape );
  }
}

void MpiDatasetProxy::serializeImplementation(
  const xdm::StructuredArray* array,
  const xdm::DataSelectionMap& selectionMap ) {

  int localRank;
  MPI_Comm_rank( mCommunicator, &localRank );

  // prepare a stream for sending and receiving data.
  BinaryIOStream dataStream( mCommBuffer.get() );

  // if I'm not rank 0 in the communicator, pack my data and send it.
  if ( localRank != 0 ) {
    
    dataStream << *array;
    dataStream << selectionMap;
    dataStream.flush();
  
  } else {
    
    // write local process data to the dataset.
    mDataset->serialize( array, selectionMap );

    // receive data from everyone else in the communicator.
    int totalProcesses;
    MPI_Comm_size( mCommunicator, &totalProcesses );
    int received = 1; // already wrote local data
    while( received < totalProcesses ) {
      // synchronize the stream to receive from a single process.
      dataStream.sync();

      // reconstruct the information from the message
      xdm::StructuredArray processArray( 
        xdm::primitiveType::kChar, 
        &mArrayBuffer[0],
        xdm::DataShape<>() );
      xdm::DataSelectionMap processSelectionMap;
      dataStream >> processArray;
      dataStream >> processSelectionMap;

      // write the process data to the dataset.
      mDataset->serialize( &processArray, processSelectionMap );
      
      // completed a process
      received++;
    }
  }
}

void MpiDatasetProxy::finalizeImplementation() {
  int rank;
  MPI_Comm_rank( mCommunicator, &rank );
  if ( rank == 0 ) {
    mDataset->finalize();
  }
}

XDM_COMM_NAMESPACE_END

