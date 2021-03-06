// Distributed under the MIT License.
// See LICENSE.txt for details.

#include "PointwiseFunctions/AnalyticSolutions/Poisson/Lorentzian.hpp"

#include <array>

#include "DataStructures/DataVector.hpp"  // IWYU pragma: keep
#include "DataStructures/Tensor/EagerMath/DotProduct.hpp"
#include "DataStructures/Tensor/Tensor.hpp"  // IWYU pragma: keep
#include "NumericalAlgorithms/LinearOperators/PartialDerivatives.hpp"
#include "Utilities/ConstantExpressions.hpp"
#include "Utilities/MakeWithValue.hpp"
// IWYU pragma: no_forward_declare Tensor

namespace Poisson {
namespace Solutions {

template <>
tuples::TaggedTuple<Tags::Field> Lorentzian<3>::variables(
    const tnsr::I<DataVector, 3>& x,
    tmpl::list<Tags::Field> /*meta*/) noexcept {
  return {Scalar<DataVector>(1. / sqrt(1. + get(dot_product(x, x))))};
}

template <>
tuples::TaggedTuple<
    ::Tags::deriv<Tags::Field, tmpl::size_t<3>, Frame::Inertial>>
Lorentzian<3>::variables(
    const tnsr::I<DataVector, 3>& x,
    tmpl::list<::Tags::deriv<Tags::Field, tmpl::size_t<3>,
                             Frame::Inertial>> /*meta*/) noexcept {
  const DataVector prefactor = -1. / cube(sqrt(1. + get(dot_product(x, x))));
  tnsr::i<DataVector, 3, Frame::Inertial> grad_field{prefactor};
  get<0>(grad_field) *= get<0>(x);
  get<1>(grad_field) *= get<1>(x);
  get<2>(grad_field) *= get<2>(x);
  return {std::move(grad_field)};
}

template <>
tuples::TaggedTuple<::Tags::FixedSource<Tags::Field>> Lorentzian<3>::variables(
    const tnsr::I<DataVector, 3>& x,
    tmpl::list<::Tags::FixedSource<Tags::Field>> /*meta*/) noexcept {
  return {Scalar<DataVector>(3. / pow<5>(sqrt(1. + get(dot_product(x, x)))))};
}

template <size_t Dim>
void Lorentzian<Dim>::pup(PUP::er& /*p*/) noexcept {}

template <size_t Dim>
bool operator==(const Lorentzian<Dim>& /*lhs*/,
                const Lorentzian<Dim>& /*rhs*/) noexcept {
  return true;
}

template <size_t Dim>
bool operator!=(const Lorentzian<Dim>& lhs,
                const Lorentzian<Dim>& rhs) noexcept {
  return not(lhs == rhs);
}

// Explicit template instantiations
template class Lorentzian<3>;
template bool operator==(const Lorentzian<3>& lhs,
                         const Lorentzian<3>& rhs) noexcept;
template bool operator!=(const Lorentzian<3>& lhs,
                         const Lorentzian<3>& rhs) noexcept;

}  // namespace Solutions
}  // namespace Poisson
