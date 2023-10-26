//
// Created by jinhai on 23-1-24.
//

#include "unit_test/base_test.h"

import infinity_exception;
import infinity_assert;
import logger;
import selection;
import default_values;
import third_party;
import stl;
import global_resource_usage;
import infinity;

class SelectionTest : public BaseTest {};

TEST_F(SelectionTest, test1) {
    using namespace infinity;

    Selection s1;
    EXPECT_THROW(s1.Append(1), ExecutorException);
    EXPECT_THROW(s1.Size(), ExecutorException);
    EXPECT_THROW(s1.Capacity(), ExecutorException);

    EXPECT_THROW(s1.Initialize(std::numeric_limits<u16>::max() + 1), ExecutorException);

    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        EXPECT_EQ(s1.Get(i), i);
    }

    s1.Initialize();
    EXPECT_EQ(s1.Size(), 0);
    EXPECT_EQ(s1.Capacity(), DEFAULT_VECTOR_SIZE);
    EXPECT_THROW(s1.Get(DEFAULT_VECTOR_SIZE), ExecutorException);
    EXPECT_THROW(s1.Get(0), ExecutorException);

    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        s1.Append(i * 2);
    }
    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        EXPECT_EQ(s1.Get(i), 2 * i);
    }
    EXPECT_EQ(s1.Size(), DEFAULT_VECTOR_SIZE);
    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        s1[i] = 3 * i;
    }
    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        EXPECT_EQ(s1.Get(i), 3 * i);
    }
    EXPECT_THROW(s1.Get(DEFAULT_VECTOR_SIZE), ExecutorException);
    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        s1.Set(i, 4 * i);
    }
    for (SizeT i = 0; i < DEFAULT_VECTOR_SIZE; ++i) {
        EXPECT_EQ(s1.Get(i), 4 * i);
    }
}
