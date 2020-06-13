#include "gtest/gtest.h"
#include "log/inmemorysegment.h"
#include "log/offsets.h"
#include "tempdir.h"

namespace wombat::log::testing {

class OffsetsTest : public ::testing::Test {};

TEST_F(OffsetsTest, OpenEmpty) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  Offsets<InMemorySegment> offsets(segment);

  uint32_t id;
  EXPECT_FALSE(offsets.Lookup(0, &id));
  EXPECT_FALSE(offsets.Lookup(0, &id));
}

TEST_F(OffsetsTest, LookupZeroOffset) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  Offsets<InMemorySegment> offsets(segment);

  uint32_t id = 0xfa2d;
  offsets.Insert(0, id);

  uint32_t id_lookup;
  EXPECT_TRUE(offsets.Lookup(0, &id_lookup));
  EXPECT_EQ(id, id_lookup);
  EXPECT_TRUE(offsets.Lookup(0xff, &id_lookup));
  EXPECT_EQ(id, id_lookup);
}

TEST_F(OffsetsTest, LookupPositiveOffset) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  Offsets<InMemorySegment> offsets(segment);

  uint32_t id = 0xfa2d;
  offsets.Insert(0xaa, id);

  uint32_t id_lookup;
  EXPECT_FALSE(offsets.Lookup(0, &id_lookup));
  EXPECT_TRUE(offsets.Lookup(0xaa, &id_lookup));
  EXPECT_EQ(id, id_lookup);
  EXPECT_TRUE(offsets.Lookup(0xff, &id_lookup));
  EXPECT_EQ(id, id_lookup);
}

TEST_F(OffsetsTest, LookupMultiOffset) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  Offsets<InMemorySegment> offsets(segment);

  uint32_t id1 = 0x01;
  uint32_t id2 = 0x02;
  offsets.Insert(0xa0, id1);
  offsets.Insert(0xb0, id2);

  uint32_t id_lookup;
  EXPECT_FALSE(offsets.Lookup(0x9f, &id_lookup));
  EXPECT_TRUE(offsets.Lookup(0xa0, &id_lookup));
  EXPECT_EQ(id1, id_lookup);
  EXPECT_TRUE(offsets.Lookup(0xaf, &id_lookup));
  EXPECT_EQ(id1, id_lookup);
  EXPECT_TRUE(offsets.Lookup(0xb0, &id_lookup));
  EXPECT_EQ(id2, id_lookup);
  EXPECT_TRUE(offsets.Lookup(0xff, &id_lookup));
  EXPECT_EQ(id2, id_lookup);
}

TEST_F(OffsetsTest, MaxOffset) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};
  Offsets<InMemorySegment> offsets(segment);

  EXPECT_EQ(0U, offsets.MaxOffset());

  offsets.Insert(0xa0, 1);
  EXPECT_EQ(0xa0U, offsets.MaxOffset());
  offsets.Insert(0xb0, 2);
  EXPECT_EQ(0xb0U, offsets.MaxOffset());
}

TEST_F(OffsetsTest, LoadPersistent) {
  auto path = GeneratePath();

  {
    InMemorySegment segment{0x2478, path, 100};
    Offsets<InMemorySegment> offsets(segment);
    offsets.Insert(0xa0, 0x1234);
  }
  {
    InMemorySegment segment{0x2478, path, 100};
    Offsets<InMemorySegment> offsets(segment);
    offsets.Insert(0xb0, 0xffaa);
  }
  {
    InMemorySegment segment{0x2478, path, 100};
    Offsets<InMemorySegment> offsets(segment);

    uint32_t id_lookup;
    EXPECT_FALSE(offsets.Lookup(0x0f, &id_lookup));

    EXPECT_TRUE(offsets.Lookup(0xa0, &id_lookup));
    EXPECT_EQ(0x1234U, id_lookup);
    EXPECT_TRUE(offsets.Lookup(0xaf, &id_lookup));
    EXPECT_EQ(0x1234U, id_lookup);

    EXPECT_TRUE(offsets.Lookup(0xb0, &id_lookup));
    EXPECT_EQ(0xffaaU, id_lookup);
    EXPECT_TRUE(offsets.Lookup(0xff, &id_lookup));
    EXPECT_EQ(0xffaaU, id_lookup);
  }
}

}  // namespace wombat::log::testing
