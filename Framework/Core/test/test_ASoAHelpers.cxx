// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/ASoAHelpers.h"
#include "Framework/TableBuilder.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ExpressionHelpers.h"
#include <catch_amalgamated.hpp>

using namespace o2::framework;
using namespace o2::soa;

namespace test
{
DECLARE_SOA_COLUMN_FULL(X, x, int32_t, "x");
DECLARE_SOA_COLUMN_FULL(Y, y, int32_t, "y");
DECLARE_SOA_COLUMN_FULL(Z, z, int32_t, "z");
DECLARE_SOA_COLUMN_FULL(FloatZ, floatZ, float, "floatZ");
DECLARE_SOA_DYNAMIC_COLUMN(Sum, sum, [](int32_t x, int32_t y) { return x + y; });
} // namespace test

TEST_CASE("IteratorTuple")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t>({"x", "y"});
  rowWriterA(0, 0, 0);
  rowWriterA(0, 1, 0);
  rowWriterA(0, 2, 0);
  rowWriterA(0, 3, 0);
  rowWriterA(0, 4, 0);
  rowWriterA(0, 5, 0);
  rowWriterA(0, 6, 0);
  rowWriterA(0, 7, 0);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 8);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;

  TestA tests{tableA};

  REQUIRE(8 == tests.size());

  TestA::iterator beginIt = tests.begin();
  REQUIRE(!(static_cast<test::X>(beginIt).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(beginIt).getIterator().mCurrentPos) == 0);
  REQUIRE(beginIt.x() == 0);
  REQUIRE(beginIt.mRowIndex == 0);

  auto beginIterators = std::make_tuple(beginIt, beginIt);
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginIterators)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginIterators)).getIterator().mCurrentPos) == 0);
  REQUIRE(std::get<0>(beginIterators).x() == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginIterators)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginIterators)).getIterator().mCurrentPos) == 0);
  REQUIRE(std::get<1>(beginIterators).x() == 0);

  auto maxIt0 = tests.begin() + 8 - 2 + 1;
  auto maxIt1 = tests.begin() + 8 - 2 + 1 + 1;
  auto maxOffset2 = std::make_tuple(maxIt0, maxIt1);
  REQUIRE(!(static_cast<test::X>(std::get<0>(maxOffset2)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(maxOffset2)).getIterator().mCurrentPos) == 7);
  REQUIRE(std::get<0>(maxOffset2).x() == 7);
  REQUIRE(!(static_cast<test::X>(std::get<1>(maxOffset2)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(maxOffset2)).getIterator().mCurrentPos) == 8);

  expressions::Filter filter = test::x > 3;
  auto filtered = Filtered<TestA>{{tests.asArrowTable()}, o2::framework::expressions::createSelection(tests.asArrowTable(), filter)};
  std::tuple<Filtered<TestA>, Filtered<TestA>> filteredTuple = std::make_tuple(filtered, filtered);

  auto it1 = std::get<0>(filteredTuple).begin();
  REQUIRE(!(static_cast<test::X>(it1).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(it1).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(it1).getIterator().mCurrentChunk == 0);
  auto it2(it1);
  REQUIRE(!(static_cast<test::X>(it2).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(it2).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(it2).getIterator().mCurrentChunk == 0);
  auto it3 = std::get<1>(filteredTuple).begin();
  REQUIRE(!(static_cast<test::X>(it3).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(it3).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(it3).getIterator().mCurrentChunk == 0);
}

TEST_CASE("CombinationsGeneratorConstruction")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t, float>({"x", "y", "floatZ"});
  rowWriterA(0, 0, 25, -6.0f);
  rowWriterA(0, 1, 18, 0.0f);
  rowWriterA(0, 2, 48, 8.0f);
  rowWriterA(0, 3, 103, 2.0f);
  rowWriterA(0, 4, 28, -6.0f);
  rowWriterA(0, 5, 102, 2.0f);
  rowWriterA(0, 6, 12, 0.0f);
  rowWriterA(0, 7, 24, -7.0f);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 8);

  TableBuilder builderB;
  auto rowWriterB = builderB.persist<int32_t>({"x"});
  rowWriterB(0, 8);
  rowWriterB(0, 9);
  rowWriterB(0, 10);
  rowWriterB(0, 11);
  auto tableB = builderB.finalize();
  REQUIRE(tableB->num_rows() == 4);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y, test::FloatZ>;
  using TestB = InPlaceTable<0, o2::soa::Index<>, test::X>;
  using ConcatTest = Concat<TestA, TestB>;

  TestA testsA{tableA};
  TestB testsB{tableB};
  ConcatTest concatTests{tableA, tableB};
  REQUIRE(8 == testsA.size());
  REQUIRE(12 == concatTests.size());

  // Grouped data:
  // [3, 5] [0, 4, 7], [1, 6], [2]
  // Assuming bins intervals: [ , )
  std::vector<double> yBins{VARIABLE_WIDTH, 0, 5, 10, 20, 30, 40, 50, 101};
  std::vector<double> zBins{VARIABLE_WIDTH, -7.0, -5.0, -3.0, -1.0, 1.0, 3.0, 5.0, 7.0};

  ColumnBinningPolicy<test::Y, test::FloatZ> pairBinning{{yBins, zBins}, false};

  CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<TestA, TestA>>::CombinationsIterator combIt(CombinationsStrictlyUpperIndexPolicy(testsA, testsA));
  REQUIRE(!(static_cast<test::X>(std::get<0>(*(combIt))).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(*(combIt))).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(*(combIt))).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(*(combIt))).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(*(combIt))).getIterator().mCurrentPos) == 1);
  REQUIRE(static_cast<test::X>(std::get<1>(*(combIt))).getIterator().mCurrentChunk == 0);

  auto comb2 = combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsA));

  static_assert(std::same_as<decltype(comb2.begin()), CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<TestA, TestA>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(comb2.begin())), CombinationsStrictlyUpperIndexPolicy<TestA, TestA>::CombinationType&>, "Wrong combination type");

  auto beginCombination = *(comb2.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(beginCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginCombination)).getIterator().mCurrentPos) == 1);
  REQUIRE(static_cast<test::X>(std::get<1>(beginCombination)).getIterator().mCurrentChunk == 0);

  REQUIRE(!(comb2.begin() == comb2.end()));

  auto endCombination = *(comb2.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endCombination)).getIterator().mCurrentPos) == 8);
  REQUIRE(static_cast<test::X>(std::get<0>(endCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endCombination)).getIterator().mCurrentPos) == 8);
  REQUIRE(static_cast<test::X>(std::get<1>(endCombination)).getIterator().mCurrentChunk == 0);

  o2::framework::expressions::Filter filter = test::x > 3;
  auto s1 = o2::framework::expressions::createSelection(testsA.asArrowTable(), filter);
  auto filtered = Filtered<TestA>{{testsA.asArrowTable()}, s1};

  CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<Filtered<TestA>, Filtered<TestA>>>::CombinationsIterator combItFiltered(CombinationsStrictlyUpperIndexPolicy(filtered, filtered));
  REQUIRE(!(static_cast<test::X>(std::get<0>(*(combItFiltered))).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(*(combItFiltered))).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<0>(*(combItFiltered))).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(*(combItFiltered))).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(*(combItFiltered))).getIterator().mCurrentPos) == 5);
  REQUIRE(static_cast<test::X>(std::get<1>(*(combItFiltered))).getIterator().mCurrentChunk == 0);

  auto comb2Filter = combinations(CombinationsStrictlyUpperIndexPolicy<TestA, TestA>(), filter, testsA, testsA);

  static_assert(std::same_as<decltype(comb2Filter.begin()), CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<Filtered<TestA>, Filtered<TestA>>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(comb2Filter.begin())), CombinationsStrictlyUpperIndexPolicy<Filtered<TestA>, Filtered<TestA>>::CombinationType&>, "Wrong combination type");

  auto beginFilterCombination = *(comb2Filter.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginFilterCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginFilterCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<0>(beginFilterCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginFilterCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginFilterCombination)).getIterator().mCurrentPos) == 5);
  REQUIRE(static_cast<test::X>(std::get<1>(beginFilterCombination)).getIterator().mCurrentChunk == 0);

  REQUIRE(!(comb2Filter.begin() == comb2Filter.end()));

  auto endFilterCombination = *(comb2Filter.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endFilterCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endFilterCombination)).getIterator().mCurrentPos) == -1);
  REQUIRE(static_cast<test::X>(std::get<0>(endFilterCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endFilterCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endFilterCombination)).getIterator().mCurrentPos) == -1);
  REQUIRE(static_cast<test::X>(std::get<1>(endFilterCombination)).getIterator().mCurrentChunk == 0);

  auto comb2Concat = combinations(CombinationsStrictlyUpperIndexPolicy(concatTests, concatTests));

  static_assert(std::same_as<decltype(comb2Concat.begin()), CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<ConcatTest, ConcatTest>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(comb2Concat.begin())), CombinationsStrictlyUpperIndexPolicy<ConcatTest, ConcatTest>::CombinationType&>, "Wrong combination type");

  auto beginConcatCombination = *(comb2Concat.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginConcatCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginConcatCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(beginConcatCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginConcatCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginConcatCombination)).getIterator().mCurrentPos) == 1);
  REQUIRE(static_cast<test::X>(std::get<1>(beginConcatCombination)).getIterator().mCurrentChunk == 0);

  REQUIRE(!(comb2Concat.begin() == comb2Concat.end()));

  // Looks that mCurrentChunk is reset to 0 if an iterator goes too far
  // (the iterators before the end() have correct chunk numbers)
  auto endConcatCombination = *(comb2Concat.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endConcatCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endConcatCombination)).getIterator().mCurrentPos) == 12);
  REQUIRE(static_cast<test::X>(std::get<0>(endConcatCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endConcatCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endConcatCombination)).getIterator().mCurrentPos) == 12);
  REQUIRE(static_cast<test::X>(std::get<1>(endConcatCombination)).getIterator().mCurrentChunk == 0);

  auto comb2Diff = combinations(CombinationsFullIndexPolicy(testsA, testsB));

  static_assert(std::same_as<decltype(comb2Diff.begin()), CombinationsGenerator<CombinationsFullIndexPolicy<TestA, TestB>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(comb2Diff.begin())), CombinationsFullIndexPolicy<TestA, TestB>::CombinationType&>, "Wrong combination type");

  auto beginDiffCombination = *(comb2Diff.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginDiffCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginDiffCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(beginDiffCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginDiffCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginDiffCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<1>(beginDiffCombination)).getIterator().mCurrentChunk == 0);

  REQUIRE(!(comb2Diff.begin() == comb2Diff.end()));

  auto endDiffCombination = *(comb2Diff.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endDiffCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endDiffCombination)).getIterator().mCurrentPos) == 8);
  REQUIRE(static_cast<test::X>(std::get<0>(endDiffCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endDiffCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endDiffCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<1>(endDiffCombination)).getIterator().mCurrentChunk == 0);

  // More elements required for a combination than number of elements in the table
  auto comb2Bad = combinations(CombinationsStrictlyUpperIndexPolicy(testsB, testsB, testsB, testsB, testsB));

  static_assert(std::same_as<decltype(comb2Bad.begin()), CombinationsGenerator<CombinationsStrictlyUpperIndexPolicy<TestB, TestB, TestB, TestB, TestB>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(comb2Bad.begin())), CombinationsStrictlyUpperIndexPolicy<TestB, TestB, TestB, TestB, TestB>::CombinationType&>, "Wrong combination type");

  auto beginBadCombination = *(comb2Bad.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginBadCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(beginBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginBadCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<1>(beginBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<2>(beginBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<2>(beginBadCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<2>(beginBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<3>(beginBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<3>(beginBadCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<3>(beginBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<4>(beginBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<4>(beginBadCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<4>(beginBadCombination)).getIterator().mCurrentChunk == 0);

  //  REQUIRE(comb2Bad.begin() == comb2Bad.end());;

  auto endBadCombination = *(comb2Bad.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endBadCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<0>(endBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endBadCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<1>(endBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<2>(endBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<2>(endBadCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<2>(endBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<3>(endBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<3>(endBadCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<3>(endBadCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<4>(endBadCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<4>(endBadCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<4>(endBadCombination)).getIterator().mCurrentChunk == 0);

  auto combBlock = combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 2, -1, testsA, testsA));

  static_assert(std::same_as<decltype(combBlock.begin()), CombinationsGenerator<CombinationsBlockStrictlyUpperSameIndexPolicy<ColumnBinningPolicy<test::Y, test::FloatZ>, int32_t, TestA, TestA>>::CombinationsIterator>, "Wrong iterator type");
  static_assert(std::same_as<decltype(*(combBlock.begin())), CombinationsBlockStrictlyUpperSameIndexPolicy<ColumnBinningPolicy<test::Y, test::FloatZ>, int32_t, TestA, TestA>::CombinationType&>, "Wrong combination type");

  auto beginBlockCombination = *(combBlock.begin());
  REQUIRE(!(static_cast<test::X>(std::get<0>(beginBlockCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(beginBlockCombination)).getIterator().mCurrentPos) == 0);
  REQUIRE(static_cast<test::X>(std::get<0>(beginBlockCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(beginBlockCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(beginBlockCombination)).getIterator().mCurrentPos) == 4);
  REQUIRE(static_cast<test::X>(std::get<1>(beginBlockCombination)).getIterator().mCurrentChunk == 0);

  REQUIRE(!(combBlock.begin() == combBlock.end()));

  auto endBlockCombination = *(combBlock.end());
  REQUIRE(!(static_cast<test::X>(std::get<0>(endBlockCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<0>(endBlockCombination)).getIterator().mCurrentPos) == 8);
  REQUIRE(static_cast<test::X>(std::get<0>(endBlockCombination)).getIterator().mCurrentChunk == 0);
  REQUIRE(!(static_cast<test::X>(std::get<1>(endBlockCombination)).getIterator().mCurrentPos == nullptr));
  REQUIRE(*(static_cast<test::X>(std::get<1>(endBlockCombination)).getIterator().mCurrentPos) == 8);
  REQUIRE(static_cast<test::X>(std::get<1>(endBlockCombination)).getIterator().mCurrentChunk == 0);
}

TEST_CASE("Combinations")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t>({"x", "y"});
  rowWriterA(0, 0, 0);
  rowWriterA(0, 1, 0);
  rowWriterA(0, 2, 0);
  rowWriterA(0, 3, 0);
  rowWriterA(0, 4, 0);
  rowWriterA(0, 5, 0);
  rowWriterA(0, 6, 0);
  rowWriterA(0, 7, 0);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 8);

  TableBuilder builderB;
  auto rowWriterB = builderB.persist<int32_t>({"x"});
  rowWriterB(0, 8);
  rowWriterB(0, 9);
  rowWriterB(0, 10);
  rowWriterB(0, 11);
  auto tableB = builderB.finalize();
  REQUIRE(tableB->num_rows() == 4);

  TableBuilder builderC;
  auto rowWriterC = builderC.persist<int32_t, int32_t, int32_t>({"x", "y", "z"});
  rowWriterC(0, 12, 0, 0);
  rowWriterC(0, 13, 0, 0);
  rowWriterC(0, 14, 0, 0);
  rowWriterC(0, 15, 0, 0);
  auto tableC = builderC.finalize();
  REQUIRE(tableC->num_rows() == 4);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;
  using TestB = InPlaceTable<0, o2::soa::Index<>, test::X>;
  using TestC = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y, test::Z>;
  using ConcatTest = Concat<TestA, TestB>;

  TestA testsA{tableA};
  TestB testsB{tableB};
  TestC testsC{tableC};
  ConcatTest concatTests{tableA, tableB};

  REQUIRE(8 == testsA.size());
  REQUIRE(4 == testsB.size());
  REQUIRE(4 == testsC.size());
  REQUIRE(12 == concatTests.size());
  int nA = testsA.size();
  int nB = testsB.size();
  int nC = testsC.size();

  int count = 0;
  int i = 0;
  int j = 1;
  for (auto& [t0, t1] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 28);

  expressions::Filter pairsFilter = test::x > 3;

  count = 0;
  i = 4;
  j = 5;
  for (auto& [t0, t1] : combinations(CombinationsStrictlyUpperIndexPolicy<TestA, TestA>(), pairsFilter, testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 6);

  count = 0;
  i = 0;
  j = 1;
  int k = 2;
  for (auto& [t0, t1, t2] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsA, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA) {
      if (j == nA - 2) {
        i++;
        j = i;
      }
      j++;
      k = j + 1;
    }
  }
  REQUIRE(count == 56);

  expressions::Filter triplesFilter = test::x < 4;

  count = 0;
  i = 0;
  j = 1;
  k = 2;
  for (auto& [t0, t1, t2] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsA, testsA), triplesFilter, testsA, testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == 4) {
      if (j == 2) {
        i++;
        j = i;
      }
      j++;
      k = j + 1;
    }
  }
  REQUIRE(count == 4);

  int nConcat = concatTests.size();

  count = 0;
  i = 0;
  j = 1;
  for (auto [t0, t1] : combinations(CombinationsStrictlyUpperIndexPolicy(concatTests, concatTests))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(static_cast<test::X>(t0).getIterator().mCurrentChunk == (i < nA ? 0 : 1));
    REQUIRE(static_cast<test::X>(t1).getIterator().mCurrentChunk == (j < nA ? 0 : 1));
    count++;
    j++;
    if (j == nConcat) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 66);

  count = 0;
  i = 0;
  j = 1;
  k = 2;
  for (auto [t0, t1, t2] : combinations(CombinationsStrictlyUpperIndexPolicy(concatTests, concatTests, concatTests))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    REQUIRE(static_cast<test::X>(t0).getIterator().mCurrentChunk == (i < nA ? 0 : 1));
    REQUIRE(static_cast<test::X>(t1).getIterator().mCurrentChunk == (j < nA ? 0 : 1));
    REQUIRE(static_cast<test::X>(t2).getIterator().mCurrentChunk == (k < nA ? 0 : 1));
    count++;
    k++;
    if (k == nConcat) {
      if (j == nConcat - 2) {
        i++;
        j = i;
      }
      j++;
      k = j + 1;
    }
  }
  REQUIRE(count == 220);

  count = 0;
  i = 0;
  j = 1;
  k = 2;
  int l = 3;
  int m = 4;
  for (auto& [t0, t1, t2, t3, t4] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsA, testsA, testsA, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    REQUIRE(t3.x() == l);
    REQUIRE(t4.x() == m);
    count++;
    m++;
    if (m == nA) {
      if (l == nA - 2) {
        if (k == nA - 3) {
          if (j == nA - 4) {
            i++;
            j = i;
          }
          j++;
          k = j;
        }
        k++;
        l = k;
      }
      l++;
      m = l + 1;
    }
  }
  REQUIRE(count == 56);

  // Combinations shortcut
  count = 0;
  i = 0;
  j = 1;
  for (auto& [t0, t1] : combinations(testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 28);

  count = 0;
  i = 4;
  j = 5;
  for (auto& [t0, t1] : combinations(pairsFilter, testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 6);

  count = 0;
  i = 0;
  j = 0 + nA;
  for (auto& [t0, t1] : combinations(testsA, testsB)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA + nB) {
      i++;
      j = nA + i;
    }
  }
  REQUIRE(count == 10);

  // Different tables of different size
  count = 0;
  i = 0;
  j = 0 + nA;
  for (auto& [t0, t1] : combinations(CombinationsFullIndexPolicy(testsA, testsB))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA + nB) {
      i++;
      j = 0 + nA;
    }
  }
  REQUIRE(count == 32);

  count = 0;
  i = 0 + nA;
  j = 0;
  for (auto& [t0, t1] : combinations(CombinationsFullIndexPolicy(testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA) {
      i++;
      j = 0;
    }
  }
  REQUIRE(count == 32);

  count = 0;
  i = 0;
  j = 0 + nA;
  k = 0 + nA + nB;
  for (auto& [t0, t1, t2] : combinations(CombinationsFullIndexPolicy(testsA, testsB, testsC))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA + nB + nC) {
      if (j == nA + nB - 1) {
        i++;
        j = 0 + nA;
      } else {
        j++;
      }
      k = 0 + nA + nB;
    }
  }
  REQUIRE(count == 128);

  count = 0;
  i = 0 + nA + nB;
  j = 0 + nA;
  k = 0;
  for (auto& [t0, t1, t2] : combinations(CombinationsFullIndexPolicy(testsC, testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA) {
      if (j == nA + nB - 1) {
        i++;
        j = 0 + nA;
      } else {
        j++;
      }
      k = 0;
    }
  }
  REQUIRE(count == 128);

  count = 0;
  i = 0;
  j = 0 + nA;
  for (auto& [t0, t1] : combinations(CombinationsUpperIndexPolicy(testsA, testsB))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA + nB) {
      i++;
      j = nA + i;
    }
  }
  REQUIRE(count == 10);

  count = 0;
  i = 0 + nA;
  j = 0;
  for (auto& [t0, t1] : combinations(CombinationsUpperIndexPolicy(testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA) {
      i++;
      j = -nA + i;
    }
  }
  REQUIRE(count == 26);

  count = 0;
  i = 0;
  j = 0 + nA;
  k = 0 + nA + nB;
  for (auto& [t0, t1, t2] : combinations(CombinationsUpperIndexPolicy(testsA, testsB, testsC))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA + nB + nC) {
      if (j == nA + nB - 1) {
        i++;
        j = nA + i;
      } else {
        j++;
      }
      k = nB + j;
    }
  }
  REQUIRE(count == 20);

  count = 0;
  i = 0 + nA + nB;
  j = 0 + nA;
  k = 0;
  for (auto& [t0, t1, t2] : combinations(CombinationsUpperIndexPolicy(testsC, testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA) {
      if (j == nA + nB - 1) {
        i++;
        j = -nB + i;
      } else {
        j++;
      }
      k = -nA + j;
    }
  }
  REQUIRE(count == 60);

  count = 0;
  i = 0;
  j = 0 + nA + 1;
  for (auto& [t0, t1] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsB))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA + nB) {
      i++;
      j = nA + i + 1;
    }
  }
  REQUIRE(count == 6);

  count = 0;
  i = 0 + nA;
  j = 1;
  for (auto& [t0, t1] : combinations(CombinationsStrictlyUpperIndexPolicy(testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == 0 + nA) {
      i++;
      j = -nA + i + 1;
    }
  }
  REQUIRE(count == 18);

  count = 0;
  i = 0;
  j = 0 + nA + 1;
  k = 0 + nA + nB + 2;
  for (auto& [t0, t1, t2] : combinations(CombinationsStrictlyUpperIndexPolicy(testsA, testsB, testsC))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA + nB + nC) {
      if (j == nA + nB - 2) {
        i++;
        j = nA + i + 1;
      } else {
        j++;
      }
      k = nB + j + 1;
    }
  }
  REQUIRE(count == 4);

  count = 0;
  i = 0 + nA + nB;
  j = 0 + nA + 1;
  k = 2;
  for (auto& [t0, t1, t2] : combinations(CombinationsStrictlyUpperIndexPolicy(testsC, testsB, testsA))) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA) {
      if (j == nA + nB - 2) {
        i++;
        j = -nB + i + 1;
      } else {
        j++;
      }
      k = -nA + j + 1;
    }
  }
  REQUIRE(count == 16);
}

TEST_CASE("BreakingCombinations")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t>({"x", "y"});
  rowWriterA(0, 0, 0);
  rowWriterA(0, 1, 0);
  rowWriterA(0, 2, 0);
  rowWriterA(0, 3, 0);
  rowWriterA(0, 4, 0);
  rowWriterA(0, 5, 0);
  rowWriterA(0, 6, 0);
  rowWriterA(0, 7, 0);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 8);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;

  TestA testsA{tableA};

  REQUIRE(8 == testsA.size());
  int nA = testsA.size();

  int count = 0;
  int i = 0;
  int j = 1;
  for (auto& [t0, t1] : combinations(testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
    if (t0.x() == 4) {
      continue;
      REQUIRE(!(true == true));
    }
    REQUIRE(!(t0.x() == 4));
  }
  REQUIRE(count == 28);

  count = 0;
  i = 0;
  j = 1;
  for (auto& [t0, t1] : combinations(testsA, testsA)) {
    if (t0.x() == 4) {
      break;
      REQUIRE(!(true == true));
    }
    REQUIRE(t0.x() < 4);
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 22);
}

TEST_CASE("SmallTableCombinations")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t>({"x", "y"});
  rowWriterA(0, 0, 0);
  rowWriterA(0, 1, 0);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 2);

  TableBuilder builderB;
  auto rowWriterB = builderB.persist<int32_t>({"x"});
  rowWriterB(0, 8);
  rowWriterB(0, 9);
  rowWriterB(0, 10);
  auto tableB = builderB.finalize();
  REQUIRE(tableB->num_rows() == 3);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;
  using TestB = InPlaceTable<0, o2::soa::Index<>, test::X>;

  TestA testsA{tableA};
  TestB testsB{tableB};

  REQUIRE(2 == testsA.size());
  REQUIRE(3 == testsB.size());
  [[maybe_unused]] int nA = testsA.size();
  [[maybe_unused]] int nB = testsB.size();

  int count = 0;
  int i = 0;
  int j = 1;
  for (auto& [t0, t1] : combinations(testsA, testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
  }
  REQUIRE(count == 1);

  count = 0;
  i = 8;
  j = 9;
  int k = 10;
  for (auto& [t0, t1, t2] : combinations(testsB, testsB, testsB)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
  }
  REQUIRE(count == 1);

  count = 0;
  for (auto& [t0, t1, t2] : combinations(testsA, testsA, testsA)) {
    count++;
  }
  REQUIRE(count == 0);
}

TEST_CASE("BlockCombinations")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t, float>({"x", "y", "floatZ"});
  rowWriterA(0, 0, 25, -6.0f);
  rowWriterA(0, 1, 18, 0.0f);
  rowWriterA(0, 2, 48, 8.0f);
  rowWriterA(0, 3, 103, 2.0f);
  rowWriterA(0, 4, 28, -6.0f);
  rowWriterA(0, 5, 102, 2.0f);
  rowWriterA(0, 6, 12, 0.0f);
  rowWriterA(0, 7, 24, -7.0f);
  rowWriterA(0, 8, 41, 8.0f);
  rowWriterA(0, 9, 49, 8.0f);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 10);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y, test::FloatZ>;
  TestA testA{tableA};
  REQUIRE(10 == testA.size());

  TableBuilder builderAHalf;
  auto rowWriterAHalf = builderAHalf.persist<int32_t, int32_t, float>({"x", "y", "floatZ"});
  rowWriterAHalf(0, 0, 25, -6.0f);
  rowWriterAHalf(0, 1, 18, 0.0f);
  rowWriterAHalf(0, 2, 48, 8.0f);
  rowWriterAHalf(0, 3, 103, 2.0f);
  rowWriterAHalf(0, 4, 28, -6.0f);
  auto tableAHalf = builderAHalf.finalize();
  REQUIRE(tableAHalf->num_rows() == 5);

  TestA testAHalf{tableAHalf};
  REQUIRE(5 == testAHalf.size());

  // Grouped data:
  // [3, 5] [0, 4, 7], [1, 6], [2, 8, 9]
  // Assuming bins intervals: [ , )
  std::vector<double> yBins{VARIABLE_WIDTH, 0, 5, 10, 20, 30, 40, 50, 101};
  std::vector<double> zBins{VARIABLE_WIDTH, -7.0, -5.0, -3.0, -1.0, 1.0, 3.0, 5.0, 7.0};

  ColumnBinningPolicy<test::Y, test::FloatZ> pairBinning{{yBins, zBins}, false};
  ColumnBinningPolicy<test::Y, test::FloatZ> pairBinningNoOverflows{{yBins, zBins}, true};

  // 2, 3, 5, 8, 9 have overflows in testA
  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairsNoOverflows{
    {0, 0}, {0, 4}, {4, 0}, {4, 4}, {4, 7}, {7, 4}, {7, 7}, {1, 1}, {1, 6}, {6, 1}, {6, 6}};
  size_t count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockFullIndexPolicy(pairBinningNoOverflows, 1, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullPairsNoOverflows[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullPairsNoOverflows[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairsNoOverflows.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairs{
    {0, 0}, {0, 4}, {0, 7}, {4, 0}, {7, 0}, {4, 4}, {4, 7}, {7, 4}, {7, 7}, {1, 1}, {1, 6}, {6, 1}, {6, 6}, {3, 3}, {3, 5}, {5, 3}, {5, 5}, {2, 2}, {2, 8}, {2, 9}, {8, 2}, {9, 2}, {8, 8}, {8, 9}, {9, 8}, {9, 9}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockFullIndexPolicy(pairBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullPairs[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairs.size());

  std::vector<std::tuple<int32_t, int32_t, int32_t>> expectedFullTriples{
    {0, 0, 0}, {0, 0, 4}, {0, 0, 7}, {0, 4, 0}, {0, 4, 4}, {0, 4, 7}, {0, 7, 0}, {0, 7, 4}, {0, 7, 7}, {4, 0, 0}, {4, 0, 4}, {4, 0, 7}, {7, 0, 0}, {7, 0, 4}, {7, 0, 7}, {4, 4, 0}, {4, 7, 0}, {7, 4, 0}, {7, 7, 0}, {4, 4, 4}, {4, 4, 7}, {4, 7, 4}, {4, 7, 7}, {7, 4, 4}, {7, 4, 7}, {7, 7, 4}, {7, 7, 7}, {1, 1, 1}, {1, 1, 6}, {1, 6, 1}, {1, 6, 6}, {6, 1, 1}, {6, 1, 6}, {6, 6, 1}, {6, 6, 6}, {3, 3, 3}, {3, 3, 5}, {3, 5, 3}, {3, 5, 5}, {5, 3, 3}, {5, 3, 5}, {5, 5, 3}, {5, 5, 5}, {2, 2, 2}, {2, 2, 8}, {2, 2, 9}, {2, 8, 2}, {2, 8, 8}, {2, 8, 9}, {2, 9, 2}, {2, 9, 8}, {2, 9, 9}, {8, 2, 2}, {8, 2, 8}, {8, 2, 9}, {9, 2, 2}, {9, 2, 8}, {9, 2, 9}, {8, 8, 2}, {8, 9, 2}, {9, 8, 2}, {9, 9, 2}, {8, 8, 8}, {8, 8, 9}, {8, 9, 8}, {8, 9, 9}, {9, 8, 8}, {9, 8, 9}, {9, 9, 8}, {9, 9, 9}};
  count = 0;
  for (auto& [c0, c1, c2] : combinations(CombinationsBlockFullIndexPolicy(pairBinning, 2, -1, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedFullTriples[count]));
    count++;
  }
  REQUIRE(count == expectedFullTriples.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedUpperPairs{
    {0, 0}, {0, 4}, {0, 7}, {4, 4}, {4, 7}, {7, 7}, {1, 1}, {1, 6}, {6, 6}, {3, 3}, {3, 5}, {5, 5}, {2, 2}, {2, 8}, {2, 9}, {8, 8}, {8, 9}, {9, 9}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockUpperIndexPolicy(pairBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperPairs[count]));
    count++;
  }
  REQUIRE(count == expectedUpperPairs.size());

  std::vector<std::tuple<int32_t, int32_t, int32_t>> expectedUpperTriples{
    {0, 0, 0}, {0, 0, 4}, {0, 4, 4}, {4, 4, 4}, {4, 4, 7}, {4, 7, 7}, {7, 7, 7}, {1, 1, 1}, {1, 1, 6}, {1, 6, 6}, {6, 6, 6}, {3, 3, 3}, {3, 3, 5}, {3, 5, 5}, {5, 5, 5}, {2, 2, 2}, {2, 2, 8}, {2, 8, 8}, {8, 8, 8}, {8, 8, 9}, {8, 9, 9}, {9, 9, 9}};
  count = 0;
  for (auto& [c0, c1, c2] : combinations(CombinationsBlockUpperIndexPolicy(pairBinning, 1, -1, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedUpperTriples[count]));
    count++;
  }
  REQUIRE(count == expectedUpperTriples.size());

  std::vector<std::tuple<int32_t, int32_t, int32_t, int32_t, int32_t>> expectedUpperFives{{0, 0, 0, 0, 0}, {0, 0, 0, 0, 4}, {0, 0, 0, 0, 7}, {0, 0, 0, 4, 4}, {0, 0, 0, 4, 7}, {0, 0, 0, 7, 7}, {0, 0, 4, 4, 4}, {0, 0, 4, 4, 7}, {0, 0, 4, 7, 7}, {0, 0, 7, 7, 7}, {0, 4, 4, 4, 4}, {0, 4, 4, 4, 7}, {0, 4, 4, 7, 7}, {0, 4, 7, 7, 7}, {0, 7, 7, 7, 7}, {4, 4, 4, 4, 4}, {4, 4, 4, 4, 7}, {4, 4, 4, 7, 7}, {4, 4, 7, 7, 7}, {4, 7, 7, 7, 7}, {7, 7, 7, 7, 7}, {1, 1, 1, 1, 1}, {1, 1, 1, 1, 6}, {1, 1, 1, 6, 6}, {1, 1, 6, 6, 6}, {1, 6, 6, 6, 6}, {6, 6, 6, 6, 6}, {3, 3, 3, 3, 3}, {3, 3, 3, 3, 5}, {3, 3, 3, 5, 5}, {3, 3, 5, 5, 5}, {3, 5, 5, 5, 5}, {5, 5, 5, 5, 5}, {2, 2, 2, 2, 2}, {2, 2, 2, 2, 8}, {2, 2, 2, 2, 9}, {2, 2, 2, 8, 8}, {2, 2, 2, 8, 9}, {2, 2, 2, 9, 9}, {2, 2, 8, 8, 8}, {2, 2, 8, 8, 9}, {2, 2, 8, 9, 9}, {2, 2, 9, 9, 9}, {2, 8, 8, 8, 8}, {2, 8, 8, 8, 9}, {2, 8, 8, 9, 9}, {2, 8, 9, 9, 9}, {2, 9, 9, 9, 9}, {8, 8, 8, 8, 8}, {8, 8, 8, 8, 9}, {8, 8, 8, 9, 9}, {8, 8, 9, 9, 9}, {8, 9, 9, 9, 9}, {9, 9, 9, 9, 9}};
  count = 0;
  for (auto& [c0, c1, c2, c3, c4] : combinations(CombinationsBlockUpperIndexPolicy(pairBinning, 2, -1, testA, testA, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperFives[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperFives[count]));
    REQUIRE(c2.x() == std::get<2>(expectedUpperFives[count]));
    REQUIRE(c3.x() == std::get<3>(expectedUpperFives[count]));
    REQUIRE(c4.x() == std::get<4>(expectedUpperFives[count]));
    count++;
  }
  REQUIRE(count == expectedUpperFives.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedStrictlyUpperPairsSmaller{
    {0, 4}, {4, 7}, {1, 6}, {3, 5}, {2, 8}, {8, 9}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 1, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperPairsSmaller[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperPairsSmaller[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperPairsSmaller.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedStrictlyUpperPairs{
    {0, 4}, {0, 7}, {4, 7}, {1, 6}, {3, 5}, {2, 8}, {2, 9}, {8, 9}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperPairs[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperPairs.size());

  std::vector<std::tuple<int32_t, int32_t, int32_t>> expectedStrictlyUpperTriples{
    {0, 4, 7}, {2, 8, 9}};
  count = 0;
  for (auto& [c0, c1, c2] : combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 2, -1, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedStrictlyUpperTriples[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperTriples.size());

  count = 0;
  for (auto& [c0, c1, c2, c3, c4] : combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 1, -1, testA, testA, testA, testA, testA))) {
    count++;
  }
  REQUIRE(count == 0);

  // Different tables of different size
  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairsFirstSmaller{
    {0, 0}, {0, 4}, {4, 0}, {4, 4}, {4, 7}, {1, 1}, {1, 6}, {3, 3}, {3, 5}, {2, 2}, {2, 8}};
  count = 0;
  for (auto& [x0, x1] : combinations(CombinationsBlockFullIndexPolicy(pairBinning, 1, -1, testAHalf, testA))) {
    REQUIRE(x0.x() == std::get<0>(expectedFullPairsFirstSmaller[count]));
    REQUIRE(x1.x() == std::get<1>(expectedFullPairsFirstSmaller[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairsFirstSmaller.size());

  count = 0;
  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairsSecondSmaller{
    {0, 0}, {0, 4}, {4, 0}, {4, 4}, {7, 4}, {1, 1}, {6, 1}, {3, 3}, {5, 3}, {2, 2}, {8, 2}};
  for (auto& [x0, x1] : combinations(CombinationsBlockFullIndexPolicy(pairBinning, 1, -1, testA, testAHalf))) {
    REQUIRE(x0.x() == std::get<0>(expectedFullPairsSecondSmaller[count]));
    REQUIRE(x1.x() == std::get<1>(expectedFullPairsSecondSmaller[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairsSecondSmaller.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedUpperPairsFirstSmaller{
    {0, 0}, {0, 4}, {4, 4}, {4, 7}, {1, 1}, {1, 6}, {3, 3}, {3, 5}, {2, 2}, {2, 8}};
  count = 0;
  for (auto& [x0, x1] : combinations(CombinationsBlockUpperIndexPolicy(pairBinning, 1, -1, testAHalf, testA))) {
    REQUIRE(x0.x() == std::get<0>(expectedUpperPairsFirstSmaller[count]));
    REQUIRE(x1.x() == std::get<1>(expectedUpperPairsFirstSmaller[count]));
    count++;
  }
  REQUIRE(count == expectedUpperPairsFirstSmaller.size());

  count = 0;
  std::vector<std::tuple<int32_t, int32_t>> expectedUpperPairsSecondSmaller{
    {0, 0}, {0, 4}, {4, 4}, {1, 1}, {3, 3}, {2, 2}};
  for (auto& [x0, x1] : combinations(CombinationsBlockUpperIndexPolicy(pairBinning, 1, -1, testA, testAHalf))) {
    REQUIRE(x0.x() == std::get<0>(expectedUpperPairsSecondSmaller[count]));
    REQUIRE(x1.x() == std::get<1>(expectedUpperPairsSecondSmaller[count]));
    count++;
  }
  REQUIRE(count == expectedUpperPairsSecondSmaller.size());

  // Using same index combinations for better performance
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockFullSameIndexPolicy(pairBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullPairs[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairs.size());

  count = 0;
  for (auto& [c0, c1, c2] : combinations(CombinationsBlockFullSameIndexPolicy(pairBinning, 2, -1, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedFullTriples[count]));
    count++;
  }
  REQUIRE(count == expectedFullTriples.size());

  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockUpperSameIndexPolicy(pairBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperPairs[count]));
    count++;
  }
  REQUIRE(count == expectedUpperPairs.size());

  count = 0;
  for (auto& [c0, c1, c2] : combinations(CombinationsBlockUpperSameIndexPolicy(pairBinning, 1, -1, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedUpperTriples[count]));
    count++;
  }
  REQUIRE(count == expectedUpperTriples.size());

  count = 0;
  for (auto& [c0, c1, c2, c3, c4] : combinations(CombinationsBlockUpperSameIndexPolicy(pairBinning, 2, -1, testA, testA, testA, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedUpperFives[count]));
    REQUIRE(c1.x() == std::get<1>(expectedUpperFives[count]));
    REQUIRE(c2.x() == std::get<2>(expectedUpperFives[count]));
    REQUIRE(c3.x() == std::get<3>(expectedUpperFives[count]));
    REQUIRE(c4.x() == std::get<4>(expectedUpperFives[count]));
    count++;
  }
  REQUIRE(count == expectedUpperFives.size());

  count = 0;
  for (auto& [c0, c1] : selfCombinations(pairBinning, 2, -1, testA, testA)) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperPairs[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperPairs.size());

  count = 0;
  for (auto& [c0, c1, c2] : selfCombinations(pairBinning, 2, -1, testA, testA, testA)) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedStrictlyUpperTriples[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperTriples.size());

  count = 0;
  for (auto& [c0, c1, c2, c3, c4] : selfCombinations(pairBinning, 2, -1, testA, testA, testA, testA, testA)) {
    count++;
  }
  REQUIRE(count == 0);

  // Testing bin calculations for triple binning
  // Grouped data:
  // [3, 5] [0, 4], [7], [1, 6], [2], [8, 9]
  // Assuming bins intervals: [ , )
  std::vector<double> xBins{VARIABLE_WIDTH, 0, 7, 10};
  ColumnBinningPolicy<test::X, test::Y, test::FloatZ> tripleBinning{{xBins, yBins, zBins}, false};
  ColumnBinningPolicy<test::X, test::Y, test::FloatZ> tripleBinningNoOverflows{{xBins, yBins, zBins}, true};

  // 2, 3, 5, 8, 9 have overflows in testA
  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairsTripleBinningNoOverflows{
    {0, 0}, {0, 4}, {4, 0}, {4, 4}, {7, 7}, {1, 1}, {1, 6}, {6, 1}, {6, 6}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockFullIndexPolicy(tripleBinningNoOverflows, 1, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullPairsTripleBinningNoOverflows[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullPairsTripleBinningNoOverflows[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairsTripleBinningNoOverflows.size());

  std::vector<std::tuple<int32_t, int32_t>> expectedFullPairsTripleBinning{
    {0, 0}, {0, 4}, {4, 0}, {4, 4}, {7, 7}, {1, 1}, {1, 6}, {6, 1}, {6, 6}, {3, 3}, {3, 5}, {5, 3}, {5, 5}, {2, 2}, {8, 8}, {8, 9}, {9, 8}, {9, 9}};
  count = 0;
  for (auto& [c0, c1] : combinations(CombinationsBlockFullIndexPolicy(tripleBinning, 2, -1, testA, testA))) {
    REQUIRE(c0.x() == std::get<0>(expectedFullPairsTripleBinning[count]));
    REQUIRE(c1.x() == std::get<1>(expectedFullPairsTripleBinning[count]));
    count++;
  }
  REQUIRE(count == expectedFullPairsTripleBinning.size());
}

TEST_CASE("CombinationsHelpers")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t>({"x", "y"});
  rowWriterA(0, 0, 0);
  rowWriterA(0, 1, 0);
  rowWriterA(0, 2, 0);
  rowWriterA(0, 3, 0);
  rowWriterA(0, 4, 0);
  rowWriterA(0, 5, 0);
  rowWriterA(0, 6, 0);
  rowWriterA(0, 7, 0);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 8);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;

  TestA testsA{tableA};

  REQUIRE(8 == testsA.size());
  int nA = testsA.size();

  size_t count = 0;
  int i = 0;
  int j = 1;
  for (auto& [t0, t1] : pairCombinations(testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    count++;
    j++;
    if (j == nA) {
      i++;
      j = i + 1;
    }
  }
  REQUIRE(count == 28);

  count = 0;
  i = 0;
  j = 1;
  int k = 2;
  for (auto& [t0, t1, t2] : tripleCombinations(testsA)) {
    REQUIRE(t0.x() == i);
    REQUIRE(t1.x() == j);
    REQUIRE(t2.x() == k);
    count++;
    k++;
    if (k == nA) {
      if (j == nA - 2) {
        i++;
        j = i;
      }
      j++;
      k = j + 1;
    }
  }
  REQUIRE(count == 56);

  TableBuilder builderB;
  auto rowWriterB = builderB.persist<int32_t, int32_t, float>({"x", "y", "floatZ"});
  rowWriterB(0, 0, 25, -6.0f);
  rowWriterB(0, 1, 18, 0.0f);
  rowWriterB(0, 2, 48, 8.0f);
  rowWriterB(0, 3, 103, 2.0f);
  rowWriterB(0, 4, 28, -6.0f);
  rowWriterB(0, 5, 102, 2.0f);
  rowWriterB(0, 6, 12, 0.0f);
  rowWriterB(0, 7, 24, -7.0f);
  rowWriterB(0, 8, 41, 8.0f);
  rowWriterB(0, 9, 49, 8.0f);
  auto tableB = builderB.finalize();
  REQUIRE(tableB->num_rows() == 10);

  using TestB = o2::soa::InPlaceTable<0, o2::soa::Index<>, test::X, test::Y, test::FloatZ>;
  TestB testB{tableB};
  REQUIRE(10 == testB.size());

  // Grouped data:
  // [3, 5] [0, 4, 7], [1, 6], [2, 8, 9]
  // Assuming bins intervals: [ , )
  std::vector<double> yBins{VARIABLE_WIDTH, 0, 5, 10, 20, 30, 40, 50, 101};
  std::vector<double> zBins{VARIABLE_WIDTH, -7.0, -5.0, -3.0, -1.0, 1.0, 3.0, 5.0, 7.0};

  ColumnBinningPolicy<test::Y, test::FloatZ> pairBinning{{yBins, zBins}, false};

  std::vector<std::tuple<int32_t, int32_t>> expectedStrictlyUpperPairs{
    {0, 4}, {0, 7}, {4, 7}, {1, 6}, {3, 5}, {2, 8}, {2, 9}, {8, 9}};
  count = 0;
  for (auto& [c0, c1] : selfPairCombinations(pairBinning, 2, -1, testB)) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperPairs[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperPairs[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperPairs.size());

  std::vector<std::tuple<int32_t, int32_t, int32_t>> expectedStrictlyUpperTriples{
    {0, 4, 7}, {2, 8, 9}};
  count = 0;
  for (auto& [c0, c1, c2] : selfTripleCombinations(pairBinning, 2, -1, testB)) {
    REQUIRE(c0.x() == std::get<0>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c1.x() == std::get<1>(expectedStrictlyUpperTriples[count]));
    REQUIRE(c2.x() == std::get<2>(expectedStrictlyUpperTriples[count]));
    count++;
  }
  REQUIRE(count == expectedStrictlyUpperTriples.size());
}

TEST_CASE("ConstructorsWithoutTables")
{
  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y>;
  NoBinningPolicy<test::Y> noBinning;

  size_t count = 0;
  for (auto& [t0, t1] : pairCombinations<TestA>()) {
    count++;
  }
  REQUIRE(count == 0);

  count = 0;
  for (auto& [t0, t1, t2] : tripleCombinations<TestA>()) {
    count++;
  }
  REQUIRE(count == 0);

  count = 0;
  for (auto& [c0, c1] : selfPairCombinations<NoBinningPolicy<test::Y>, int, TestA>(noBinning, 2, -1)) {
    count++;
  }
  REQUIRE(count == 0);

  count = 0;
  for (auto& [c0, c1, c2] : selfTripleCombinations<NoBinningPolicy<test::Y>, int, TestA>(noBinning, 2, -1)) {
    count++;
  }
  REQUIRE(count == 0);
}

TEST_CASE("BlockCombinationsCounters")
{
  TableBuilder builderA;
  auto rowWriterA = builderA.persist<int32_t, int32_t, float>({"x", "y", "floatZ"});
  rowWriterA(0, 0, 25, -1.3f);
  rowWriterA(0, 1, 21, -1.8f);
  rowWriterA(0, 2, 48, 2.0f);
  rowWriterA(0, 3, 26, -2.0f);
  rowWriterA(0, 4, 28, -1.5f);
  rowWriterA(0, 5, 42, 2.0f);
  rowWriterA(0, 6, 47, 2.5f);
  rowWriterA(0, 7, 24, -1.8f);
  rowWriterA(0, 8, 41, 1.3f);
  rowWriterA(0, 9, 49, 1.8f);
  auto tableA = builderA.finalize();
  REQUIRE(tableA->num_rows() == 10);

  using TestA = InPlaceTable<0, o2::soa::Index<>, test::X, test::Y, test::FloatZ>;
  TestA testA{tableA};
  REQUIRE(10 == testA.size());

  // Grouped data:
  // [0, 1, 3, 4, 7], [2, 5, 6, 8, 9]
  // Assuming bins intervals: [ , )
  std::vector<double> yBins{VARIABLE_WIDTH, 0, 5, 10, 20, 30, 40, 50, 101};
  std::vector<double> zBins{VARIABLE_WIDTH, -7.0, -5.0, -3.0, -1.0, 1.0, 3.0, 5.0, 7.0};

  ColumnBinningPolicy<test::Y, test::FloatZ> pairBinning{{yBins, zBins}, false};

  // Window size < category size
  std::vector<int> expectedCollisionsInBinSmallWindow{3, 3, 2, 1, 3, 3, 2, 1};
  int countFirst = 0;
  int previousEvent = -1;
  auto combGenSmallWindow = combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 3, -1, testA, testA));
  for (auto it = combGenSmallWindow.begin(); it != combGenSmallWindow.end(); it++) {
    auto& [c0, c1] = *it;
    REQUIRE(it.isNewWindow() == (previousEvent != c0.x()));
    if (it.isNewWindow()) {
      REQUIRE(it.currentWindowNeighbours() == expectedCollisionsInBinSmallWindow[countFirst]);
      countFirst++;
    }
    previousEvent = c0.index();
  }

  // Window size = category size
  std::vector<int> expectedCollisionsInBinEqualWindow{4, 3, 2, 1, 4, 3, 2, 1};
  countFirst = 0;
  previousEvent = -1;
  auto combGenEqualWindow = combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 4, -1, testA, testA));
  for (auto it = combGenEqualWindow.begin(); it != combGenEqualWindow.end(); it++) {
    auto& [c0, c1] = *it;
    REQUIRE(it.isNewWindow() == (previousEvent != c0.x()));
    if (it.isNewWindow()) {
      REQUIRE(it.currentWindowNeighbours() == expectedCollisionsInBinEqualWindow[countFirst]);
      countFirst++;
    }
    previousEvent = c0.index();
  }

  // Window size = category size
  std::vector<int> expectedCollisionsInBinBigWindow{4, 3, 2, 1, 4, 3, 2, 1};
  countFirst = 0;
  previousEvent = -1;
  auto combGenBigWindow = combinations(CombinationsBlockStrictlyUpperSameIndexPolicy(pairBinning, 5, -1, testA, testA));
  for (auto it = combGenBigWindow.begin(); it != combGenBigWindow.end(); it++) {
    auto& [c0, c1] = *it;
    REQUIRE(it.isNewWindow() == (previousEvent != c0.x()));
    if (it.isNewWindow()) {
      REQUIRE(it.currentWindowNeighbours() == expectedCollisionsInBinBigWindow[countFirst]);
      countFirst++;
    }
    previousEvent = c0.index();
  }
}
