//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/oscillator/operators/EditDomain.h"

#include "smtk/session/oscillator/Resource.h"
#include "smtk/session/oscillator/SimulationAttribute.h"

#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/UUID.h"

#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

#include "smtk/mesh/core/CellTraits.h"
#include "smtk/mesh/core/CellTypes.h"

#include "smtk/mesh/utility/Create.h"

#include "smtk/session/oscillator/operators/EditDomain_xml.h"

namespace smtk
{
namespace session
{
namespace oscillator
{

EditDomain::Result EditDomain::operateInternal()
{
  // Access the string describing the dimension. The dimension is a string so we
  // can use optional children to assign the lengths of the other parameters.
  smtk::attribute::StringItem::Ptr dimensionItem = this->parameters()->findString("dimension");

  int dimension;
  std::string suffix;
  if (dimensionItem->value() == "2")
  {
    dimension = 2;
    suffix = "2d";
  }
  else
  {
    dimension = 3;
    suffix = "3d";
  }

  // Access the origin and size parameters
  smtk::attribute::DoubleItemPtr originItem = this->parameters()->findDouble("origin" + suffix);
  smtk::attribute::DoubleItemPtr sizeItem = this->parameters()->findDouble("size" + suffix);

  // Copy their values to local fields
  smtk::model::FloatList origin{ 0., 0., 0. };
  smtk::model::FloatList size{ 0., 0., 0. };

  for (int i = 0; i < dimension; i++)
  {
    origin[i] = originItem->value(i);
    size[i] = sizeItem->value(i);
  }

  // Construct a mapping from a unit box to the input box
  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>(
      { { origin[0] + size[0] * x[0], origin[1] + size[1] * x[1], origin[2] + size[2] * x[2] } });
  };

  smtk::session::oscillator::Resource::Ptr resource;
  auto inputItem = this->parameters()->associations();
  auto inputValue = inputItem->valueAs<smtk::model::Entity>(0);
  smtk::model::Model model;
  smtk::model::Volume volume;
  bool modelCreated = false;
  bool volumeCreated = false;
  // We could be provided a model or a model-resource:
  if (inputValue)
  {
    resource =
      std::static_pointer_cast<smtk::session::oscillator::Resource>(inputValue->resource());
    model = inputValue->referenceAs<smtk::model::Model>();
    if (!model.isValid())
    {
      volume = inputValue->referenceAs<smtk::model::Volume>();
      model = volume.owningModel();
    }
  }
  else
  {
    resource = inputItem->valueAs<smtk::session::oscillator::Resource>(0);
  }
  // ... or nothing at all, in which case we create a new resource.
  if (!resource)
  {
    resource = smtk::session::oscillator::Resource::create();
    resource->setName("new oscillator geometry");
  }

  if (!model.isValid())
  {
    model = resource->addModel(dimension, dimension, "simulation domain");
    modelCreated = true;
  }

  if (!volume.isValid())
  {
    volume = resource->addVolume();
    model.addCell(volume);
    volume.assignDefaultName();
    volumeCreated = true;
  }

  volume.setFloatProperty("origin", origin);
  volume.setFloatProperty("size", size);
  resource->resetDomainTessellation(volume);

  smtk::attribute::Resource::Ptr sim;
  auto resourceManager = resource->manager();
  bool skipAssociation = false;
  if (resourceManager)
  {
    auto potentialSims = resourceManager->find<smtk::attribute::Resource>();
    // First see if there are attributes already associated to us (in the "edit" case)
    for (const auto& potentialSim : potentialSims)
    {
      auto assocs = potentialSim->associations();
      auto assoc = assocs.find(resource);
      if (assoc != assocs.end())
      {
        sim = potentialSim;
        skipAssociation = true;
        break;
      }
    }
    if (!skipAssociation)
    {
      // We need to associate ourselves to a simulation attribute
      for (const auto& potentialSim : potentialSims)
      {
        if (potentialSim && potentialSim->findDefinition("source-term"))
        {
          // TODO: We could also examine its "Accepts" entries to verify
          //       that it requires smtk::session::oscillator::Resources
          //       before choosing potentialSim as "our" sim.
          auto assocs = potentialSim->associations();
          if (assocs.find(resource) != assocs.end())
          {
            skipAssociation = true;
          }
          sim = potentialSim;
          break;
        }
      }
    }
  }
  if (!sim)
  {
    sim = SimulationAttribute::create();
    sim->setName("new oscillator simulation"); // TODO: Turn this into a JSON SMTK file.
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  if (!skipAssociation && sim)
  {
    // Use a resource link to associate ourselves to the identified simulation attribute.
    bool success = sim->associate(resource);
    // NB: On success, see below where we append sim to the result's "resource" item
    //     since skipAssociation is false.
    if (!success)
    {
      skipAssociation = true;
      smtkWarningMacro(this->log(), "Could not create or identify a simulation attribute to use.");
    }
  }

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
    if (sim && !skipAssociation)
    {
      created->appendValue(sim);
    }
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    if (modelCreated)
    {
      created->appendValue(model.component());
    }
    else
    {
      modified->appendValue(model.component());
    }
    if (volumeCreated)
    {
      created->appendValue(volume.component());
    }
    else
    {
      modified->appendValue(volume.component());
    }
  }

  return result;
}

const char* EditDomain::xmlDescription() const
{
  return EditDomain_xml;
}
} // namespace oscillator
} // namespace session
} // namespace smtk
