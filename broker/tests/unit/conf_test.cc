// Copyright 2020 Andrew Dunstall

#include <list>
#include <optional>
#include <string>

#include "broker/conf.h"
#include "gtest/gtest.h"

namespace wombat::broker {

class ConfTest : public ::testing::Test {};

TEST_F(ConfTest, ParseOk) {
  std::list<PartitionConf> expected{};
  expected.emplace_back(
      PartitionConf::Type::kLeader,
      "/usr/local/wombat/log",
      "192.168.1.5",
      3101
  );
  expected.emplace_back(
      PartitionConf::Type::kReplica,
      "/usr/local/wombat/log",
      "10.26.104.122",
      9224
  );

  const std::string s = R"(leader:/usr/local/wombat/log:192.168.1.5:3101
replica:/usr/local/wombat/log:10.26.104.122:9224)";
  std::optional<Conf> cfg = Conf::Parse(s);
  EXPECT_TRUE(cfg);
  EXPECT_EQ(expected, cfg->partitions());
}

TEST_F(ConfTest, ParseInvalid) {
  const std::string s = "badconf";
  std::optional<Conf> cfg = Conf::Parse(s);
  EXPECT_FALSE(cfg);
}

class PartitionConfTest : public ::testing::Test {};

TEST_F(PartitionConfTest, ParseLeaderConfigOk) {
  PartitionConf expected(
      PartitionConf::Type::kLeader,
      "/usr/local/wombat/log",
      "192.168.1.5",
      3101
  );

  const std::string s = "leader:/usr/local/wombat/log:192.168.1.5:3101";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  ASSERT_TRUE(cfg);
  EXPECT_EQ(expected, *cfg);
}

TEST_F(PartitionConfTest, ParseReplicaConfigOk) {
  PartitionConf expected(
      PartitionConf::Type::kReplica,
      "/wombat/log",
      "10.26.104.122",
      9224
  );

  const std::string s = "replica:/wombat/log:10.26.104.122:9224";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  ASSERT_TRUE(cfg);
  EXPECT_EQ(expected, *cfg);
}

TEST_F(PartitionConfTest, ParseConfigTypeInvalid) {
  const std::string s = "nan:/wombat/log/replica:10.26.104.122:9224";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  EXPECT_FALSE(cfg);
}

TEST_F(PartitionConfTest, ParseConfigFieldsMissing) {
  const std::string s = "replica:/wombat/log/replica:10.26.104.122";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  EXPECT_FALSE(cfg);
}

TEST_F(PartitionConfTest, ParseConfigPortInvalid) {
  const std::string s = "replica:/wombat/log/replica:10.26.104.122:nan";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  EXPECT_FALSE(cfg);
}

TEST_F(PartitionConfTest, ParseConfigPortOverflow) {
  const std::string s = "replica:/wombat/log/replica:10.26.104.122:99999";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  EXPECT_FALSE(cfg);
}

TEST_F(PartitionConfTest, ParseConfigPortUnderflow) {
  const std::string s = "replica:/wombat/log/replica:10.26.104.122:-1";
  std::optional<PartitionConf> cfg = PartitionConf::Parse(s);

  EXPECT_FALSE(cfg);
}

}  // namespace wombat::broker
