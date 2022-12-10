//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/oscillator/operators/Export.h"

#include "smtk/session/oscillator/Resource.h"
#include "smtk/session/oscillator/SimulationAttribute.h"
#include "smtk/session/oscillator/operators/Export_xml.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include <fstream>

using namespace smtk::model;
using namespace smtk::common;

using ResourceArray = std::vector<smtk::attribute::Resource::Ptr>;
using EntitySet = std::set<smtk::model::Entity::Ptr>;

namespace smtk
{
namespace session
{
namespace oscillator
{

Export::Result Export::operateInternal()
{
  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");
  std::string filename = filenameItem->value();

  auto simulations = this->parameters()->associatedObjects<ResourceArray>();
  if (simulations.empty())
  {
    smtkErrorMacro(this->log(), "No simulations to export.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto simulation = *simulations.begin();
  SimulationAttribute linter;
  bool ok = linter.lint(simulation, this->log());
  if (!ok)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  std::ofstream oscFile(filename.c_str());
  oscFile << "## type      center      r       omega0      zeta ##\n";
  for (const auto& sourceTerm : linter.sourceTerms())
  {
    auto sourceType = sourceTerm->definition()->type();
    std::string termName;
    bool hasZeta = false;
    double zeta;
    double omega = sourceTerm->findDouble("omega")->value();
    if (sourceType == "periodic-source")
    {
      termName = "periodic";
    }
    else if (sourceType == "decaying-source")
    {
      termName = "decaying";
    }
    else if (sourceType == "damped-source")
    {
      termName = "damped";
      zeta = sourceTerm->findDouble("zeta")->value();
      hasZeta = true;
    }
    else
    {
      smtkErrorMacro(this->log(), "Unknown source term type \"" << sourceType << "\". Skipping.");
      continue;
    }
    auto assocPoints = sourceTerm->associatedObjects<EntitySet>();
    for (const auto& assocPoint : assocPoints)
    {
      auto sourcePoint = assocPoint->referenceAs<smtk::model::AuxiliaryGeometry>();
      auto center = sourcePoint.floatProperty("center");
      auto radius = sourcePoint.floatProperty("radius");
      oscFile << termName << "   " << center[0] << " " << center[1] << " " << center[2] << "   "
              << radius[0] << "   " << omega;
      if (hasZeta)
      {
        oscFile << "   " << zeta;
      }
      oscFile << "\n";
    }
  }

  auto solver = simulation->findAttribute("solver parameters");
  auto coprocessing = simulation->findAttribute("coprocessing parameters");

  int numRanks = solver->findInt("job size")->value();
  double timestepSize = solver->findDouble("time step")->value();
  double endTime = solver->findDouble("end time")->value();
  std::string pathToSim("/stage/build/sensei/sensei/bin/oscillator");
  auto resolutionItem = solver->findInt("resolution");
  std::vector<int> resolution(resolutionItem->begin(), resolutionItem->end());
  // Pad simulation domain discretization to be 3-D:
  for (std::size_t ii = resolution.size(); ii < 3; ++ii)
  {
    resolution.push_back(1);
  }

  bool log = coprocessing->findVoid("log")->isEnabled();
  bool includeAnalysis = coprocessing->findFile("script")->isEnabled();

  std::ostringstream commandLine;
  commandLine << "mpirun -np " << numRanks << "\\\n"
              << "  " << pathToSim << "\\\n"
              << "  -s " << resolution[0] << " " << resolution[1] << " " << resolution[2] << "  -b "
              << numRanks << " \\\n"
              << "  --dt " << timestepSize << "\\\n"
              << "  -t " << endTime << "\\\n";
  if (includeAnalysis)
  {
    commandLine << "  -f " << coprocessing->findFile("script")->value() << "\\\n";
  }
  if (log)
  {
    commandLine << "  --log \\\n";
  }
  commandLine << "  " << filename;

  std::cout << "\n" << commandLine.str() << "\n\n";

  return this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
}

const char* Export::xmlDescription() const
{
  return Export_xml;
}

bool exportResource(const smtk::resource::ResourcePtr& resource)
{
  Export::Ptr exportResource = Export::create();
  if (!exportResource->parameters()->associations()->setValue(resource))
  {
    return false;
  }
  Export::Result result = exportResource->operate();
  return (result->findInt("outcome")->value() == static_cast<int>(Export::Outcome::SUCCEEDED));
}

} // namespace oscillator
} //namespace session
} // namespace smtk
