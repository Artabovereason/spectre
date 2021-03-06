// Distributed under the MIT License.
// See LICENSE.txt for details.

#include "Framework/TestingFramework.hpp"

#include <algorithm>
#include <array>
#include <boost/functional/hash.hpp>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <pup.h>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "DataStructures/Tensor/Tensor.hpp"
#include "Domain/Block.hpp"
#include "Domain/BlockNeighbor.hpp"
#include "Domain/CoordinateMaps/CoordinateMap.tpp"
#include "Domain/CoordinateMaps/Identity.hpp"
#include "Domain/CreateInitialElement.hpp"
#include "Domain/Direction.hpp"
#include "Domain/DirectionMap.hpp"
#include "Domain/Element.hpp"
#include "Domain/ElementId.hpp"
#include "Domain/Neighbors.hpp"
#include "Domain/OrientationMap.hpp"
#include "Domain/SegmentId.hpp"
#include "Utilities/Gsl.hpp"
#include "Utilities/MakeArray.hpp"
#include "Utilities/StdHelpers.hpp"

namespace {
void test_create_initial_element(
    const ElementId<2>& element_id, const Block<2>& block,
    const std::vector<std::array<size_t, 2>>& refinement_levels,
    const DirectionMap<2, Neighbors<2>>& expected_neighbors) noexcept {
  const auto created_element = domain::Initialization::create_initial_element(
      element_id, block, refinement_levels);
  const Element<2> expected_element{element_id, expected_neighbors};
  CHECK(created_element == expected_element);
}

void test_h_refinement() noexcept {
  const auto check_upper =
      [](const OrientationMap<3>& neighbor_orientation,
         const std::array<size_t, 3>& neighbor_refinement,
         const std::unordered_set<ElementId<3>>& expected_neighbors) noexcept {
    CAPTURE(neighbor_orientation);
    CAPTURE(neighbor_refinement);
    const ElementId<3> self_id(0, {{{1, 1}, {1, 0}, {1, 1}}});
    const Block<3> self_block(
        domain::make_coordinate_map_base<Frame::Logical, Frame::Inertial>(
            domain::CoordinateMaps::Identity<3>{}),
        0, {{Direction<3>::upper_xi(), {1, neighbor_orientation}}});
    const std::vector<std::array<size_t, 3>> refinement_levels{
        {{1, 1, 1}}, neighbor_refinement};

    const auto refined_neighbors =
        domain::Initialization::create_initial_element(self_id, self_block,
                                                       refinement_levels)
            .neighbors()
            .at(Direction<3>::upper_xi())
            .ids();
    CHECK(refined_neighbors == expected_neighbors);
  };

  const auto check_lower =
      [](const OrientationMap<3>& neighbor_orientation,
         const std::array<size_t, 3>& neighbor_refinement,
         const std::unordered_set<ElementId<3>>& expected_neighbors) noexcept {
    CAPTURE(neighbor_orientation);
    CAPTURE(neighbor_refinement);
    const ElementId<3> self_id(0, {{{1, 0}, {1, 0}, {1, 1}}});
    const Block<3> self_block(
        domain::make_coordinate_map_base<Frame::Logical, Frame::Inertial>(
            domain::CoordinateMaps::Identity<3>{}),
        0, {{Direction<3>::lower_xi(), {1, neighbor_orientation}}});
    const std::vector<std::array<size_t, 3>> refinement_levels{
        {{1, 1, 1}}, neighbor_refinement};

    const auto refined_neighbors =
        domain::Initialization::create_initial_element(self_id, self_block,
                                                       refinement_levels)
            .neighbors()
            .at(Direction<3>::lower_xi())
            .ids();
    CHECK(refined_neighbors == expected_neighbors);
  };

  const OrientationMap<3> aligned{};
  const OrientationMap<3> rotated{
      {{Direction<3>::upper_zeta(), Direction<3>::upper_xi(),
        Direction<3>::upper_eta()}}};
  const OrientationMap<3> reflected{
      {{Direction<3>::lower_xi(), Direction<3>::lower_eta(),
        Direction<3>::upper_zeta()}}};

  // Same tangential refinement
  check_upper(aligned, {{1, 1, 1}}, {{1, {{{1, 0}, {1, 0}, {1, 1}}}}});
  check_lower(aligned, {{1, 1, 1}}, {{1, {{{1, 1}, {1, 0}, {1, 1}}}}});
  check_upper(rotated, {{1, 1, 1}}, {{1, {{{1, 0}, {1, 1}, {1, 0}}}}});
  check_lower(rotated, {{1, 1, 1}}, {{1, {{{1, 0}, {1, 1}, {1, 1}}}}});
  check_upper(reflected, {{1, 1, 1}}, {{1, {{{1, 1}, {1, 1}, {1, 1}}}}});
  check_lower(reflected, {{1, 1, 1}}, {{1, {{{1, 0}, {1, 1}, {1, 1}}}}});

  check_upper(aligned, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 0}, {1, 1}}}}});
  check_lower(aligned, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 0}, {1, 1}}}}});
  check_upper(rotated, {{1, 1, 0}}, {{1, {{{1, 0}, {1, 1}, {0, 0}}}}});
  check_lower(rotated, {{1, 1, 0}}, {{1, {{{1, 0}, {1, 1}, {0, 0}}}}});
  check_upper(reflected, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 1}, {1, 1}}}}});
  check_lower(reflected, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 1}, {1, 1}}}}});

  check_upper(aligned, {{2, 1, 1}}, {{1, {{{2, 0}, {1, 0}, {1, 1}}}}});
  check_lower(aligned, {{2, 1, 1}}, {{1, {{{2, 3}, {1, 0}, {1, 1}}}}});
  check_upper(rotated, {{1, 1, 2}}, {{1, {{{1, 0}, {1, 1}, {2, 0}}}}});
  check_lower(rotated, {{1, 1, 2}}, {{1, {{{1, 0}, {1, 1}, {2, 3}}}}});
  check_upper(reflected, {{2, 1, 1}}, {{1, {{{2, 3}, {1, 1}, {1, 1}}}}});
  check_lower(reflected, {{2, 1, 1}}, {{1, {{{2, 0}, {1, 1}, {1, 1}}}}});

  // Larger in both dimensions
  check_upper(aligned, {{1, 0, 0}}, {{1, {{{1, 0}, {0, 0}, {0, 0}}}}});
  check_lower(aligned, {{1, 0, 0}}, {{1, {{{1, 1}, {0, 0}, {0, 0}}}}});
  check_upper(rotated, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 0}}}}});
  check_lower(rotated, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 1}}}}});
  check_upper(reflected, {{1, 0, 0}}, {{1, {{{1, 1}, {0, 0}, {0, 0}}}}});
  check_lower(reflected, {{1, 0, 0}}, {{1, {{{1, 0}, {0, 0}, {0, 0}}}}});

  check_upper(aligned, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});
  check_lower(aligned, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});
  check_upper(rotated, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});
  check_lower(rotated, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});
  check_upper(reflected, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});
  check_lower(reflected, {{0, 0, 0}}, {{1, {{{0, 0}, {0, 0}, {0, 0}}}}});

  check_upper(aligned, {{2, 0, 0}}, {{1, {{{2, 0}, {0, 0}, {0, 0}}}}});
  check_lower(aligned, {{2, 0, 0}}, {{1, {{{2, 3}, {0, 0}, {0, 0}}}}});
  check_upper(rotated, {{0, 0, 2}}, {{1, {{{0, 0}, {0, 0}, {2, 0}}}}});
  check_lower(rotated, {{0, 0, 2}}, {{1, {{{0, 0}, {0, 0}, {2, 3}}}}});
  check_upper(reflected, {{2, 0, 0}}, {{1, {{{2, 3}, {0, 0}, {0, 0}}}}});
  check_lower(reflected, {{2, 0, 0}}, {{1, {{{2, 0}, {0, 0}, {0, 0}}}}});

  // Smaller in both dimensions
  check_upper(aligned, {{1, 2, 2}},
              {{1, {{{1, 0}, {2, 0}, {2, 2}}}},
               {1, {{{1, 0}, {2, 0}, {2, 3}}}},
               {1, {{{1, 0}, {2, 1}, {2, 2}}}},
               {1, {{{1, 0}, {2, 1}, {2, 3}}}}});
  check_lower(aligned, {{1, 2, 2}},
              {{1, {{{1, 1}, {2, 0}, {2, 2}}}},
               {1, {{{1, 1}, {2, 0}, {2, 3}}}},
               {1, {{{1, 1}, {2, 1}, {2, 2}}}},
               {1, {{{1, 1}, {2, 1}, {2, 3}}}}});
  check_upper(rotated, {{2, 2, 1}},
              {{1, {{{2, 0}, {2, 2}, {1, 0}}}},
               {1, {{{2, 0}, {2, 3}, {1, 0}}}},
               {1, {{{2, 1}, {2, 2}, {1, 0}}}},
               {1, {{{2, 1}, {2, 3}, {1, 0}}}}});
  check_lower(rotated, {{2, 2, 1}},
              {{1, {{{2, 0}, {2, 2}, {1, 1}}}},
               {1, {{{2, 0}, {2, 3}, {1, 1}}}},
               {1, {{{2, 1}, {2, 2}, {1, 1}}}},
               {1, {{{2, 1}, {2, 3}, {1, 1}}}}});
  check_upper(reflected, {{1, 2, 2}},
              {{1, {{{1, 1}, {2, 3}, {2, 2}}}},
               {1, {{{1, 1}, {2, 3}, {2, 3}}}},
               {1, {{{1, 1}, {2, 2}, {2, 2}}}},
               {1, {{{1, 1}, {2, 2}, {2, 3}}}}});
  check_lower(reflected, {{1, 2, 2}},
              {{1, {{{1, 0}, {2, 3}, {2, 2}}}},
               {1, {{{1, 0}, {2, 3}, {2, 3}}}},
               {1, {{{1, 0}, {2, 2}, {2, 2}}}},
               {1, {{{1, 0}, {2, 2}, {2, 3}}}}});

  check_upper(aligned, {{0, 2, 2}},
              {{1, {{{0, 0}, {2, 0}, {2, 2}}}},
               {1, {{{0, 0}, {2, 0}, {2, 3}}}},
               {1, {{{0, 0}, {2, 1}, {2, 2}}}},
               {1, {{{0, 0}, {2, 1}, {2, 3}}}}});
  check_lower(aligned, {{0, 2, 2}},
              {{1, {{{0, 0}, {2, 0}, {2, 2}}}},
               {1, {{{0, 0}, {2, 0}, {2, 3}}}},
               {1, {{{0, 0}, {2, 1}, {2, 2}}}},
               {1, {{{0, 0}, {2, 1}, {2, 3}}}}});
  check_upper(rotated, {{2, 2, 0}},
              {{1, {{{2, 0}, {2, 2}, {0, 0}}}},
               {1, {{{2, 0}, {2, 3}, {0, 0}}}},
               {1, {{{2, 1}, {2, 2}, {0, 0}}}},
               {1, {{{2, 1}, {2, 3}, {0, 0}}}}});
  check_lower(rotated, {{2, 2, 0}},
              {{1, {{{2, 0}, {2, 2}, {0, 0}}}},
               {1, {{{2, 0}, {2, 3}, {0, 0}}}},
               {1, {{{2, 1}, {2, 2}, {0, 0}}}},
               {1, {{{2, 1}, {2, 3}, {0, 0}}}}});
  check_upper(reflected, {{0, 2, 2}},
              {{1, {{{0, 0}, {2, 3}, {2, 2}}}},
               {1, {{{0, 0}, {2, 3}, {2, 3}}}},
               {1, {{{0, 0}, {2, 2}, {2, 2}}}},
               {1, {{{0, 0}, {2, 2}, {2, 3}}}}});
  check_lower(reflected, {{0, 2, 2}},
              {{1, {{{0, 0}, {2, 3}, {2, 2}}}},
               {1, {{{0, 0}, {2, 3}, {2, 3}}}},
               {1, {{{0, 0}, {2, 2}, {2, 2}}}},
               {1, {{{0, 0}, {2, 2}, {2, 3}}}}});

  check_upper(aligned, {{2, 2, 2}},
              {{1, {{{2, 0}, {2, 0}, {2, 2}}}},
               {1, {{{2, 0}, {2, 0}, {2, 3}}}},
               {1, {{{2, 0}, {2, 1}, {2, 2}}}},
               {1, {{{2, 0}, {2, 1}, {2, 3}}}}});
  check_lower(aligned, {{2, 2, 2}},
              {{1, {{{2, 3}, {2, 0}, {2, 2}}}},
               {1, {{{2, 3}, {2, 0}, {2, 3}}}},
               {1, {{{2, 3}, {2, 1}, {2, 2}}}},
               {1, {{{2, 3}, {2, 1}, {2, 3}}}}});
  check_upper(rotated, {{2, 2, 2}},
              {{1, {{{2, 0}, {2, 2}, {2, 0}}}},
               {1, {{{2, 0}, {2, 3}, {2, 0}}}},
               {1, {{{2, 1}, {2, 2}, {2, 0}}}},
               {1, {{{2, 1}, {2, 3}, {2, 0}}}}});
  check_lower(rotated, {{2, 2, 2}},
              {{1, {{{2, 0}, {2, 2}, {2, 3}}}},
               {1, {{{2, 0}, {2, 3}, {2, 3}}}},
               {1, {{{2, 1}, {2, 2}, {2, 3}}}},
               {1, {{{2, 1}, {2, 3}, {2, 3}}}}});
  check_upper(reflected, {{2, 2, 2}},
              {{1, {{{2, 3}, {2, 3}, {2, 2}}}},
               {1, {{{2, 3}, {2, 3}, {2, 3}}}},
               {1, {{{2, 3}, {2, 2}, {2, 2}}}},
               {1, {{{2, 3}, {2, 2}, {2, 3}}}}});
  check_lower(reflected, {{2, 2, 2}},
              {{1, {{{2, 0}, {2, 3}, {2, 2}}}},
               {1, {{{2, 0}, {2, 3}, {2, 3}}}},
               {1, {{{2, 0}, {2, 2}, {2, 2}}}},
               {1, {{{2, 0}, {2, 2}, {2, 3}}}}});

  // Larger in one dimension
  check_upper(aligned, {{1, 0, 1}}, {{1, {{{1, 0}, {0, 0}, {1, 1}}}}});
  check_lower(aligned, {{1, 0, 1}}, {{1, {{{1, 1}, {0, 0}, {1, 1}}}}});
  check_upper(rotated, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 1}, {1, 0}}}}});
  check_lower(rotated, {{0, 1, 1}}, {{1, {{{0, 0}, {1, 1}, {1, 1}}}}});
  check_upper(reflected, {{1, 0, 1}}, {{1, {{{1, 1}, {0, 0}, {1, 1}}}}});
  check_lower(reflected, {{1, 0, 1}}, {{1, {{{1, 0}, {0, 0}, {1, 1}}}}});

  check_upper(aligned, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 1}}}}});
  check_lower(aligned, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 1}}}}});
  check_upper(rotated, {{0, 1, 0}}, {{1, {{{0, 0}, {1, 1}, {0, 0}}}}});
  check_lower(rotated, {{0, 1, 0}}, {{1, {{{0, 0}, {1, 1}, {0, 0}}}}});
  check_upper(reflected, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 1}}}}});
  check_lower(reflected, {{0, 0, 1}}, {{1, {{{0, 0}, {0, 0}, {1, 1}}}}});

  check_upper(aligned, {{2, 0, 1}}, {{1, {{{2, 0}, {0, 0}, {1, 1}}}}});
  check_lower(aligned, {{2, 0, 1}}, {{1, {{{2, 3}, {0, 0}, {1, 1}}}}});
  check_upper(rotated, {{0, 1, 2}}, {{1, {{{0, 0}, {1, 1}, {2, 0}}}}});
  check_lower(rotated, {{0, 1, 2}}, {{1, {{{0, 0}, {1, 1}, {2, 3}}}}});
  check_upper(reflected, {{2, 0, 1}}, {{1, {{{2, 3}, {0, 0}, {1, 1}}}}});
  check_lower(reflected, {{2, 0, 1}}, {{1, {{{2, 0}, {0, 0}, {1, 1}}}}});

  // Smaller in one dimension
  check_upper(
      aligned, {{1, 2, 1}},
      {{1, {{{1, 0}, {2, 0}, {1, 1}}}}, {1, {{{1, 0}, {2, 1}, {1, 1}}}}});
  check_lower(
      aligned, {{1, 2, 1}},
      {{1, {{{1, 1}, {2, 0}, {1, 1}}}}, {1, {{{1, 1}, {2, 1}, {1, 1}}}}});
  check_upper(
      rotated, {{2, 1, 1}},
      {{1, {{{2, 0}, {1, 1}, {1, 0}}}}, {1, {{{2, 1}, {1, 1}, {1, 0}}}}});
  check_lower(
      rotated, {{2, 1, 1}},
      {{1, {{{2, 0}, {1, 1}, {1, 1}}}}, {1, {{{2, 1}, {1, 1}, {1, 1}}}}});
  check_upper(
      reflected, {{1, 2, 1}},
      {{1, {{{1, 1}, {2, 3}, {1, 1}}}}, {1, {{{1, 1}, {2, 2}, {1, 1}}}}});
  check_lower(
      reflected, {{1, 2, 1}},
      {{1, {{{1, 0}, {2, 3}, {1, 1}}}}, {1, {{{1, 0}, {2, 2}, {1, 1}}}}});

  check_upper(
      aligned, {{0, 2, 1}},
      {{1, {{{0, 0}, {2, 0}, {1, 1}}}}, {1, {{{0, 0}, {2, 1}, {1, 1}}}}});
  check_lower(
      aligned, {{0, 2, 1}},
      {{1, {{{0, 0}, {2, 0}, {1, 1}}}}, {1, {{{0, 0}, {2, 1}, {1, 1}}}}});
  check_upper(
      rotated, {{2, 1, 0}},
      {{1, {{{2, 0}, {1, 1}, {0, 0}}}}, {1, {{{2, 1}, {1, 1}, {0, 0}}}}});
  check_lower(
      rotated, {{2, 1, 0}},
      {{1, {{{2, 0}, {1, 1}, {0, 0}}}}, {1, {{{2, 1}, {1, 1}, {0, 0}}}}});
  check_upper(
      reflected, {{0, 2, 1}},
      {{1, {{{0, 0}, {2, 3}, {1, 1}}}}, {1, {{{0, 0}, {2, 2}, {1, 1}}}}});
  check_lower(
      reflected, {{0, 2, 1}},
      {{1, {{{0, 0}, {2, 3}, {1, 1}}}}, {1, {{{0, 0}, {2, 2}, {1, 1}}}}});

  check_upper(
      aligned, {{2, 2, 1}},
      {{1, {{{2, 0}, {2, 0}, {1, 1}}}}, {1, {{{2, 0}, {2, 1}, {1, 1}}}}});
  check_lower(
      aligned, {{2, 2, 1}},
      {{1, {{{2, 3}, {2, 0}, {1, 1}}}}, {1, {{{2, 3}, {2, 1}, {1, 1}}}}});
  check_upper(
      rotated, {{2, 1, 2}},
      {{1, {{{2, 0}, {1, 1}, {2, 0}}}}, {1, {{{2, 1}, {1, 1}, {2, 0}}}}});
  check_lower(
      rotated, {{2, 1, 2}},
      {{1, {{{2, 0}, {1, 1}, {2, 3}}}}, {1, {{{2, 1}, {1, 1}, {2, 3}}}}});
  check_upper(
      reflected, {{2, 2, 1}},
      {{1, {{{2, 3}, {2, 3}, {1, 1}}}}, {1, {{{2, 3}, {2, 2}, {1, 1}}}}});
  check_lower(
      reflected, {{2, 2, 1}},
      {{1, {{{2, 0}, {2, 3}, {1, 1}}}}, {1, {{{2, 0}, {2, 2}, {1, 1}}}}});

  // Larger in one dimension and smaller in another dimension
  check_upper(
      aligned, {{1, 2, 0}},
      {{1, {{{1, 0}, {2, 0}, {0, 0}}}}, {1, {{{1, 0}, {2, 1}, {0, 0}}}}});
  check_lower(
      aligned, {{1, 2, 0}},
      {{1, {{{1, 1}, {2, 0}, {0, 0}}}}, {1, {{{1, 1}, {2, 1}, {0, 0}}}}});
  check_upper(
      rotated, {{2, 0, 1}},
      {{1, {{{2, 0}, {0, 0}, {1, 0}}}}, {1, {{{2, 1}, {0, 0}, {1, 0}}}}});
  check_lower(
      rotated, {{2, 0, 1}},
      {{1, {{{2, 0}, {0, 0}, {1, 1}}}}, {1, {{{2, 1}, {0, 0}, {1, 1}}}}});
  check_upper(
      reflected, {{1, 2, 0}},
      {{1, {{{1, 1}, {2, 3}, {0, 0}}}}, {1, {{{1, 1}, {2, 2}, {0, 0}}}}});
  check_lower(
      reflected, {{1, 2, 0}},
      {{1, {{{1, 0}, {2, 3}, {0, 0}}}}, {1, {{{1, 0}, {2, 2}, {0, 0}}}}});

  check_upper(
      aligned, {{0, 2, 0}},
      {{1, {{{0, 0}, {2, 0}, {0, 0}}}}, {1, {{{0, 0}, {2, 1}, {0, 0}}}}});
  check_lower(
      aligned, {{0, 2, 0}},
      {{1, {{{0, 0}, {2, 0}, {0, 0}}}}, {1, {{{0, 0}, {2, 1}, {0, 0}}}}});
  check_upper(
      rotated, {{2, 0, 0}},
      {{1, {{{2, 0}, {0, 0}, {0, 0}}}}, {1, {{{2, 1}, {0, 0}, {0, 0}}}}});
  check_lower(
      rotated, {{2, 0, 0}},
      {{1, {{{2, 0}, {0, 0}, {0, 0}}}}, {1, {{{2, 1}, {0, 0}, {0, 0}}}}});
  check_upper(
      reflected, {{0, 2, 0}},
      {{1, {{{0, 0}, {2, 3}, {0, 0}}}}, {1, {{{0, 0}, {2, 2}, {0, 0}}}}});
  check_lower(
      reflected, {{0, 2, 0}},
      {{1, {{{0, 0}, {2, 3}, {0, 0}}}}, {1, {{{0, 0}, {2, 2}, {0, 0}}}}});

  check_upper(
      aligned, {{2, 2, 0}},
      {{1, {{{2, 0}, {2, 0}, {0, 0}}}}, {1, {{{2, 0}, {2, 1}, {0, 0}}}}});
  check_lower(
      aligned, {{2, 2, 0}},
      {{1, {{{2, 3}, {2, 0}, {0, 0}}}}, {1, {{{2, 3}, {2, 1}, {0, 0}}}}});
  check_upper(
      rotated, {{2, 0, 2}},
      {{1, {{{2, 0}, {0, 0}, {2, 0}}}}, {1, {{{2, 1}, {0, 0}, {2, 0}}}}});
  check_lower(
      rotated, {{2, 0, 2}},
      {{1, {{{2, 0}, {0, 0}, {2, 3}}}}, {1, {{{2, 1}, {0, 0}, {2, 3}}}}});
  check_upper(
      reflected, {{2, 2, 0}},
      {{1, {{{2, 3}, {2, 3}, {0, 0}}}}, {1, {{{2, 3}, {2, 2}, {0, 0}}}}});
  check_lower(
      reflected, {{2, 2, 0}},
      {{1, {{{2, 0}, {2, 3}, {0, 0}}}}, {1, {{{2, 0}, {2, 2}, {0, 0}}}}});
}
}  // namespace

