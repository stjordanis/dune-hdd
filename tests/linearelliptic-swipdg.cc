// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#define DUNE_STUFF_FUNCTIONS_DISABLE_CHECKS

// This one has to come first (includes the config.h)!
#include <dune/stuff/test/test_common.hh>

//#define DUNE_HDD_LINEARELLIPTIC_DISCRETIZATIONS_SWIPDG_ESTIMATOR_ALTERNATE_SUMMATION 1

#if HAVE_ALUGRID_SERIAL_H || HAVE_ALUGRID_PARALLEL_H
# define ENABLE_ALUGRID 1
# include <dune/grid/alugrid.hh>
#else
# error This test requires ALUGrid!
#endif

#include <dune/stuff/common/exceptions.hh>

#include <dune/stuff/common/print.hh>
#include <dune/stuff/common/float_cmp.hh>

#include <dune/hdd/playground/linearelliptic/testcases/ESV2007.hh>

#include "linearelliptic-swipdg.hh"

using namespace Dune;
using namespace HDD;


// change this to toggle output
std::ostream& test_out = std::cout;
//std::ostream& test_out = DSC_LOG.devnull();


typedef Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > AluConform2dGridType;

typedef testing::Types< LinearElliptic::TestCases::ESV2007< AluConform2dGridType >
                      > AluConform2dTestCases;


template< class TestCaseType >
struct linearelliptic_SWIPDG_discretization
  : public ::testing::Test
{
  template< GDT::ChooseSpaceBackend space_backend, Stuff::LA::ChooseBackend la_backend >
  static void eoc_study()
  {
    const TestCaseType test_case;
    test_case.print_header(test_out);
    test_out << std::endl;
    LinearElliptic::Tests::EocStudySWIPDG< TestCaseType, 1, space_backend, la_backend > eoc_study(test_case);
    auto errors = eoc_study.run(false, test_out);
    for (const auto& norm : eoc_study.provided_norms())
      if (!Dune::Stuff::Common::FloatCmp::lt(errors[norm],
                                             truncate_vector(eoc_study.expected_results(norm), errors[norm].size()))) {
        std::stringstream ss;
        Dune::Stuff::Common::print(errors[norm],                        "errors           (" + norm + ")", ss);
        Dune::Stuff::Common::print(eoc_study.expected_results(norm), "   expected results (" + norm + ")", ss);
        DUNE_THROW(errors_are_not_as_expected, ss.str());
      }
  } // ... eoc_study()

  static void eoc_study_using_fem_and_istl()
  {
    eoc_study< GDT::ChooseSpaceBackend::fem, Stuff::LA::ChooseBackend::istl_sparse >();
  }

  static void eoc_study_using_fem_and_eigen_sparse()
  {
    eoc_study< GDT::ChooseSpaceBackend::fem, Stuff::LA::ChooseBackend::eigen_sparse >();
  }

//  static void eoc_study_using_pdelab_and_istl()
//  {
//    eoc_study< GDT::ChooseSpaceBackend::pdelab, Stuff::LA::ChooseBackend::istl_sparse >();
//  }

//  static void eoc_study_using_pdelab_and_eigen_sparse()
//  {
//    eoc_study< GDT::ChooseSpaceBackend::pdelab, Stuff::LA::ChooseBackend::eigen_sparse >();
//  }
}; // linearelliptic_SWIPDG_discretization


TYPED_TEST_CASE(linearelliptic_SWIPDG_discretization, AluConform2dTestCases);
TYPED_TEST(linearelliptic_SWIPDG_discretization, eoc_study_using_fem_and_istl) {
  this->eoc_study_using_fem_and_istl();
}
TYPED_TEST(linearelliptic_SWIPDG_discretization, eoc_study_using_fem_and_eigen_sparse) {
  this->eoc_study_using_fem_and_eigen_sparse();
}
//TYPED_TEST(linearelliptic_SWIPDG_discretization, eoc_study_using_pdelab_and_istl) {
//  this->eoc_study_using_pdelab_and_istl();
//}
//TYPED_TEST(linearelliptic_SWIPDG_discretization, eoc_study_using_pdelab_and_eigen_sparse) {
//  this->eoc_study_using_pdelab_and_eigen_sparse();
//}


#include <dune/stuff/test/test_main.hh>
