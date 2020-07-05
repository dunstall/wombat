// Copyright 2020 Andrew Dunstall

#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "partition/partition.h"
#include "record/record.h"
#include "record/recordrequest.h"
#include "record/request.h"
#include "tmp/log.h"

namespace wombat::broker::partition::testing {

class MockLog : public Log {
 public:
  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));
  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size), (override));  // NOLINT
  MOCK_METHOD(uint32_t, Send, (uint32_t offset, uint32_t size, int fd), (override));  // NOLINT
};

class PartitionTest : public ::testing::Test {};

TEST_F(PartitionTest, HandleProduceOk) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const record::Record record{data};
  const record::Request request{
      record::RequestType::kProduce, record.Encode()
  };

  EXPECT_CALL(*log, Append(record.Encode())).Times(1);

  EXPECT_EQ(std::nullopt, partition.Handle(request));
}

TEST_F(PartitionTest, HandleProduceInvalid) {
  // No calls should be made to log.
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};

  const std::vector<uint8_t> payload{0, 0, 0xff, 0xff, 1, 2, 3};
  const record::Request request{record::RequestType::kProduce, payload};

  EXPECT_EQ(std::nullopt, partition.Handle(request));
}

TEST_F(PartitionTest, HandleRecordRequestOk) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};
  
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const record::Record record{data};
  const std::vector<uint8_t> record_enc = record.Encode();

  const uint32_t offset = 0x05;
  const record::RecordRequest consume{offset};
  const record::Request request{
    record::RequestType::kConsume, consume.Encode()
  };
  EXPECT_CALL(*log, Lookup(offset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(
          std::vector<uint8_t>(record_enc.begin(), record_enc.begin() + 4)
      ));
  EXPECT_CALL(*log, Lookup(offset, record_enc.size()))
      .WillOnce(::testing::Return(record_enc));

  std::optional<record::Response> response = partition.Handle(request);
  EXPECT_TRUE(response);
  EXPECT_EQ(response->payload(), record_enc);
}

TEST_F(PartitionTest, HandleRecordRequestOffsetNotFound) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};

  const uint32_t offset = 0x05;
  const record::RecordRequest consume{offset};
  const record::Request request{
    record::RequestType::kConsume, consume.Encode()
  };
  EXPECT_CALL(*log, Lookup(offset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(std::vector<uint8_t>()));

  // Empty record should be returned.
  const record::Record record{{}};
  const std::vector<uint8_t> record_enc = record.Encode();
  
  std::optional<record::Response> response = partition.Handle(request);
  EXPECT_TRUE(response);
  EXPECT_EQ(response->payload(), record_enc);
}

TEST_F(PartitionTest, HandleRecordRequestInvalid) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};
  
  const record::Request request{record::RequestType::kConsume, {0, 0xff}};

  std::optional<record::Response> response = partition.Handle(request);
  EXPECT_FALSE(response);
}

TEST_F(PartitionTest, HandleUnrecognizedRecord) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  Partition partition{log};

  const std::vector<uint8_t> payload{1, 2, 3};
  const record::Request request{
    static_cast<record::RequestType>(0xff), payload
  };

  EXPECT_EQ(std::nullopt, partition.Handle(request));
}

}  // namespace wombat::broker::partition::testing
