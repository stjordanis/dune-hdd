// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_HDD_LINEARELLIPTIC_DISCRETIZATIONS_SWIPDG_HH
#define DUNE_HDD_LINEARELLIPTIC_DISCRETIZATIONS_SWIPDG_HH

#include <memory>
#include <vector>

#include <dune/common/timer.hh>

#include <dune/stuff/common/configtree.hh>
#include <dune/stuff/grid/partview.hh>
#include <dune/stuff/grid/provider.hh>
#include <dune/stuff/la/container.hh>
#include <dune/stuff/la/solver.hh>

#include <dune/gdt/spaces/discontinuouslagrange.hh>
#include <dune/gdt/discretefunction/default.hh>
#include <dune/gdt/assembler/system.hh>
#include <dune/gdt/operators/elliptic-swipdg.hh>
#include <dune/gdt/functionals/l2.hh>
#include <dune/gdt/functionals/swipdg.hh>

#include <dune/gdt/localevaluation/elliptic.hh>
#include <dune/gdt/localoperator/codim0.hh>
#include <dune/gdt/localoperator/codim1.hh>
#include <dune/gdt/localevaluation/product.hh>
#include <dune/gdt/localevaluation/swipdg.hh>
#include <dune/gdt/localfunctional/codim0.hh>
#include <dune/gdt/localfunctional/codim1.hh>
#include <dune/gdt/products/l2.hh>
#include <dune/gdt/products/h1.hh>
#include <dune/gdt/products/elliptic.hh>

#include "base.hh"

namespace Dune {
namespace HDD {
namespace LinearElliptic {
namespace Discretizations {


// forward, needed in the Traits
template< class GridImp, Stuff::Grid::ChooseLayer layer, class RangeFieldImp, int rangeDim, int polynomialOrder = 1,
          GDT::ChooseSpaceBackend space_backend = GDT::ChooseSpaceBackend::fem_localfunction,
          Stuff::LA::ChooseBackend la_backend = Stuff::LA::ChooseBackend::eigen_sparse >
class SWIPDG;


namespace internal {


template< class GridImp, Stuff::Grid::ChooseLayer layer, class RangeFieldImp, int rangeDim, int polynomialOrder,
          GDT::ChooseSpaceBackend space_backend,
          Stuff::LA::ChooseBackend la_backend >
class SWIPDGTraits
{
public:
  typedef SWIPDG< GridImp, layer, RangeFieldImp, rangeDim, polynomialOrder, space_backend, la_backend > derived_type;
  typedef GridImp GridType;
  typedef RangeFieldImp RangeFieldType;
  static const unsigned int dimRange = rangeDim;
  static const unsigned int polOrder = polynomialOrder;

private:
  typedef GDT::Spaces::DiscontinuousLagrangeProvider< GridType, layer, space_backend,
                                                      polOrder, RangeFieldType, dimRange > SpaceProvider;

