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

## Getting started
In order to use PropaneDB with your application, you can use the Golang driver.
1. First define your messages and RPC's in a .proto file and generate GRPC code and a Descriptor file:
```
protoc  --go_out=:. --go-grpc_out=:. --descriptor_set_out=./propane/test.bin -I.  ./api/test.proto
```
2. Open a terminal and use the following command to start an instance of PropaneDB
```
docker run -p=50051:50051  ghcr.io/elan8/propanedb:latest
```
3. First import the driver in your Go app and other dependencies (assuming you are using Go modules)
```
import (
	"context"
	"log"
	"io/ioutil"

	"github.com/elan8/propanedb-go-driver/propane"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/types/descriptorpb"
)
```
4. Connect to PropaneDB and create a database
```
	databaseName := "test"
	ctx := context.Background()
        port := "50051"

	b, err := ioutil.ReadFile("../propane/test.bin")
	if err != nil {
		log.Fatalf("Error: %s", err)
	}

	fds := &descriptorpb.FileDescriptorSet{}
	proto.Unmarshal(b, fds)

	client, err := propane.Connect(ctx, "localhost:"+port)
	if err != nil {
		log.Fatalf("Error: %s", err)
	}
	err = client.CreateDatabase(ctx, databaseName, fds)
	if err != nil {
		log.Printf("Error: %s", err)
	}
```
5. Instantiate a Protobuf message (=struct in Go) that you previously defined in your .proto file and store it in the database:
```
	item1 := &propane.TestEntity{}
	item1.Description = "Test 1"
	id1, err := client.Put(ctx, item1)
	if err != nil {
		log.Fatalf("Error: %s", err)
	}
	log.Print("Id1=" + id1)
```

## Related projects
- [Golang driver](https://github.com/elan8/propanedb-go-driver)
- [Demo](https://github.com/elan8/propanedb-demo)

## Implementation
- Storage engine is RocksDB.
- Interface is based on GRPC
- Deployment using Docker containers
