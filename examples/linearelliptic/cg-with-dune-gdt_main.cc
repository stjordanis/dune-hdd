// This file is part of the dune-hdd project:
//   http://users.dune-project.org/projects/dune-hdd
// Copyright holders: Felix Albrecht
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifdef HAVE_CMAKE_CONFIG
  #include "cmake_config.h"
#elif defined (HAVE_CONFIG_H)
  #include "config.h"
#endif

#include <string>
#include <vector>

#include <dune/pymor/common/exceptions.hh>

#include "cg-with-dune-gdt.hh"

#include <boost/filesystem.hpp>

int main(int argc, char** argv)
{
  try {
    // create empty example
    typedef LinearellipticExampleCG< Dune::GridSelector::GridType > LinearellipticExample;
    LinearellipticExample example;

    // write settings file
    const std::string settingsFilename = example.static_id() + ".settings";
    if (!boost::filesystem::exists(settingsFilename))
      example.write_settings_file(settingsFilename);

    // init discrete problem and discretization
    example.initialize(std::vector< std::string >(argv, argv + argc));
    const auto discreteProblem = example.discrete_problem();
    const bool debugLogging = discreteProblem.debugLogging();
    auto& info = DSC_LOG_INFO;
    auto& debug = DSC_LOG_DEBUG;
    Dune::Timer timer;

    // inspect
    const auto discretization = example.discretization();
    info << std::endl;
    info << "available operator:   '" << discretization.available_operators()[0] << "'" << std::endl;
    info << "available functional: '" << discretization.available_functionals()[0] << "'" << std::endl;
    const std::string context = discretization.solver_options()[0];
    info << "options for solving in context '" << context << "' are: '" << discretization.solver_options(context)
         << "'" << std::endl;

    // solve
    if (discretization.parametric()) {
      info << "discretization is parametric with parameter_type:" << std::endl;
      info << "  " << discretization.parameter_type() << std::endl;
      const auto& settings = discreteProblem.settings();
      if (settings.hasSub("parameter")) {
        const auto parameters = settings.sub("parameter");
        size_t pp = 0;
        while (parameters.hasSub(Dune::Stuff::Common::toString(pp))) {
          const auto parameter = parameters.sub(Dune::Stuff::Common::toString(pp));
          Dune::Pymor::Parameter mu;
          for (std::string key : parameter.getValueKeys())
            mu.set(key, parameter.getVector< double >(key, 1));
          info << "solving for mu = " << mu;
          if (debugLogging)
            info << ":" << std::endl;
          else
            info << "... " << std::flush;
          timer.reset();
          auto solution = discretization.create_vector();
          discretization.solve(solution, mu, debug, "  ");
          if (!debugLogging)
            info << " done (took " << timer.elapsed() << "s)" << std::endl;
          discretization.visualize(solution,
                                   example.static_id() + ".solution_to_parameter_" + Dune::Stuff::Common::toString(pp),
                                   "solution to parameter " + Dune::Stuff::Common::toString(pp));
          ++pp;
        }
      } else
        info << "doing nothing, since there is no 'parameter' specified in the settings!" << std::endl;
    } else {
      info << "discretization is not parametric, solving";
      if (debugLogging)
        info << ":" << std::endl;
      else
        info << "... " << std::flush;
      timer.reset();
      auto solution = discretization.create_vector();
      discretization.solve(solution, Dune::Pymor::Parameter(), debug, "  ");
      if (!debugLogging)
        info << " done (took " << timer.elapsed() << "s)" << std::endl;
      discretization.visualize(solution, example.static_id() + ".solution", "solution");
    }

    // if we came that far we can as well be happy about it
    return 0;
  } catch (Dune::PymorException& e) {
    std::cerr << e << std::endl;
  } catch (Dune::Exception& e) {
    std::cerr << Dune::Stuff::Common::colorStringRed("Dune reported error: ") << e << std::endl;
  } catch (...) {
    std::cerr << Dune::Stuff::Common::colorStringRed("Unknown exception thrown!") << std::endl;
  }
}
