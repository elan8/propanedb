#include <gtest/gtest.h>
#include "test.pb.h"
#include "DatabaseServiceImpl.h"

// Demonstrate some basic assertions.
TEST(HelloTest, PutGet) {

  DatabaseServiceImpl service("/tmp/test1");

  test::TodoItem entity;
    string description("Test PropaneDB");
    entity.set_description(description);

    Any *anyMessage = new Any();
    anyMessage->PackFrom(entity);

    grpc::ServerContext context;

    PropaneEntity request;
    request.set_allocated_data(anyMessage);

    propane::PropaneId reply;

    grpc::Status s = service.Put(&context,&request, &reply);
 
    EXPECT_EQ(s.ok(), true);

//   // Expect two strings not to be equal.
//   EXPECT_STRNE("hello", "world");
//   // Expect equality.
//   EXPECT_EQ(7 * 6, 42);
}