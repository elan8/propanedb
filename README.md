# PropaneDB
A database microservice for Protocol Buffer messages with GRPC interface

Ideal for GRPC microservices: Use the messages defined in the proto file both for communication and storage

Planned features:
- [x] Store Protobuf objects in serialized form (google.protobuf.Any) in a database
- [x] Retrieve those objects using their UUID
- [x] Delete objects based on their UUID  
- [ ] Search function based on the fields of the Protobuf objects
- [ ] Support multiple databases per instance
- [ ] Database cluster functionality (cluster eventually consistent..sharding? )

Currently this project is still pre-alpha: not suitable for any practical usage yet.


## Implementation
The storage engine is RocksDB.
Supported platform: Linux
