# PropaneDB
A document database for Protocol Buffer messages with GRPC interface

Ideal for GRPC microservices: Use the messages defined in the proto file both for communication and storage

Features:
- [x] Store Protobuf objects in serialized form (google.protobuf.Any) in a database
- [x] Retrieve those objects using their UUID
- [x] Delete objects based on their UUID  
- [x] Search function based on the fields of the Protobuf objects
- [x] Support multiple databases per instance


Currently this project is still pre-alpha: not suitable for any practical usage yet.


## Implementation
The storage engine is RocksDB.
Supported platform: Linux
Deplyment using Docker containers
