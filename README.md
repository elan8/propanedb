# PropaneDB
A database for Protocol Buffer messages with GRPC interface

Ideal for GRPC microservices: Use the messages defined in the proto file both for communication and storage

Features:
- Put Protobuf message in database
- Get Protobuf message from database using ID

Currently still under heavy development: not suitable for any practical usage yet.


## Implementation
The storage engine is RocksDB.
Supported platform: Linux
