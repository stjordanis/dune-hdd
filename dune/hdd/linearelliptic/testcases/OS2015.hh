// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_HDD_LINEARELLIPTIC_TESTCASES_OS2015_HH
#define DUNE_HDD_LINEARELLIPTIC_TESTCASES_OS2015_HH

#include <algorithm>
#include <cmath>
#include <map>

#include <dune/stuff/common/disable_warnings.hh>
# if HAVE_ALUGRID
#   include <dune/grid/alugrid.hh>
# endif
#include <dune/stuff/common/reenable_warnings.hh>

#include <dune/stuff/functions/constant.hh>

#include <dune/pymor/functions/default.hh>

#include <dune/hdd/linearelliptic/problems/OS2014.hh>

#include "base.hh"

namespace Dune {
namespace HDD {
namespace LinearElliptic {
namespace TestCases {
namespace OS2015 {
namespace internal {


template< class GridType >
class ParametricConvergenceBase
{
  static_assert(GridType::dimension == 2, "This test case is only available in 2d!");
public:
  typedef typename GridType::template Codim< 0 >::Entity EntityType;
  typedef typename GridType::ctype                       DomainFieldType;
  static const unsigned int                              dimDomain = GridType::dimension;
  typedef double            RangeFieldType;
  static const unsigned int dimRange = 1;
public:
  typedef Problems::OS2014::ParametricESV2007
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ProblemType;
  typedef Stuff::Functions::Constant
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ExactSolutionType;
  typedef typename ProblemType::FunctionType::NonparametricType            FunctionType;

  typedef std::map< std::string, Pymor::ParameterType > ParameterTypesMapType;
  typedef std::map< std::string, Pymor::Parameter >     ParametersMapType;

protected:
  static const size_t default_num_refinements_ = 3;

  static int initial_refinements()
  {
    int ret = 1;
#if HAVE_ALUGRID
    if (std::is_same< GridType, ALUConformGrid< 2, 2 > >::value
        || std::is_same< GridType, ALUGrid< 2, 2, simplex, conforming > >::value)
      ret += 1;
#endif // HAVE_ALUGRID
    return ret;
  } // ... initial_refinements()

  static ParametersMapType add_parameter_range(const ParametersMapType& parameters)
  {
    ParametersMapType ret = parameters;
    ret["parameter_range_min"] = Pymor::Parameter("mu", 0.1);
    ret["parameter_range_max"] = Pymor::Parameter("mu", 1.0);
    return ret;
  } // ... add_parameter_range(...)

public:
  static ParameterTypesMapType required_parameters()
  {
    return ParameterTypesMapType({{"mu",            Pymor::ParameterType("mu", 1)},
                                  {"mu_bar",        Pymor::ParameterType("mu", 1)},
                                  {"mu_hat",        Pymor::ParameterType("mu", 1)}});
  }

  ParametricConvergenceBase(const ParametersMapType& parameters)
    : parameters_(add_parameter_range(parameters))
    , boundary_info_cfg_(Stuff::Grid::BoundaryInfoConfigs::AllDirichlet::default_config())
    , problem_(3)
    , exact_solution_(0)
  {}

  void print_header(std::ostream& out = std::cout) const
  {
    out << "+==========================================================+\n"
        << "|+========================================================+|\n"
        << "||  Testcase OS2014: (parametric) ESV2007                 ||\n"
        << "||  (see Ohlberger, Schindler, 2014)                      ||\n"
        << "|+--------------------------------------------------------+|\n"
        << "||  domain = [-1, 1] x [-1, 1]                            ||\n"
        << "||  diffusion = 1 + (1 - mu) cos(1/2 pi x) cos(1/2 pi y)  ||\n"
        << "||  force     = 1/2 pi^2 cos(1/2 pi x) cos(1/2 pi y)      ||\n"
        << "||  dirichlet = 0                                         ||\n"
        << "||  reference solution: discrete solution on finest grid  ||\n"
        << "||  parameter: mu in [0.1, 1]                             ||\n"
        << "|+========================================================+|\n"
        << "+==========================================================+" << std::endl;
  } // ... print_header(...)

  const Stuff::Common::Configuration& boundary_info() const
  {
    return boundary_info_cfg_;
  }

