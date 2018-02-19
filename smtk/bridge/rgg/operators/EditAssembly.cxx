//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditAssembly.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/operators/CreateInstances.h"

#include "smtk/bridge/rgg/EditAssembly_xml.h"

#include <limits>
#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditAssembly::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);

  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess || !sess->manager())
  {
    return result;
  }
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isGroup())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non group type entity");
    return result;
  }

  smtk::model::Group assembly = entities[0].as<smtk::model::Group>();
  CreateAssembly::populateAssembly(this, assembly);
  // Remove and add instances
  smtk::attribute::ModelEntityItemPtr dI = this->findModelEntity("instance to be deleted");
  for (auto it = dI->begin(); it != dI->end(); it++)
  {
    std::cout << "Remove " << it->name() << " from assembly" << std::endl;
    assembly.removeEntity(*it);
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr aI = this->findModelEntity("instance to be added");
  for (auto it = aI->begin(); it != aI->end(); it++)
  {
    // Instances would inheret its related prototype's color
    smtk::model::Instance instance = it->as<smtk::model::Instance>();
    instance.setColor(instance.prototype().color());
    this->addEntityToResult(result, instance, MODIFIED);

    std::cout << "add " << it->name() << " into assembly" << std::endl;
    assembly.addEntity(*it);
  }

  this->addEntityToResult(result, assembly, MODIFIED);
  return result;

  //  // Create instances for pins and duct
  //  smtk::attribute::ModelEntityItemPtr ductItem = this->findModelEntity("associated duct");
  //  smtk::model::EntityRef duct = ductItem->value();
  //  this->generateInstances(duct,smtk::model::IntegerList(), result);

  //  // Get duct innest size
  //  smtk::model::FloatList pitches, thicknesses;
  //  if (duct.hasFloatProperty("pitch"))
  //  {
  //    pitches = duct.floatProperty("pitch");
  //    if (pitches.size() != 2)
  //    {
  //      smtkErrorMacro(smtk::io::Logger::instance(),"duct " << duct.name() <<
  //                     " does not have a valid pitch");
  //      return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  //    }
  //  }
  //  if (duct.hasFloatProperty("thicknesses(normalized)"))
  //  {
  //    thicknesses = duct.floatProperty("thicknesses(normalized)");
  //    if (thicknesses.size() / 2 < 1)
  //    {
  //      smtkErrorMacro(smtk::io::Logger::instance(),"duct " << duct.name() <<
  //                     " does not have valid thicknesses");
  //      return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  //    }
  //  }
  //  double thickness0(std::numeric_limits<double>::max()),
  //         thickness1(std::numeric_limits<double>::max());
  //  for (auto i = 0; i < thicknesses.size() / 2; i++)
  //  {
  //    double currentT0 = pitches[0] * thicknesses[i*2];
  //    double currentT1 = pitches[1] * thicknesses[i*2 + 1];
  //    thickness0 = (currentT0 < thickness0) ? currentT0 : thickness0;
  //    thickness1 = (currentT1 < thickness1) ? currentT1 : thickness1;
  //  }
  //  std::cout << "  thickness0=" << thickness0 << " thickness1=" << thickness1
  //            <<std::endl;

  //  // Get lattice size
  //  smtk::attribute::IntItemPtr latticeSizeItem = this->findInt("lattice size");
  //  smtk::model::IntegerList latticeSize;
  //  if (latticeSizeItem && latticeSizeItem->numberOfValues() == 2)
  //  {
  //      latticeSize = smtk::model::IntegerList(latticeSizeItem->begin(), latticeSizeItem->end());
  //  /******************************************************/
  //  std::cout << "  lattice size=" << latticeSize[0] <<" " << latticeSize[1] <<std::endl;
  //  /******************************************************/
  //  }
  //  else
  //  {
  //    smtkErrorMacro(this->log(), "Cannot edit an assembly without a valid lattice  size");
  //    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  //  }
  //  double pitchXSpace = thickness0 / static_cast<double>(latticeSize[0]);
  //  double pitchYSpace = thickness1 / static_cast<double>(latticeSize[1]);

  //  smtk::attribute::GroupItemPtr piecesGItem = this->findGroup("pins and layouts");
  //  size_t numPins;
  //  if (piecesGItem != nullptr)
  //  {
  //    numPins = piecesGItem->numberOfGroups();
  //    for (std::size_t index = 0; index < numPins; index++)
  //    {
  //      smtk::attribute::StringItemPtr pinIdItem = piecesGItem->findAs
  //          <smtk::attribute::StringItem>(index, "pin UUID");
  //      smtk::attribute::IntItemPtr schemaPlanItem = piecesGItem->findAs<smtk::attribute::IntItem>(index, "schema plan");

  //      std::string uuid = pinIdItem->value();
  //      smtk::model::IntegerList layout(schemaPlanItem->begin(), schemaPlanItem->end());
  //      smtk::model::EntityRef pin = sess->manager()->findEntity(uuid);
  //      if (pin.isValid() && layout.size() > 0)
  //      {
  //        this->generateInstances(pin, layout, result, pitchXSpace, pitchYSpace);
  //      }
  //    }
  //  }

  //  this->addEntityToResult(result, auxGeom, MODIFIED);
  //  return result;
}

void EditAssembly::generateInstances(const smtk::model::EntityRef& prototype,
  const smtk::model::IntegerList& layout, smtk::model::OperatorResult& result, double pitchXSpace,
  double pitchYSpace)

{ // Following the logic in create instance op
  ManagerPtr mgr = prototype.manager();
  if (mgr)
  {
    Instance instance = mgr->addInstance(prototype);
    instance.assignDefaultName();
    this->addEntityToResult(result, prototype, MODIFIED);
    this->addEntityToResult(result, instance, CREATED);
    result->findModelEntity("tess_changed")->appendValue(instance);

    instance.setRule("tabular");
    smtk::model::FloatList pprop;
    if (layout.size() == 0)
    {
      pprop = { 0.0, 0.0, 0.0 };
    }
    else
    {
      size_t numPlace = layout.size() / 2;
      pprop.reserve(numPlace * 3); // 2d to 3D
      for (size_t index = 0; index < numPlace; index++)
      {
        pprop.push_back(layout[2 * index] * pitchXSpace);
        pprop.push_back(layout[2 * index + 1] * pitchYSpace);
        pprop.push_back(0);
      }
    }
    /******************************************************************/
    std::cout << "  generateInstances for entity " << prototype.name()
              << "\n    layout: " << std::endl;
    for (double i : pprop)
    {
      std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "    pitchXSpace=" << pitchXSpace << " pitchYSpace=" << pitchYSpace << std::endl;
    /******************************************************************/
    instance.setFloatProperty("placements", pprop);

    // Now that the instance is fully specified, generate
    // the placement points.
    instance.generateTessellation();
  }
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditAssembly,
  rgg_edit_assembly, "edit assembly", EditAssembly_xml, smtk::bridge::rgg::Session);
