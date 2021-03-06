//
// MIT License
//
// Copyright (c) 2017-2018 Thibault Martinez and Simon Ninon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#include <gtest/gtest.h>

#include <iota/models/neighbor.hpp>

TEST(Neighbor, CtorDefault) {
  IOTA::Models::Neighbor n;

  EXPECT_EQ(n.getAddress(), "");
  EXPECT_EQ(n.getNumberOfAllTransactions(), 0);
  EXPECT_EQ(n.getNumberOfInvalidTransactions(), 0);
  EXPECT_EQ(n.getNumberOfNewTransactions(), 0);
}

TEST(Neighbor, CtorFull) {
  IOTA::Models::Neighbor n = { "addr", 1, 2, 3 };

  EXPECT_EQ(n.getAddress(), "addr");
  EXPECT_EQ(n.getNumberOfAllTransactions(), 1);
  EXPECT_EQ(n.getNumberOfInvalidTransactions(), 2);
  EXPECT_EQ(n.getNumberOfNewTransactions(), 3);
}

TEST(Neighbor, ConstGetters) {
  const IOTA::Models::Neighbor n = { "addr", 1, 2, 3 };

  EXPECT_EQ(n.getAddress(), "addr");
  EXPECT_EQ(n.getNumberOfAllTransactions(), 1);
  EXPECT_EQ(n.getNumberOfInvalidTransactions(), 2);
  EXPECT_EQ(n.getNumberOfNewTransactions(), 3);
}

TEST(Neighbor, AddressGetterAndSetter) {
  IOTA::Models::Neighbor n;
  EXPECT_EQ(n.getAddress(), "");

  n.setAddress("addr");
  EXPECT_EQ(n.getAddress(), "addr");
}

TEST(Neighbor, NumberOfAllTransactionsGetterAndSetter) {
  IOTA::Models::Neighbor n;
  EXPECT_EQ(n.getNumberOfAllTransactions(), 0);

  n.setNumberOfAllTransactions(1);
  EXPECT_EQ(n.getNumberOfAllTransactions(), 1);
}

TEST(Neighbor, NumberOfInvalidTransactionsGetterAndSetter) {
  IOTA::Models::Neighbor n;
  EXPECT_EQ(n.getNumberOfInvalidTransactions(), 0);

  n.setNumberOfInvalidTransactions(1);
  EXPECT_EQ(n.getNumberOfInvalidTransactions(), 1);
}

TEST(Neighbor, NumberOfNewTransactionsGetterAndSetter) {
  IOTA::Models::Neighbor n;
  EXPECT_EQ(n.getNumberOfNewTransactions(), 0);

  n.setNumberOfNewTransactions(1);
  EXPECT_EQ(n.getNumberOfNewTransactions(), 1);
}

TEST(Neighbor, EqOperator) {
  IOTA::Models::Neighbor lhs_eq_1("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_1("addr", 1, 2, 3);
  EXPECT_TRUE(lhs_eq_1 == rhs_eq_1);

  IOTA::Models::Neighbor lhs_neq("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_neq("neq_addr", 1, 2, 3);
  EXPECT_FALSE(lhs_neq == rhs_neq);

  IOTA::Models::Neighbor lhs_eq_2("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_2("addr", 4, 2, 3);
  EXPECT_TRUE(lhs_eq_2 == rhs_eq_2);

  IOTA::Models::Neighbor lhs_eq_3("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_3("addr", 1, 4, 3);
  EXPECT_TRUE(lhs_eq_3 == rhs_eq_3);

  IOTA::Models::Neighbor lhs_eq_4("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_4("addr", 1, 2, 4);
  EXPECT_TRUE(lhs_eq_4 == rhs_eq_4);
}

TEST(Neighbor, NEqOperator) {
  IOTA::Models::Neighbor lhs_eq_1("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_1("addr", 1, 2, 3);
  EXPECT_FALSE(lhs_eq_1 != rhs_eq_1);

  IOTA::Models::Neighbor lhs_neq("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_neq("neq_addr", 1, 2, 3);
  EXPECT_TRUE(lhs_neq != rhs_neq);

  IOTA::Models::Neighbor lhs_eq_2("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_2("addr", 4, 2, 3);
  EXPECT_FALSE(lhs_eq_2 != rhs_eq_2);

  IOTA::Models::Neighbor lhs_eq_3("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_3("addr", 1, 4, 3);
  EXPECT_FALSE(lhs_eq_3 != rhs_eq_3);

  IOTA::Models::Neighbor lhs_eq_4("addr", 1, 2, 3);
  IOTA::Models::Neighbor rhs_eq_4("addr", 1, 2, 4);
  EXPECT_FALSE(lhs_eq_4 != rhs_eq_4);
}