  const ProblemType& problem() const
  {
    return problem_;
  }

  bool provides_exact_solution() const
  {
    return false;
  }

  const ExactSolutionType& exact_solution() const
  {
    DUNE_THROW(Stuff::Exceptions::you_are_using_this_wrong,
               "Do not call exact_solution() if provides_exact_solution() is false!");
  }

  const ParametersMapType& parameters() const
  {
    return parameters_;
  }

protected:
  const ParametersMapType parameters_;
  const Stuff::Common::Configuration boundary_info_cfg_;
  const ProblemType problem_;
  const ExactSolutionType exact_solution_;
}; // class ParametricConvergenceBase


template< class GridType >
class FiveSpotBase
{
  static_assert(GridType::dimension == 2, "This test case is only available in 2d!");
public:
  typedef typename GridType::template Codim< 0 >::Entity EntityType;
  typedef typename GridType::ctype                       DomainFieldType;
  static const unsigned int                              dimDomain = GridType::dimension;
  typedef double            RangeFieldType;
  static const unsigned int dimRange = 1;
public:
  typedef Problems::OS2014::FiveSpot
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ProblemType;
  typedef Stuff::Functions::Constant
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ExactSolutionType;
  typedef typename ProblemType::FunctionType::NonparametricType            FunctionType;

  typedef std::map< std::string, Pymor::ParameterType > ParameterTypesMapType;
  typedef std::map< std::string, Pymor::Parameter >     ParametersMapType;

protected:
  static const size_t default_num_refinements_ = 3;

  static int initial_refinements()
  {
    int ret = 1;
#if HAVE_ALUGRID
    if (std::is_same< GridType, ALUConformGrid< 2, 2 > >::value
        || std::is_same< GridType, ALUGrid< 2, 2, simplex, conforming > >::value)
      ret += 1;
#endif // HAVE_ALUGRID
    return ret;
  } // ... initial_refinements()

  static ParametersMapType add_parameter_range(const ParametersMapType& parameters)
  {
    ParametersMapType ret = parameters;
    ret["parameter_range_min"] = Pymor::Parameter("mu", 0.1);
    ret["parameter_range_max"] = Pymor::Parameter("mu", 0.9);
    return ret;
  } // ... add_parameter_range(...)

public:
  static ParameterTypesMapType required_parameters()
  {
    return ParameterTypesMapType({{"mu",     Pymor::ParameterType("mu", 1)},
                                  {"mu_bar", Pymor::ParameterType("mu", 1)},
                                  {"mu_hat", Pymor::ParameterType("mu", 1)}});
  }

  FiveSpotBase(const ParametersMapType parameters)
    : parameters_(add_parameter_range(parameters))
    , boundary_info_cfg_(Stuff::Grid::BoundaryInfoConfigs::AllDirichlet::default_config())
    , problem_(2)
    , exact_solution_(0)
  {}

  void print_header(std::ostream& out = std::cout) const
  {
    out << "+===============================================================================+\n"
        << "|+=============================================================================+|\n"
        << "||  Testcase OS2014: (parametric) five spot study                              ||\n"
        << "||  (see Ohlberger, Schindler, 2014)                                           ||\n"
        << "|+-----------------------------------------------------------------------------+|\n"
        << "||  domain = [-1, 1] x [-1, 1]                                                 ||\n"
        << "||  diffusion = 1                                                              ||\n"
        << "||              - mu      *exp(-((x + 1/2)^2/(2.0*0.1^2)))                     ||\n"
        << "||              - (1 - mu)*exp(-((x - 1/2)^2/(2.0*0.1^2)))                     ||\n"
        << "||  force     =   exp(-((x + 3/4)^2/(2.0*0.1^2)) - ((y + 3/4)^2/(2.0*0.1^2)))  ||\n"
        << "||              + exp(-((x - 3/4)^2/(2.0*0.1^2)) - ((y - 3/4)^2/(2.0*0.1^2)))  ||\n"
        << "||              + exp(-((x - 3/4)^2/(2.0*0.1^2)) - ((y + 3/4)^2/(2.0*0.1^2)))  ||\n"
        << "||              + exp(-((x + 3/4)^2/(2.0*0.1^2)) - ((y - 3/4)^2/(2.0*0.1^2)))  ||\n"
        << "||              - exp(-(        x^2/(2.0*0.1^2)) - (        y^2/(2.0*0.1^2)))  ||\n"
        << "||  dirichlet = 0                                                              ||\n"
        << "||  reference solution: discrete solution on finest grid                       ||\n"
        << "||  parameter: mu in [0.1, 0.9]                                                ||\n"
        << "|+=============================================================================+|\n"
        << "+===============================================================================+" << std::endl;
  } // ... print_header(...)

