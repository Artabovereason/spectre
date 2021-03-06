# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY Spectral)

set(LIBRARY_SOURCES
    Chebyshev.cpp
    ComplexDataView.cpp
    Filtering.cpp
    FiniteDifference.cpp
    Legendre.cpp
    Projection.cpp
    Spectral.cpp
    SwshCoefficients.cpp
    SwshCollocation.cpp
    SwshDerivatives.cpp
    SwshFiltering.cpp
    SwshInterpolation.cpp
    SwshTags.cpp
    SwshTransform.cpp
    )

add_spectre_library(${LIBRARY} ${LIBRARY_SOURCES})

target_link_libraries(
  ${LIBRARY}
  PUBLIC
  DataStructures
  ErrorHandling
  Libsharp

  INTERFACE
  # LinearOperators should be PUBLIC but there is a cyclic dependency
  LinearOperators

  PRIVATE
  Blas
  Boost::boost
  Lapack
  )