SPECTRE_TEST_CASE("Unit.Domain.CreateInitialElement", "[Domain][Unit]") {
  OrientationMap<2> aligned(
      make_array(Direction<2>::upper_xi(), Direction<2>::upper_eta()));
  OrientationMap<2> unaligned(
      make_array(Direction<2>::lower_eta(), Direction<2>::upper_xi()));
  Block<2> test_block(
      domain::make_coordinate_map_base<Frame::Logical, Frame::Inertial>(
          domain::CoordinateMaps::Identity<2>{}),
      0,
      {{Direction<2>::upper_xi(), BlockNeighbor<2>{1, aligned}},
       {Direction<2>::upper_eta(), BlockNeighbor<2>{2, unaligned}}});
  std::vector<std::array<size_t, 2>> refinement{{{2, 3}}, {{2, 3}}, {{3, 2}}};

  // interior element
  test_create_initial_element(
      ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 4}}}}, test_block,
      refinement,
      {{Direction<2>::upper_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 4}}}}},
                     aligned}},
       {Direction<2>::lower_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 1}, SegmentId{3, 4}}}}},
                     aligned}},
       {Direction<2>::upper_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 5}}}}},
                     aligned}},
       {Direction<2>::lower_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 3}}}}},
                     aligned}}});

  // element on external boundary
  test_create_initial_element(
      ElementId<2>{0, {{SegmentId{2, 0}, SegmentId{3, 0}}}}, test_block,
      refinement,
      {{Direction<2>::upper_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 1}, SegmentId{3, 0}}}}},
                     aligned}},
       {Direction<2>::upper_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 0}, SegmentId{3, 1}}}}},
                     aligned}}});

  // element bounding aligned neighbor block
  test_create_initial_element(
      ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 4}}}}, test_block,
      refinement,
      {{Direction<2>::upper_xi(),
        Neighbors<2>{{ElementId<2>{1, {{SegmentId{2, 0}, SegmentId{3, 4}}}}},
                     aligned}},
       {Direction<2>::lower_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 4}}}}},
                     aligned}},
       {Direction<2>::upper_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 5}}}}},
                     aligned}},
       {Direction<2>::lower_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 3}}}}},
                     aligned}}});

  // element bounding unaligned neighbor block
  test_create_initial_element(
      ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 7}}}}, test_block,
      refinement,
      {{Direction<2>::upper_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 7}}}}},
                     aligned}},
       {Direction<2>::lower_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 1}, SegmentId{3, 7}}}}},
                     aligned}},
       {Direction<2>::upper_eta(),
        Neighbors<2>{{ElementId<2>{2, {{SegmentId{3, 0}, SegmentId{2, 1}}}}},
                     unaligned}},
       {Direction<2>::lower_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 6}}}}},
                     aligned}}});

  // element bounding both neighbor blocks
  test_create_initial_element(
      ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 7}}}}, test_block,
      refinement,
      {{Direction<2>::upper_xi(),
        Neighbors<2>{{ElementId<2>{1, {{SegmentId{2, 0}, SegmentId{3, 7}}}}},
                     aligned}},
       {Direction<2>::lower_xi(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 2}, SegmentId{3, 7}}}}},
                     aligned}},
       {Direction<2>::upper_eta(),
        Neighbors<2>{{ElementId<2>{2, {{SegmentId{3, 0}, SegmentId{2, 0}}}}},
                     unaligned}},
       {Direction<2>::lower_eta(),
        Neighbors<2>{{ElementId<2>{0, {{SegmentId{2, 3}, SegmentId{3, 6}}}}},
                     aligned}}});

  test_h_refinement();
}