  const Stuff::Common::Configuration& boundary_info() const
  {
    return boundary_info_cfg_;
  }

  const ProblemType& problem() const
  {
    return problem_;
  }

  bool provides_exact_solution() const
  {
    return false;
  }

  const ExactSolutionType& exact_solution() const
  {
    DUNE_THROW(Stuff::Exceptions::you_are_using_this_wrong,
               "Do not call exact_solution() if provides_exact_solution() is false!");
  }

  const ParametersMapType& parameters() const
  {
    return parameters_;
  }

protected:
  const ParametersMapType parameters_;
  const Stuff::Common::Configuration boundary_info_cfg_;
  const ProblemType problem_;
  const ExactSolutionType exact_solution_;
}; // class FiveSpotBase


template< class GridType >
class LocalThermalblockBase
{
  static_assert(GridType::dimension == 2, "This test case is only available in 2d!");
public:
  typedef typename GridType::template Codim< 0 >::Entity EntityType;
  typedef typename GridType::ctype                       DomainFieldType;
  static const unsigned int                              dimDomain = GridType::dimension;
  typedef double            RangeFieldType;
  static const unsigned int dimRange = 1;
public:
  typedef Problems::OS2014::LocalThermalblock
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ProblemType;
  typedef Stuff::Functions::Constant
      < EntityType, DomainFieldType, dimDomain, RangeFieldType, dimRange > ExactSolutionType;
  typedef typename ProblemType::FunctionType::NonparametricType            FunctionType;

  typedef std::map< std::string, Pymor::ParameterType > ParameterTypesMapType;
  typedef std::map< std::string, Pymor::Parameter >     ParametersMapType;

protected:
  static const size_t default_num_refinements_ = 3;

  static int initial_refinements()
  {
    int ret = 1;
#if HAVE_ALUGRID
    if (std::is_same< GridType, ALUConformGrid< 2, 2 > >::value
        || std::is_same< GridType, ALUGrid< 2, 2, simplex, conforming > >::value)
      ret += 1;
#endif // HAVE_ALUGRID
    return ret;
  } // ... initial_refinements()

  static ParametersMapType add_parameter_range(const ParametersMapType& parameters)
  {
    ParametersMapType ret = parameters;
    ret["parameter_range_min"] = Pymor::Parameter("mu", 0.1);
    ret["parameter_range_max"] = Pymor::Parameter("mu", 1.0);
    return ret;
  } // ... add_parameter_range(...)

public:
  static ParameterTypesMapType required_parameters()
  {
    return ParameterTypesMapType({{"mu",     Pymor::ParameterType("mu", 1)},
                                  {"mu_bar", Pymor::ParameterType("mu", 1)},
                                  {"mu_hat", Pymor::ParameterType("mu", 1)}});
  }

  LocalThermalblockBase(const ParametersMapType parameters)
    : parameters_(add_parameter_range(parameters))
    , boundary_info_cfg_(Stuff::Grid::BoundaryInfoConfigs::AllDirichlet::default_config())
    , problem_()
    , exact_solution_(0)
  {}

  void print_header(std::ostream& out = std::cout) const
  {
    out << "+===============================================================================+\n"
        << "|+=============================================================================+|\n"
        << "||  Testcase OS2014: (parametric) local Thermalblock                           ||\n"
        << "|+-----------------------------------------------------------------------------+|\n"
        << "||  domain = [0, 1] x [0, 1]                                                   ||\n"
        << "||  diffusion = mu in [1/6, 2/6) x [1/6, 2/6)]                                 ||\n"
        << "||              1  else                                                        ||\n"
        << "||  force     = 1                                                              ||\n"
        << "||  dirichlet = 0                                                              ||\n"
        << "||  reference solution: discrete solution on finest grid                       ||\n"
        << "||  parameter: mu in [0.1, 1.0]                                                ||\n"
        << "|+=============================================================================+|\n"
        << "+===============================================================================+" << std::endl;
  } // ... print_header(...)

