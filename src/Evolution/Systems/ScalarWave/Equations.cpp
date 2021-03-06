// Distributed under the MIT License.
// See LICENSE.txt for details.

#include "Evolution/Systems/ScalarWave/Equations.hpp"

#include <algorithm>
#include <array>

#include "DataStructures/DataBox/PrefixHelpers.hpp"
#include "DataStructures/DataVector.hpp"
#include "DataStructures/Tensor/Tensor.hpp"
#include "DataStructures/Variables.hpp"  // IWYU pragma: keep
#include "Evolution/Systems/ScalarWave/System.hpp"
#include "Utilities/GenerateInstantiations.hpp"
#include "Utilities/Gsl.hpp"   // IWYU pragma: keep
#include "Utilities/TMPL.hpp"  // IWYU pragma: keep

// IWYU pragma: no_forward_declare Tensor

namespace ScalarWave {
// Doxygen is not good at templates and so we have to hide the definition.
/// \cond
template <size_t Dim>
void ComputeDuDt<Dim>::apply(
    const gsl::not_null<Scalar<DataVector>*> dt_pi,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*> dt_phi,
    const gsl::not_null<Scalar<DataVector>*> dt_psi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& d_psi,
    const Scalar<DataVector>& pi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& d_pi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& phi,
    const tnsr::ij<DataVector, Dim, Frame::Inertial>& d_phi,
    const Scalar<DataVector>& gamma2) noexcept {
  get(*dt_psi) = -get(pi);
  get(*dt_pi) = -get<0, 0>(d_phi);
  for (size_t d = 1; d < Dim; ++d) {
    get(*dt_pi) -= d_phi.get(d, d);
  }
  for (size_t d = 0; d < Dim; ++d) {
    dt_phi->get(d) = -d_pi.get(d) + get(gamma2) * (d_psi.get(d) - phi.get(d));
  }
}

template <size_t Dim>
void ComputeNormalDotFluxes<Dim>::apply(
    const gsl::not_null<Scalar<DataVector>*> pi_normal_dot_flux,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        phi_normal_dot_flux,
    const gsl::not_null<Scalar<DataVector>*> psi_normal_dot_flux,
    const Scalar<DataVector>& pi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& phi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        interface_unit_normal) noexcept {
  // We assume that all values of psi_normal_dot_flux are the same. The reason
  // is that std::fill is actually surprisingly/disappointingly slow.
  if (psi_normal_dot_flux->get()[0] != 0.0) {
    std::fill(psi_normal_dot_flux->get().begin(),
              psi_normal_dot_flux->get().end(), 0.0);
  }

  get(*pi_normal_dot_flux) = get<0>(interface_unit_normal) * get<0>(phi);
  for (size_t i = 1; i < Dim; ++i) {
    get(*pi_normal_dot_flux) += interface_unit_normal.get(i) * phi.get(i);
  }

  for (size_t i = 0; i < Dim; ++i) {
    phi_normal_dot_flux->get(i) = interface_unit_normal.get(i) * get(pi);
  }
}

template <size_t Dim>
void PenaltyFlux<Dim>::package_data(
    const gsl::not_null<Scalar<DataVector>*> packaged_n_dot_flux_pi,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_n_dot_flux_phi,
    const gsl::not_null<Scalar<DataVector>*> packaged_v_plus,
    const gsl::not_null<Scalar<DataVector>*> packaged_v_minus,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_n_times_v_plus,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_n_times_v_minus,
    const Scalar<DataVector>& normal_dot_flux_pi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& normal_dot_flux_phi,
    const Scalar<DataVector>& v_plus, const Scalar<DataVector>& v_minus,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& interface_unit_normal)
    const noexcept {
  // Computes the contribution to the numerical flux from one side of the
  // interface.
  //
  // Note: when PenaltyFlux::operator() is called, an Element passes in its own
  // packaged data to fill the interior fields, and its neighbor's packaged data
  // to fill the exterior fields. This introduces a sign flip for each normal
  // used in computing the exterior fields.
  *packaged_n_dot_flux_pi = normal_dot_flux_pi;
  *packaged_n_dot_flux_phi = normal_dot_flux_phi;
  *packaged_v_plus = v_plus;
  *packaged_v_minus = v_minus;
  for (size_t d = 0; d < Dim; ++d) {
    packaged_n_times_v_plus->get(d) =
        get(v_plus) * interface_unit_normal.get(d);
    packaged_n_times_v_minus->get(d) =
        get(v_minus) * interface_unit_normal.get(d);
  }
}

template <size_t Dim>
void PenaltyFlux<Dim>::operator()(
    const gsl::not_null<Scalar<DataVector>*> pi_normal_dot_numerical_flux,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        phi_normal_dot_numerical_flux,
    const gsl::not_null<Scalar<DataVector>*> psi_normal_dot_numerical_flux,
    const Scalar<DataVector>& normal_dot_flux_pi_interior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_dot_flux_phi_interior,
    const Scalar<DataVector>& /* v_plus_interior */,
    const Scalar<DataVector>& v_minus_interior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
    /* normal_times_v_plus_interior */,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_times_v_minus_interior,
    const Scalar<DataVector>& /* minus_normal_dot_flux_pi_exterior */,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
    /* minus_normal_dot_flux_phi_exterior */,
    const Scalar<DataVector>& v_plus_exterior,
    const Scalar<DataVector>& /* v_minus_exterior */,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        minus_normal_times_v_plus_exterior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
    /* minus_normal_times_v_minus_exterior */) const noexcept {
  constexpr double penalty_factor = 1.;

  // NormalDotNumericalFlux<Psi>
  std::fill(psi_normal_dot_numerical_flux->get().begin(),
            psi_normal_dot_numerical_flux->get().end(), 0.);
  // NormalDotNumericalFlux<Pi>
  get(*pi_normal_dot_numerical_flux) =
      get(normal_dot_flux_pi_interior) +
      0.5 * penalty_factor * (get(v_minus_interior) - get(v_plus_exterior));
  // NormalDotNumericalFlux<Phi>
  for (size_t d = 0; d < Dim; ++d) {
    phi_normal_dot_numerical_flux->get(d) =
        normal_dot_flux_phi_interior.get(d) -
        0.5 * penalty_factor *
            (normal_times_v_minus_interior.get(d) +
             minus_normal_times_v_plus_exterior.get(d));
  }
}

template <size_t Dim>
void UpwindFlux<Dim>::package_data(
    const gsl::not_null<Scalar<DataVector>*> packaged_n_dot_flux_pi,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_n_dot_flux_phi,
    const gsl::not_null<Scalar<DataVector>*> packaged_pi,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_n_times_flux_pi,
    const gsl::not_null<Scalar<DataVector>*> packaged_gamma2_psi,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        packaged_normal_times_gamma2_psi,
    const Scalar<DataVector>& normal_dot_flux_pi,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& normal_dot_flux_phi,
    const Scalar<DataVector>& pi, const Scalar<DataVector>& psi,
    const Scalar<DataVector>& constraint_gamma2,
    const tnsr::i<DataVector, Dim, Frame::Inertial>& interface_unit_normal)
    const noexcept {
  // Computes the contribution to the numerical flux from one side of the
  // interface.
  //
  // Note: when Upwind::operator() is called, an Element passes in its own
  // packaged data to fill the interior fields, and its neighbors packaged data
  // to fill the exterior fields. This introduces a sign flip for each normal
  // used in computing the exterior fields.
  *packaged_pi = pi;
  *packaged_n_dot_flux_pi = normal_dot_flux_pi;
  *packaged_n_dot_flux_phi = normal_dot_flux_phi;
  for (size_t d = 0; d < Dim; ++d) {
    packaged_n_times_flux_pi->get(d) =
        interface_unit_normal.get(d) * get(normal_dot_flux_pi);
  }
  // Package quantities needed for constraint damping related terms
  get(*packaged_gamma2_psi) = get(constraint_gamma2) * get(psi);
  auto& normal_times_flux_psi = *packaged_normal_times_gamma2_psi;
  for (size_t d = 0; d < Dim; ++d) {
    normal_times_flux_psi.get(d) =
        get(*packaged_gamma2_psi) * interface_unit_normal.get(d);
  }
}

template <size_t Dim>
void UpwindFlux<Dim>::operator()(
    const gsl::not_null<Scalar<DataVector>*> pi_normal_dot_numerical_flux,
    const gsl::not_null<tnsr::i<DataVector, Dim, Frame::Inertial>*>
        phi_normal_dot_numerical_flux,
    const gsl::not_null<Scalar<DataVector>*> psi_normal_dot_numerical_flux,
    const Scalar<DataVector>& normal_dot_flux_pi_interior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_dot_flux_phi_interior,
    const Scalar<DataVector>& pi_interior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_times_flux_pi_interior,
    const Scalar<DataVector>& gamma2_psi_interior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_times_gamma2_psi_interior,
    const Scalar<DataVector>& minus_normal_dot_flux_pi_exterior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        minus_normal_dot_flux_phi_exterior,
    const Scalar<DataVector>& pi_exterior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        normal_times_flux_pi_exterior,
    const Scalar<DataVector>& gamma2_psi_exterior,
    const tnsr::i<DataVector, Dim, Frame::Inertial>&
        minus_normal_times_gamma2_psi_exterior) const noexcept {
  std::fill(psi_normal_dot_numerical_flux->get().begin(),
            psi_normal_dot_numerical_flux->get().end(), 0.);
  get(*pi_normal_dot_numerical_flux) =
      0.5 *
      (get(pi_interior) - get(pi_exterior) + get(normal_dot_flux_pi_interior) -
       get(minus_normal_dot_flux_pi_exterior) + get(gamma2_psi_exterior) -
       get(gamma2_psi_interior));
  for (size_t d = 0; d < Dim; ++d) {
    phi_normal_dot_numerical_flux->get(d) =
        0.5 * (normal_dot_flux_phi_interior.get(d) -
               minus_normal_dot_flux_phi_exterior.get(d) +
               normal_times_flux_pi_interior.get(d) -
               normal_times_flux_pi_exterior.get(d) -
               normal_times_gamma2_psi_interior.get(d) +
               minus_normal_times_gamma2_psi_exterior.get(d));
  }
}
/// \endcond
}  // namespace ScalarWave

