//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/session/oscillator/SimulationAttribute.h"
#include "smtk/session/oscillator/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Volume.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include <set>
#include <string>

#include "smtk/session/oscillator/SimulationAttribute_xml.h"

using namespace smtk::session::oscillator;
using AuxGeomSet = std::set<smtk::model::AuxiliaryGeometry>;
using EntitySet = std::set<smtk::model::EntityPtr>;

smtk::attribute::Resource::Ptr SimulationAttribute::create()
{
  smtk::attribute::Resource::Ptr resource = smtk::attribute::Resource::create();

  smtk::io::AttributeReader reader;
  bool ok = reader.readContents(resource, SimulationAttribute_xml, smtk::io::Logger::instance());
  (void)ok;
  return resource;
}

bool SimulationAttribute::lint(
  const smtk::attribute::ResourcePtr& simulation, smtk::io::Logger& feedback)
{
  if (!simulation)
  {
    smtkErrorMacro(feedback, "Cannot lint a null simulation");
    return false;
  }

  // I. Identify the geometric model associated with \a simulation.
  auto resources = simulation->associations();
  smtk::session::oscillator::Resource::Ptr modelResource;
  for (const auto& resource : resources)
  {
    modelResource = std::dynamic_pointer_cast<smtk::session::oscillator::Resource>(resource);
    if (modelResource)
    {
      break;
    }
  }
  if (!modelResource)
  {
    smtkErrorMacro(feedback, "No geometric model resource associated with the simulation.");
    return false;
  }
  m_geometry = modelResource;

  // II. Find the simulation's geometric domain.
  auto volumes = modelResource->entitiesMatchingFlagsAs<smtk::model::Volumes>(
    smtk::model::VOLUME, /* exactMatch */ true);
  if (volumes.empty())
  {
    smtkErrorMacro(feedback, "No simulation domain included in the geometric model.");
    return false;
  }
  if (volumes.size() > 1)
  {
    smtkErrorMacro(feedback, "The oscillator simulation only supports a single domain but "
        << volumes.size() << " were found.");
    return false;
  }
  auto volume = *volumes.begin();
  m_domain = volume.entityRecord();

  // III. Obtain a list of generator points; then verify it
  //      is self-consistent and complete.
  auto sourcePoints = modelResource->entitiesMatchingFlagsAs<AuxGeomSet>(
    smtk::model::AUX_GEOM_ENTITY | smtk::model::DIMENSION_2, /* exactMatch */ true);
  if (sourcePoints.empty())
  {
    smtkErrorMacro(feedback, "No oscillator source-points included in the geometric model.");
    return false;
  }

  auto sourceAttribs = simulation->findAttributes("source-term");
  for (const auto& sourceAttrib : sourceAttribs)
  {
    auto assocPoints = sourceAttrib->associatedObjects<EntitySet>();
    if (assocPoints.empty())
    {
      smtkWarningMacro(feedback, "Source term \"" << sourceAttrib->name()
                                                  << "\" has no associated source point geometry.");
      continue;
    }
    for (const auto& assocPoint : assocPoints)
    {
      auto sourcePoint = assocPoint->referenceAs<smtk::model::AuxiliaryGeometry>();
      sourcePoints.erase(sourcePoint);
    }
  }
  if (!sourcePoints.empty())
  {
    smtkErrorMacro(feedback, "There are "
        << sourcePoints.size() << " source points with no associated source term information.");
    return false;
  }
  m_sources = sourceAttribs;
  return true;
}