  const Stuff::Common::Configuration& boundary_info() const
  {
    return boundary_info_cfg_;
  }

  const ProblemType& problem() const
  {
    return problem_;
  }

  bool provides_exact_solution() const
  {
    return false;
  }

  const ExactSolutionType& exact_solution() const
  {
    DUNE_THROW(Stuff::Exceptions::you_are_using_this_wrong,
               "Do not call exact_solution() if provides_exact_solution() is false!");
  }

  const ParametersMapType& parameters() const
  {
    return parameters_;
  }

protected:
  const ParametersMapType parameters_;
  const Stuff::Common::Configuration boundary_info_cfg_;
  const ProblemType problem_;
  const ExactSolutionType exact_solution_;
}; // class LocalThermalblockBase


} // namespace internal


template< class GridType >
class ParametricConvergence
  : public internal::ParametricConvergenceBase< GridType >
  , public Base< GridType >
{
  typedef internal::ParametricConvergenceBase< GridType > ParametricConvergenceBaseType;
  typedef Base< GridType >                 TestCaseBaseType;

  static std::shared_ptr< GridType > create_initial_grid(const int refinements)
  {
    Stuff::Grid::Providers::Cube< GridType > grid_provider(-1, 1, 4);
    auto grid = grid_provider.grid_ptr();
    grid->globalRefine(refinements);
    return grid;
  } // ... create_initial_grid(...)

public:
  typedef typename TestCaseBaseType::ParametersMapType ParametersMapType;

  using ParametricConvergenceBaseType::required_parameters;
  using ParametricConvergenceBaseType::parameters;

  ParametricConvergence(const ParametersMapType parameters,
                        const size_t num_refinements = ParametricConvergenceBaseType::default_num_refinements_)
    : ParametricConvergenceBaseType(parameters)
    , TestCaseBaseType(create_initial_grid(ParametricConvergenceBaseType::initial_refinements()), num_refinements)
  {
    this->check_parameters(ParametricConvergenceBaseType::required_parameters(), parameters);
    this->inherit_parameter_type(this->problem_, "problem");
  }
}; // class ParametricConvergence


#if HAVE_DUNE_GRID_MULTISCALE


template< class GridType >
class Academic
  : public internal::ParametricConvergenceBase< GridType >
  , public MultiscaleCubeBase< GridType >
{
  typedef internal::ParametricConvergenceBase< GridType > ParametricConvergenceBaseType;
  typedef MultiscaleCubeBase< GridType >   TestCaseBaseType;

  static Stuff::Common::Configuration initial_grid_cfg(const std::string num_partitions,
                                                       const size_t oversampling_layers)
  {
    Stuff::Common::Configuration grid_cfg = Stuff::Grid::Providers::Cube< GridType >::default_config();
    grid_cfg["lower_left"] = "-1";
    grid_cfg["upper_right"] = "1";
    grid_cfg["num_elements"] = "4";
    grid_cfg["num_partitions"] = num_partitions;
    grid_cfg.set("oversampling_layers", oversampling_layers, /*overwrite=*/true);
    return grid_cfg;
  } // ... initial_grid_cfg(...)

public:
  typedef typename TestCaseBaseType::ParametersMapType ParametersMapType;

  using ParametricConvergenceBaseType::required_parameters;
  using ParametricConvergenceBaseType::parameters;

  Academic(const ParametersMapType parameters,
                             const std::string num_partitions = "[1 1 1]",
                             const size_t num_refinements = ParametricConvergenceBaseType::default_num_refinements_,
                             const size_t oversampling_layers = 0,
                             const bool H_with_h = false)
    : ParametricConvergenceBaseType(parameters)
    , TestCaseBaseType(initial_grid_cfg(num_partitions, oversampling_layers),
                       ParametricConvergenceBaseType::initial_refinements(),
                       num_refinements,
                       H_with_h)
  {
    this->check_parameters(ParametricConvergenceBaseType::required_parameters(), parameters);
    this->inherit_parameter_type(this->problem_, "problem");
  }
}; // class Academic


template< class GridType >
class FiveSpotBlock
  : public internal::FiveSpotBase< GridType >
  , public MultiscaleCubeBase< GridType >
{
  typedef internal::FiveSpotBase< GridType > FiveSpotBaseType;
  typedef MultiscaleCubeBase< GridType >     TestCaseBaseType;

  static Stuff::Common::Configuration initial_grid_cfg(const std::string num_partitions,
                                                       const size_t oversampling_layers)
  {
    Stuff::Common::Configuration grid_cfg = Stuff::Grid::Providers::Cube< GridType >::default_config();
    grid_cfg["lower_left"] = "-1";
    grid_cfg["upper_right"] = "1";
    grid_cfg["num_elements"] = "4";
    grid_cfg["num_partitions"] = num_partitions;
    grid_cfg.set("oversampling_layers", oversampling_layers, /*overwrite=*/true);
    return grid_cfg;
  } // ... initial_grid_cfg(...)

public:
  typedef typename TestCaseBaseType::ParametersMapType ParametersMapType;

  using FiveSpotBaseType::required_parameters;
  using FiveSpotBaseType::parameters;

  FiveSpotBlock(const ParametersMapType parameters,
                const std::string num_partitions = "[1 1 1]",
                const size_t num_refinements = FiveSpotBaseType::default_num_refinements_,
                const size_t oversampling_layers = 0)
    : FiveSpotBaseType(parameters)
    , TestCaseBaseType(initial_grid_cfg(num_partitions, oversampling_layers),
                       FiveSpotBaseType::initial_refinements(),
                       num_refinements)
  {
    this->check_parameters(FiveSpotBaseType::required_parameters(), parameters);
    this->inherit_parameter_type(this->problem_, "problem");
  }
}; // class FiveSpotBlock


template< class GridType >
class LocalThermalblockBlock
  : public internal::LocalThermalblockBase< GridType >
  , public MultiscaleCubeBase< GridType >
{
  typedef internal::LocalThermalblockBase< GridType > LocalThermalblockBaseType;
  typedef MultiscaleCubeBase< GridType >              TestCaseBaseType;

  static Stuff::Common::Configuration initial_grid_cfg(const std::string num_partitions,
                                                       const size_t oversampling_layers)
  {
    Stuff::Common::Configuration grid_cfg = Stuff::Grid::Providers::Cube< GridType >::default_config();
    grid_cfg["lower_left"] = "0";
    grid_cfg["upper_right"] = "1";
    grid_cfg["num_elements"] = "6";
    grid_cfg["num_partitions"] = num_partitions;
    grid_cfg.set("oversampling_layers", oversampling_layers, /*overwrite=*/true);
    return grid_cfg;
  } // ... initial_grid_cfg(...)

public:
  typedef typename TestCaseBaseType::ParametersMapType ParametersMapType;

  using LocalThermalblockBaseType::required_parameters;
  using LocalThermalblockBaseType::parameters;

  LocalThermalblockBlock(const ParametersMapType parameters,
                         const std::string num_partitions = "[1 1 1]",
                         const size_t num_refinements = LocalThermalblockBaseType::default_num_refinements_,
                         const size_t oversampling_layers = 0)
    : LocalThermalblockBaseType(parameters)
    , TestCaseBaseType(initial_grid_cfg(num_partitions, oversampling_layers),
                       LocalThermalblockBaseType::initial_refinements(),
                       num_refinements)
  {
    this->check_parameters(LocalThermalblockBaseType::required_parameters(), parameters);
    this->inherit_parameter_type(this->problem_, "problem");
  }
}; // class LocalThermalblockBlock


#endif // HAVE_DUNE_GRID_MULTISCALE

} // namespace OS2015
} // namespace TestCases
} // namespace LinearElliptic
} // namespace HDD
} // namespace Dune

#endif // DUNE_HDD_LINEARELLIPTIC_TESTCASES_OS2015_HH