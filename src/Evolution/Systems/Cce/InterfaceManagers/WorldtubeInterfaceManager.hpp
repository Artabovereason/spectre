// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include <boost/optional.hpp>
#include <memory>
#include <tuple>

#include "DataStructures/Tensor/Tensor.hpp"
#include "DataStructures/Tensor/TypeAliases.hpp"
#include "Options/Options.hpp"
#include "Parallel/CharmPupable.hpp"
#include "Time/TimeStepId.hpp"

namespace Cce {
/// \cond
class GhLockstepInterfaceManager;
/// \endcond

/*!
 * \brief Abstract base class for storage and retrieval of generalized harmonic
 * quantities communicated from a Cauchy simulation to the Cce system.
 *
 * \details The functions that are required to be overriden in the derived
 * classes are:
 * - `GhWorldtubeInterfaceManager::get_clone()`: should return a
 * `std::unique_ptr<GhWorldtubeInterfaceManager>` with cloned state.
 * - `GhWorldtubeInterfaceManager::insert_gh_data()`: should store the portions
 * of the provided generalized harmonic data that are required to provide useful
 * boundary values for the CCE evolution at requested timesteps.
 * - `GhWorldtubeInterfaceManager::request_gh_data()`: should register requests
 * from the CCE evolution for boundary data.
 * - `GhWorldtubeInterfaceManager::retrieve_and_remove_first_ready_gh_data()`:
 * should return a `boost::optional<std::tuple<TimeStepId, tnsr::aa<DataVector,
 * 3>, tnsr::iaa<DataVector, 3>, tnsr::aa<DataVector, 3>>>` containing the
 * boundary data associated with the oldest requested timestep if enough data
 * has been supplied via `insert_gh_data()` to determine the boundary data.
 * Otherwise, return a `boost::none` to indicate that the CCE system must
 * continue waiting for generalized harmonic input.
 * - `GhWorldtubeInterfaceManager::number_of_pending_requests()`: should return
 * the number of requests that have been registered to the class that do not yet
 * been retrieved via `retrieve_and_remove_first_ready_gh_data()`.
 * - `GhWorldtubeInterfaceManager::number_of_gh_times()`: should return the
 * number of time steps sent to `insert_gh_data()` that have not yet been
 * retrieved via `retrieve_and_remove_first_ready_gh_data()`.
 */
class GhWorldtubeInterfaceManager : public PUP::able {
 public:
  using creatable_classes = tmpl::list<GhLockstepInterfaceManager>;

  WRAPPED_PUPable_abstract(GhWorldtubeInterfaceManager);  // NOLINT

  virtual std::unique_ptr<GhWorldtubeInterfaceManager> get_clone() const
      noexcept = 0;

  virtual void insert_gh_data(TimeStepId time_id,
                              tnsr::aa<DataVector, 3> spacetime_metric,
                              tnsr::iaa<DataVector, 3> phi,
                              tnsr::aa<DataVector, 3> pi,
                              tnsr::aa<DataVector, 3> dt_spacetime_metric,
                              tnsr::iaa<DataVector, 3> dt_phi,
                              tnsr::aa<DataVector, 3> dt_pi) noexcept = 0;

  virtual void request_gh_data(const TimeStepId&) noexcept = 0;

  virtual auto retrieve_and_remove_first_ready_gh_data() noexcept
      -> boost::optional<
          std::tuple<TimeStepId, tnsr::aa<DataVector, 3>,
                     tnsr::iaa<DataVector, 3>, tnsr::aa<DataVector, 3>>> = 0;

  virtual size_t number_of_pending_requests() const noexcept = 0;

  virtual size_t number_of_gh_times() const noexcept = 0;
};

}  // namespace Cce
