// Distributed under the MIT License.
// See LICENSE.txt for details.

/// \file
/// Define prefixes for DataBox tags

#pragma once

#include "DataStructures/DataBoxTag.hpp"
#include "DataStructures/Tensor/IndexType.hpp"
#include "DataStructures/Tensor/Metafunctions.hpp"
#include "DataStructures/Tensor/Tensor.hpp"
#include "Utilities/Requires.hpp"
#include "Utilities/TMPL.hpp"

namespace Tags {
/// \ingroup DataBoxTags
/// \brief prefix for a `VolumeDim` spatial derivatives with respect to the `Fr`
/// coordinate frame.
template <typename Tag, typename VolumeDim, typename Fr,
          typename = std::nullptr_t>
struct d;

template <typename Tag, typename VolumeDim, typename Fr>
struct d<Tag, VolumeDim, Fr, Requires<tt::is_a_v<Tensor, db::item_type<Tag>>>>
    : db::DataBoxPrefix {
  using type = TensorMetafunctions::prepend_spatial_index<
    db::item_type<Tag>, VolumeDim::value, UpLo::Lo, Fr>;
  using tag = Tag;
  static constexpr db::DataBoxString_t label = "d";
};

template <typename Tag, typename VolumeDim, typename Fr>
struct d<Tag, VolumeDim, Fr,
         Requires<tt::is_a_v<Variables, db::item_type<Tag>>>>
    : db::DataBoxPrefix {
  using type = db::item_type<Tag>;
  using tag = Tag;
  static constexpr db::DataBoxString_t label = "d";
};
}  // namespace Tags