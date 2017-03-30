//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/cgm/Engines.h"

#include "smtk/bridge/cgm/CAUUID.h"

#include "smtk/Function.h"
#include "smtk/Options.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#ifdef CGM_HAVE_VERSION_H
#include "cgm_version.h"
#endif
#include "DLIList.hpp"
#include "FacetModifyEngine.hpp"
#include "GeometryHealerEngine.hpp"
#include "GeometryHealerTool.hpp"
#include "GeometryModifyEngine.hpp"
#include "GeometryModifyTool.hpp"
#include "GeometryQueryEngine.hpp"
#include "GeometryQueryTool.hpp"
#include "InitCGMA.hpp"

#ifdef HAVE_OCC
#include "OCCModifyEngine.hpp"
#endif
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <locale>

namespace smtk
{
namespace bridge
{
namespace cgm
{

static bool cgmaInitialized = false;
static std::string s_currentEngine;

bool Engines::areInitialized()
{
  return cgmaInitialized;
}

bool Engines::isInitialized(const std::string& engine, const std::vector<std::string>& args)
{
  std::vector<const char*> targs; // translated arguments
  targs.push_back("smtk");        // input \a args should not provide program name.
  std::vector<std::string>::const_iterator ait;
  for (ait = args.begin(); ait != args.end(); ++ait)
    targs.push_back(ait->c_str());

#if CGM_MAJOR_VERSION >= 14
  std::vector<CubitString> cargs(targs.begin(), targs.end());
  CGMApp::instance()->startup(cargs);
#else
  CubitObserver::init_static_observers();
  CGMApp::instance()->startup(targs.size(), const_cast<char**>(&targs[0]));
#endif

  smtk::bridge::cgm::CAUUID::registerWithAttributeManager();
  CubitStatus s = InitCGMA::initialize_cgma(engine.empty() ? "OCC" : engine.c_str());
  if (GeometryQueryTool::instance())
  {
    const char* modType = GeometryQueryTool::instance()->get_gqe()->modeler_type();
    s_currentEngine = modType ? modType : "Unknown";
  }
  cgmaInitialized = true;
  return (s == CUBIT_SUCCESS) ? true : false;
}

bool Engines::setDefault(const std::string& engine)
{
  // If we've never called init, do some extra stuff:
  if (!Engines::areInitialized())
  {
    return Engines::isInitialized(engine);
  }
  // Otherwise, see if we can create the appropriate engines
  // and set them in the default query/modify tool.

  // Downcase the engine string. Some facet engines return "facet"
  // for their modeler type, but the engine itself is named "FACET".
  // Grrr....
  std::string engineLower = engine;
  std::transform(engineLower.begin(), engineLower.end(), engineLower.begin(),
    std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("C")));

  bool defaultChanged = false;
  DLIList<GeometryModifyEngine*> gmes;
  GeometryModifyEngine* gme;
  GeometryModifyTool* gmt = NULL;
  DLIList<GeometryQueryEngine*> gqes;
  GeometryQueryEngine* gqe;
  GeometryQueryTool* gqt = NULL;
  //DLIList<GeometryHealerEngine*> ghes;
  //GeometryHealerEngine* ghe;
  //GeometryHealerTool* ght = NULL;
  // GeometryFeatureTool does not provide list of engines

  GeometryQueryTool::instance()->get_gqe_list(gqes);
  for (int i = 0; i < gqes.size(); ++i)
  {
    gqe = gqes.get_and_step();
    const char* mtxt = gqe->modeler_type();
    std::string modeler = (mtxt && mtxt[0] ? mtxt : "(null)");
    std::transform(modeler.begin(), modeler.end(), modeler.begin(),
      std::bind2nd(std::ptr_fun(&std::tolower<char>), std::locale("C")));
    if (modeler == engineLower)
    {
      GeometryQueryTool::instance()->set_default_gqe(gqe);
      gqt = GeometryQueryTool::instance();
      break;
    }
  }

  /* GeometryModifyEngine does not provide modeler_type().
  GeometryModifyTool::instance()->get_gme_list(gmes);
  for (int i = 0; i < gmes.size(); ++i)
    {
    gme = gmes.get_and_step();
    std::string modeler = gme->modeler_type();
    if (modeler == engineLower)
      {
      gmt = GeometryModifyTool::instance(gme);
      break;
      }
    }
    */
  if (engine == "FACET")
    gme = FacetModifyEngine::instance();
#ifdef HAVE_OCC
  else if (engine == "OCC")
    gme = OCCModifyEngine::instance();
#endif
  else
    gme = NULL;
  if (gme)
  {
    gmt = GeometryModifyTool::instance(gme);
  }

  // For now, we cannot rely on anyone providing a healer, so only gmt & gqt are tested:
  defaultChanged = (gmt && gqt ? true : false);
  s_currentEngine = engine;
  return defaultChanged;
}

std::string Engines::currentEngine()
{
  return s_currentEngine;
}

std::vector<std::string> Engines::listEngines()
{
  std::vector<std::string> result;

  if (!Engines::areInitialized())
    Engines::setDefault("FACET");

  GeometryQueryEngine* gqe;
  DLIList<GeometryQueryEngine*> gqes;
  GeometryQueryTool::instance()->get_gqe_list(gqes);
  for (int i = 0; i < gqes.size(); ++i)
  {
    gqe = gqes.get_and_step();
    if (gqe->is_intermediate_engine())
      continue; // skip, e.g., virtual geometry engine
    const char* kernel = gqe->modeler_type();
    if (!kernel || !kernel[0])
    {
      std::string typeName = typeid(*gqe).name();
      if (typeName.find("FacetQueryEngine") != std::string::npos)
      {
        kernel = "FACET";
      }
      else
      {
        std::cerr << "Unknown unnamed query engine, type " << typeName << ". Skipping.\n";
        continue;
      }
    }
    result.push_back(kernel);
  }
  return result;
}

bool Engines::shutdown()
{
  InitCGMA::deinitialize_cgma();
  cgmaInitialized = false;
  return true;
}

} // namespace cgm
} //namespace bridge
} // namespace smtk
