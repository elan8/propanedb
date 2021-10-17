# PropaneDB
A document database for Protocol Buffer messages with GRPC interface

Ideal for GRPC microservices: 
- Use the messages defined in the proto file both for communication and storage: single source of truth
- Can be used with any programming language that is supported by GRPC 

Features:
- Store Protobuf objects in serialized form (google.protobuf.Any) in a database
- Retrieve those objects using their UUID
- Delete objects based on their UUID  
- Search function based on the fields of the Protobuf objects
- Support multiple databases per instance

Currently this project is still pre-alpha: not suitable for any practical usage yet.

## Drivers
- [Golang driver](https://github.com/elan8/propanedb-go-driver)
- [Demo](https://github.com/elan8/propanedb-demo)

## Implementation
- Storage engine is RocksDB.
- Interface is based on GRPC
- Deployment using Docker containers