  friend class SWIPDG< GridImp, layer, RangeFieldImp, rangeDim, polynomialOrder, space_backend, la_backend >;

public:
  typedef typename Stuff::LA::Container< RangeFieldType, la_backend >::VectorType VectorType;
  typedef typename Stuff::LA::Container< RangeFieldType, la_backend >::MatrixType MatrixType;
  typedef typename SpaceProvider::Type TestSpaceType;
  typedef TestSpaceType AnsatzSpaceType;
  typedef typename TestSpaceType::GridViewType GridViewType;
}; // class SWIPDGTraits


} // namespace internal


template< class GridImp, Stuff::Grid::ChooseLayer layer, class RangeFieldImp, int rangeDim, int polynomialOrder,
          GDT::ChooseSpaceBackend space_backend,
          Stuff::LA::ChooseBackend la_backend >
class SWIPDG
  : public ContainerBasedDefault< internal::SWIPDGTraits< GridImp, layer, RangeFieldImp, rangeDim, polynomialOrder,
                                                          space_backend, la_backend > >
{
  typedef ContainerBasedDefault< internal::SWIPDGTraits< GridImp, layer, RangeFieldImp, rangeDim, polynomialOrder,
                                                         space_backend, la_backend > > BaseType;
public:
  typedef internal::SWIPDGTraits< GridImp, layer, RangeFieldImp, rangeDim, polynomialOrder, space_backend, la_backend >
      Traits;
  typedef GridImp GridType;
  using typename BaseType::ProblemType;
  using typename BaseType::BoundaryInfoType;
  using typename BaseType::TestSpaceType;
  using typename BaseType::AnsatzSpaceType;
  using typename BaseType::MatrixType;
  using typename BaseType::VectorType;
  using typename BaseType::GridViewType;
  using typename BaseType::RangeFieldType;

private:
  typedef typename Traits::SpaceProvider SpaceProvider;

  typedef Stuff::Grid::ConstProviderInterface< GridType > GridProviderType;
  using typename BaseType::AffinelyDecomposedMatrixType;
  using typename BaseType::AffinelyDecomposedVectorType;

public:
  static std::string static_id()
  {
    return typename DiscretizationInterface< Traits >::static_id() + ".swipdg";
  }

  SWIPDG(const GridProviderType& grid_provider,
         const Stuff::Common::ConfigTree& bound_inf_cfg,
         const ProblemType& prob,
         const int level = 0)
    : BaseType(std::make_shared< TestSpaceType >(SpaceProvider::create(grid_provider, level)),
               std::make_shared< AnsatzSpaceType >(SpaceProvider::create(grid_provider, level)),
               bound_inf_cfg,
               prob)
    , beta_(1.0)
  {}

  void init(std::ostream& out = Stuff::Common::Logger().devnull(), const std::string prefix = "")
  {
    if (!this->container_based_initialized_) {
      using namespace GDT;

      this->matrix_ = std::make_shared< AffinelyDecomposedMatrixType >();
      this->rhs_ = std::make_shared< AffinelyDecomposedVectorType >();

      auto& matrix = *(this->matrix_);
      auto& rhs = *(this->rhs_);
      const auto& space = *(this->test_space_);
      const auto& boundary_info = *(this->boundary_info_);

      out << prefix << "assembling... " << std::flush;
      Dune::Timer timer;
      SystemAssembler< TestSpaceType > system_assembler(space);

      // lhs operator
      typedef typename ProblemType::DiffusionFactorType::NonparametricType DiffusionFactorType;
      const auto& diffusion_factor = this->problem_.diffusion_factor();
      typedef Operators::EllipticSWIPDG< DiffusionFactorType, MatrixType, TestSpaceType > EllipticOperatorType;
      std::vector< std::unique_ptr< EllipticOperatorType > > elliptic_operators;
      auto pattern = EllipticOperatorType::pattern(space);
      for (size_t qq = 0; qq < diffusion_factor.num_components(); ++qq) {
        const size_t id = matrix.register_component(diffusion_factor.coefficient(qq),
                                                    space.mapper().size(), space.mapper().size(), pattern);
        elliptic_operators.emplace_back(new EllipticOperatorType(
            *(diffusion_factor.component(qq)),
            boundary_info,
            *(matrix.component(id)),
            space));
      }
      if (diffusion_factor.has_affine_part()) {
        if (!matrix.has_affine_part())
          matrix.register_affine_part(space.mapper().size(), space.mapper().size(), pattern);
        elliptic_operators.emplace_back(new EllipticOperatorType(
            *(diffusion_factor.affine_part()),
            boundary_info,
            *(matrix.affine_part()),
            space));
      }
      for (auto& elliptic_operator : elliptic_operators)
        system_assembler.add(*elliptic_operator);

      // rhs functional
      // * volume
      typedef typename ProblemType::FunctionType::NonparametricType FunctionType;
      const auto& force = this->problem_.force();
      typedef Functionals::L2Volume< FunctionType, VectorType, TestSpaceType > L2VolumeFunctionalType;
      std::vector< std::unique_ptr< L2VolumeFunctionalType > > force_functionals;
      for (size_t qq = 0; qq < force.num_components(); ++qq) {
        const size_t id = rhs.register_component(force.coefficient(qq), space.mapper().size());
        force_functionals.emplace_back(new L2VolumeFunctionalType(*(force.component(qq)),
                                                                  *(rhs.component(id)),
                                                                  space));
      }
      if (force.has_affine_part()) {
        if (!rhs.has_affine_part())
          rhs.register_affine_part(space.mapper().size());
        force_functionals.emplace_back(new L2VolumeFunctionalType(*(force.affine_part()),
                                                                  *(rhs.affine_part()),
                                                                  space));
      }
      for (auto& force_functional : force_functionals)
        system_assembler.add(*force_functional);
      // * dirichlet boundary
      const auto& dirichlet = this->problem_.dirichlet();
      typedef Functionals::DirichletBoundarySWIPDG< DiffusionFactorType, FunctionType, VectorType, TestSpaceType >
          DirichletBoundaryFunctionalType;
      std::vector< std::unique_ptr< DirichletBoundaryFunctionalType > > dirichlet_boundary_functionals;
      if (diffusion_factor.has_affine_part() && dirichlet.has_affine_part()) {
        if (!rhs.has_affine_part())
          rhs.register_affine_part(space.mapper().size());
        dirichlet_boundary_functionals.emplace_back(new DirichletBoundaryFunctionalType(
            *(diffusion_factor.affine_part()),
            *(dirichlet.affine_part()),
            boundary_info,
            *(rhs.affine_part()),
            space));
      }
      if (diffusion_factor.has_affine_part()) {
        for (size_t qq = 0; qq < dirichlet.num_components(); ++qq) {
          const size_t id = rhs.register_component(dirichlet.coefficient(qq), space.mapper().size());
          dirichlet_boundary_functionals.emplace_back(new DirichletBoundaryFunctionalType(
              *(diffusion_factor.affine_part()),
              *(dirichlet.component(qq)),
              boundary_info,
              *(rhs.component(id)),
              space));
        }
      }
      if (dirichlet.has_affine_part()) {
        for (size_t qq = 0; qq < diffusion_factor.num_components(); ++qq) {
          const size_t id = rhs.register_component(diffusion_factor.coefficient(qq), space.mapper().size());
          dirichlet_boundary_functionals.emplace_back(new DirichletBoundaryFunctionalType(
              *(diffusion_factor.component(qq)),
              *(dirichlet.affine_part()),
              boundary_info,
              *(rhs.component(id)),
              space));
        }
      }
      Pymor::ParameterType param;
      for (const auto& key : diffusion_factor.parameter_type().keys())
        param.set(key, diffusion_factor.parameter_type().get(key));
      for (const auto& key : dirichlet.parameter_type().keys())
        param.set(key, dirichlet.parameter_type().get(key));
      for (size_t pp = 0; pp < diffusion_factor.num_components(); ++ pp) {
        for (size_t qq = 0; qq < dirichlet.num_components(); ++qq) {
          const std::string expression = "(" + diffusion_factor.coefficient(pp)->expression()
                                             + ")*(" + dirichlet.coefficient(qq)->expression() + ")";
          const size_t id = rhs.register_component(param, expression, space.mapper().size());
          dirichlet_boundary_functionals.emplace_back(new DirichletBoundaryFunctionalType(
              *(diffusion_factor.component(pp)),
              *(dirichlet.component(qq)),
              boundary_info,
              *(rhs.component(id)),
              space));
        }
      }
      for (auto& dirichlet_boundary_functional : dirichlet_boundary_functionals)
        system_assembler.add(*dirichlet_boundary_functional);

      // * neumann boundary
      const auto& neumann = this->problem_.neumann();
      typedef Functionals::L2Face< FunctionType, VectorType, TestSpaceType > L2FaceFunctionalType;
      std::vector< std::unique_ptr< L2FaceFunctionalType > > neumann_boundary_functionals;
      for (size_t qq = 0; qq < neumann.num_components(); ++qq) {
        const size_t id = rhs.register_component(neumann.coefficient(qq), space.mapper().size());
        neumann_boundary_functionals.emplace_back(new L2FaceFunctionalType(
            *(neumann.component(qq)),
            *(rhs.component(id)),
            space,
            new ApplyOn::NeumannIntersections< GridViewType >(boundary_info)));
      }
      if (neumann.has_affine_part()) {
        if (!rhs.has_affine_part())
          rhs.register_affine_part(space.mapper().size());
        neumann_boundary_functionals.emplace_back(new L2FaceFunctionalType(
            *(neumann.affine_part()),
            *(rhs.affine_part()),
            space,
            new ApplyOn::NeumannIntersections< GridViewType >(boundary_info)));
      }
      for (auto& neumann_boundary_functional : neumann_boundary_functionals)
        system_assembler.add(*neumann_boundary_functional);

      // products
      // * L2
      typedef Products::L2Assemblable< MatrixType, TestSpaceType > L2ProductType;
      auto l2_product_matrix = std::make_shared< AffinelyDecomposedMatrixType >();
      l2_product_matrix->register_affine_part(new MatrixType(space.mapper().size(),
                                                             space.mapper().size(),
                                                             L2ProductType::pattern(space)));
      L2ProductType l2_product(*(l2_product_matrix->affine_part()), space);
      system_assembler.add(l2_product);
      // * H1 semi
      typedef Products::H1SemiAssemblable< MatrixType, TestSpaceType > H1ProductType;
      auto h1_product_matrix = std::make_shared< AffinelyDecomposedMatrixType >();
      h1_product_matrix->register_affine_part(new MatrixType(space.mapper().size(),
                                                             space.mapper().size(),
                                                             H1ProductType::pattern(space)));
      H1ProductType h1_product(*(h1_product_matrix->affine_part()), space);
      system_assembler.add(h1_product);

      // do the actual assembling
      system_assembler.walk();
      out << "done (took " << timer.elapsed() << "s)" << std::endl;

      // build parameter type
      this->inherit_parameter_type(matrix.parameter_type(), "lhs");
      this->inherit_parameter_type(rhs.parameter_type(), "rhs");

      // finalize
      this->products_.insert(std::make_pair("l2", l2_product_matrix));
      this->products_.insert(std::make_pair("h1_semi", h1_product_matrix));

      this->container_based_initialized_ = true;
    } // if (!this->container_based_initialized_)
  } // ... init(...)

private:
  const RangeFieldType beta_;
}; // class SWIPDG


} // namespace Discretizations
} // namespace LinearElliptic
} // namespace HDD
} // namespace Dune

#endif // DUNE_HDD_LINEARELLIPTIC_DISCRETIZATIONS_SWIPDG_HH