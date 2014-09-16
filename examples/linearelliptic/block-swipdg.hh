// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_HDD_EXAMPLES_LINEARELLIPTIC_BLOCK_SWIPDG_HH
#define DUNE_HDD_EXAMPLES_LINEARELLIPTIC_BLOCK_SWIPDG_HH

#include <memory>

#include <dune/stuff/common/memory.hh>

#include <dune/hdd/linearelliptic/discreteproblem.hh>
#include <dune/hdd/linearelliptic/discretizations/block-swipdg.hh>

template< class GridImp >
class LinearellipticExampleBlockSWIPDG
{
public:
  typedef GridImp GridType;
  typedef Dune::HDD::LinearElliptic::DiscreteBlockProblem< GridType > DiscreteProblemType;
  typedef typename DiscreteProblemType::RangeFieldType                RangeFieldType;
  static const unsigned int                                           dimRange = DiscreteProblemType::dimRange;
  static const unsigned int                                           polOrder = 1;
  typedef Dune::HDD::LinearElliptic::Discretizations::BlockSWIPDG
      < GridType, RangeFieldType, dimRange, polOrder >                DiscretizationType;

  static std::string static_id()
  {
    return "linearelliptic.block-swipdg";
  }

  static void write_config_file(const std::string filename = static_id() + ".cfg")
  {
    DiscreteProblemType::write_config(filename, static_id());
  }

  void initialize(const std::vector< std::string > arguments)
  {
    using namespace Dune;
    if (!discrete_problem_) {
      discrete_problem_ = Stuff::Common::make_unique< DiscreteProblemType >(static_id(), arguments);
      const bool debug_logging = discrete_problem_->debug_logging();
      auto& info = DSC_LOG_INFO;
      auto& debug = DSC_LOG_DEBUG;
      discretization_ = Stuff::Common::make_unique< DiscretizationType >(discrete_problem_->grid_provider(),
                                                                         discrete_problem_->boundary_info(),
                                                                         discrete_problem_->problem());
      info << "initializing discretization";
      if (debug_logging)
        info << ":" << std::endl;
      else
        info << "... " << std::flush;
      Dune::Timer timer;
      discretization_->init(debug, "  ");
      if (!debug_logging)
        info << "done (took " << timer.elapsed() << "s)" << std::endl;
    } else
      DSC_LOG_INFO << "initialize has already been called" << std::endl;
  } // ... initialize(...)

  const DiscreteProblemType& discrete_problem() const
  {
    return *discrete_problem_;
  }

  const DiscretizationType& discretization() const
  {
    return *discretization_;
  }

  DiscretizationType* discretization_and_return_ptr() const
  {
    return new DiscretizationType(*discretization_);
  }

private:
  std::unique_ptr< DiscreteProblemType > discrete_problem_;
  std::unique_ptr< DiscretizationType > discretization_;
}; // class LinearellipticExampleBlockSWIPDG


#endif // DUNE_HDD_EXAMPLES_LINEARELLIPTIC_BLOCK_SWIPDG_HH
