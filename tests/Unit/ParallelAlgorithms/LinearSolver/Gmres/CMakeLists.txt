# Distributed under the MIT License.
# See LICENSE.txt for details.

set(LIBRARY "Test_ParallelGmres")

set(LIBRARY_SOURCES
  Test_ElementActions.cpp
  Test_ResidualMonitorActions.cpp
  )

add_test_library(
  ${LIBRARY}
  "ParallelAlgorithms/LinearSolver/Gmres"
  "${LIBRARY_SOURCES}"
  "DataStructures;IO;ParallelLinearSolver"
  )

add_dependencies(
  ${LIBRARY}
  module_ConstGlobalCache
  )

add_linear_solver_algorithm_test("GmresAlgorithm")
add_linear_solver_algorithm_test("DistributedGmresAlgorithm")
