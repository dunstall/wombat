// Copyright 2020 Andrew Dunstall

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "partition/partition.h"
#include "log/log.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"
#include "record/consumerequest.h"
#include "record/record.h"
#include "record/request.h"

namespace wombat::broker::partition::testing {

class PartitionTest : public ::testing::Test {};

TEST_F(PartitionTest, HandleProduceOk) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const record::Record record{data};
  const record::Request request{record::RequestType::kProduce, record.Encode()};

  EXPECT_EQ(std::nullopt, partition.Handle(request));

  EXPECT_EQ(record.Encode(), log->Lookup(0, record.Encode().size()));
}

TEST_F(PartitionTest, HandleProduceInvalid) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  const std::vector<uint8_t> payload{0, 0, 0xff, 0xff, 1, 2, 3};
  const record::Request request{record::RequestType::kProduce, payload};

  EXPECT_EQ(std::nullopt, partition.Handle(request));

  // Log should still be empty.
  EXPECT_EQ(0U, log->size());
  EXPECT_TRUE(log->Lookup(0U, 5U).empty());
}

TEST_F(PartitionTest, HandleConsumeRequestOk) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  // Add buffer data to read from non-zero offset.
  log->Append({0, 0, 0, 0, 0});

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const record::Record record{data};
  log->Append(record.Encode());

  const uint32_t offset = 0x05;
  const record::ConsumeRequest consume{offset};
  const record::Request request{record::RequestType::kConsume, consume.Encode()};

  std::optional<record::Response> response = partition.Handle(request);

  EXPECT_TRUE(response);
  EXPECT_EQ(response->payload(), record.Encode());
}

TEST_F(PartitionTest, HandleConsumeRequestOffsetNotFound) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  // Add buffer data to read from non-zero offset.
  log->Append({0, 0, 0, 0, 0});

  const uint32_t offset = 0x05;
  const record::ConsumeRequest consume{offset};
  const record::Request request{record::RequestType::kConsume, consume.Encode()};

  std::optional<record::Response> response = partition.Handle(request);


  const record::Record expected{{}};

  EXPECT_TRUE(response);
  EXPECT_EQ(response->payload(), expected.Encode());
}

TEST_F(PartitionTest, HandleConsumeRequestInvalid) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  const record::Request request{record::RequestType::kConsume, {0, 0xff}};

  std::optional<record::Response> response = partition.Handle(request);
  EXPECT_FALSE(response);
}

TEST_F(PartitionTest, HandleUnrecognizedRecord) {
  std::shared_ptr<log::Log<log::InMemorySegment>> log
      = std::make_shared<log::Log<log::InMemorySegment>>(
          log::GeneratePath(), 10'000
      );
  Partition<log::InMemorySegment> partition{log};

  const std::vector<uint8_t> payload{1, 2, 3};
  const record::Request request{static_cast<record::RequestType>(0xff), payload};

  EXPECT_EQ(std::nullopt, partition.Handle(request));

  // Log should still be empty.
  EXPECT_EQ(0U, log->size());
  EXPECT_TRUE(log->Lookup(0U, 5U).empty());
}

}  // namespace wombat::broker::partition::testing
