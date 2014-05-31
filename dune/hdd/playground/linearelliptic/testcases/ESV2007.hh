// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_HDD_LINEARELLIPTIC_TESTCASES_ESV2007_HH
#define DUNE_HDD_LINEARELLIPTIC_TESTCASES_ESV2007_HH

#include <dune/stuff/functions/ESV2007.hh>
#include <dune/stuff/grid/provider/cube.hh>
#include <dune/stuff/common/configtree.hh>

#include <dune/hdd/linearelliptic/problems/ESV2007.hh>
#include "base.hh"

namespace Dune {
namespace HDD {
namespace LinearElliptic {
namespace TestCases {


template< class GridType >
class ESV2007
  : public Base< GridType >
{
  typedef Base< GridType > BaseType;
  static_assert(BaseType::dimDomain == 2, "This test case is only available in 2d!");
public:
  typedef typename BaseType::EntityType EntityType;
  typedef typename BaseType::DomainFieldType DomainFieldType;
  static const unsigned int                  dimDomain = BaseType::dimDomain;
  typedef double            RangeFieldType;
  static const unsigned int dimRange = 1;

  typedef Problems::ESV2007< EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ProblemType;
  typedef Stuff::Functions::ESV2007::Testcase1ExactSolution
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ExactSolutionType;

  typedef Stuff::LocalizableFunctionInterface < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange >
      FunctionType;

  ESV2007(const size_t num_refinements = 3)
    : BaseType(create_initial_grid(), num_refinements)
    , boundary_info_cfg_(Stuff::Grid::BoundaryInfoConfigs::AllDirichlet::default_config())
    , problem_(3)
    , exact_solution_(2, problem_.static_id() + ".exact_solution")
  {}

  void print_header(std::ostream& out = std::cout) const
  {
    out << "+==================================================================+\n"
        << "|+================================================================+|\n"
        << "||  Testcase ESV2007: smooth data, homogeneous dirichlet          ||\n"
        << "||  (see testcase 1, page 23 in Ern, Stephansen, Vohralik, 2007)  ||\n"
        << "|+----------------------------------------------------------------+|\n"
        << "||  domain = [-1, 1] x [-1, 1]                                    ||\n"
        << "||  diffusion = 1                                                 ||\n"
        << "||  force     = 1/2 pi^2 cos(1/2 pi x) cos(1/2 pi y)              ||\n"
        << "||  dirichlet = 0                                                 ||\n"
        << "||  exact solution = cos(1/2 pi x) cos(1/2 pi y)                  ||\n"
        << "|+================================================================+|\n"
        << "+==================================================================+" << std::endl;
  } // ... print_header(...)

  const Stuff::Common::ConfigTree& boundary_info() const
  {
    return boundary_info_cfg_;
  }

  const ProblemType& problem() const
  {
    return problem_;
  }

  bool provides_exact_solution() const
  {
    return true;
  }

  const ExactSolutionType& exact_solution() const
  {
    return exact_solution_;
  }

private:
  static std::shared_ptr< GridType > create_initial_grid()
  {
    Dune::Stuff::Grid::Providers::Cube< GridType > grid_provider(-1, 1, 4);
    auto grid = grid_provider.grid();
#if HAVE_ALUGRID
    if (std::is_same< GridType, Dune::ALUConformGrid< 2, 2 > >::value
        || std::is_same< GridType, Dune::ALUGrid< 2, 2, Dune::simplex, Dune::conforming > >::value)
      grid->globalRefine(1);
#endif // HAVE_ALUGRID
    grid->globalRefine(1);
    return grid;
  } // ... create_initial_grid(...)

  const Stuff::Common::ConfigTree boundary_info_cfg_;
  const ProblemType problem_;
  const ExactSolutionType exact_solution_;
}; // class ESV2007


} // namespace TestCases
} // namespace LinearElliptic
} // namespace HDD
} // namespace Dune

#endif // DUNE_HDD_LINEARELLIPTIC_TESTCASES_ESV2007_HH