// Generate explicit instantiations of partial_derivatives function as well as
// all other functions in Equations.cpp

#include "NumericalAlgorithms/LinearOperators/PartialDerivatives.tpp"  // IWYU pragma: keep

template <size_t Dim>
using derivative_tags = typename ScalarWave::System<Dim>::gradients_tags;

template <size_t Dim>
using variables_tags =
    typename ScalarWave::System<Dim>::variables_tag::tags_list;

using derivative_frame = Frame::Inertial;

#define DIM(data) BOOST_PP_TUPLE_ELEM(0, data)

#define INSTANTIATION(_, data)                                               \
  template class ScalarWave::ComputeDuDt<DIM(data)>;                         \
  template class ScalarWave::ComputeNormalDotFluxes<DIM(data)>;              \
  template class ScalarWave::UpwindFlux<DIM(data)>;                          \
  template class ScalarWave::PenaltyFlux<DIM(data)>;                         \
  template Variables<                                                        \
      db::wrap_tags_in<::Tags::deriv, derivative_tags<DIM(data)>,            \
                       tmpl::size_t<DIM(data)>, derivative_frame>>           \
  partial_derivatives<derivative_tags<DIM(data)>, variables_tags<DIM(data)>, \
                      DIM(data), derivative_frame>(                          \
      const Variables<variables_tags<DIM(data)>>& u,                         \
      const Mesh<DIM(data)>& mesh,                                           \
      const InverseJacobian<DataVector, DIM(data), Frame::Logical,           \
                            derivative_frame>& inverse_jacobian) noexcept;

GENERATE_INSTANTIATIONS(INSTANTIATION, (1, 2, 3))

#undef INSTANTIATION
#undef DIM
