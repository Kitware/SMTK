//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/EditCore.h"

#include "smtk/session/rgg/Resource.h"
#include "smtk/session/rgg/Session.h"

#include "smtk/session/rgg/operators/CreateModel.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/session/rgg/operators/CreateDuct.h"

#include "smtk/session/rgg/EditCore_xml.h"

namespace smtk
{
namespace session
{
namespace rgg
{

EditCore::Result EditCore::operateInternal()
{
  Result result = this->createResult(smtk::operation::Operation::Outcome::FAILED);

  smtk::model::EntityRefArray entities =
    this->parameters()->associatedModelEntities<smtk::model::EntityRefArray>();
  if (entities.empty() || !entities[0].isGroup())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non group type core");
    return result;
  }

  smtk::model::Group core = entities[0].as<smtk::model::Group>();
  smtk::model::Model model = core.owningModel();
  auto resource = std::dynamic_pointer_cast<smtk::session::rgg::Resource>(model.resource());
  CreateModel::populateCore(this, core);
  smtk::model::FloatList ductH = core.owningModel().floatProperty("duct height");
  if (ductH.size() != 2)
  {
    smtkErrorMacro(this->log(), "The core does not have a valid duct height value");
    return result;
  }

  // Remove and add instances
  smtk::attribute::ModelEntityItemPtr dI =
    this->parameters()->findModelEntity("instance to be deleted");
  for (auto it = dI->begin(); it != dI->end(); it++)
  {
    auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    core.removeEntity(smtk::model::EntityRef(entity->modelResource(), entity->id()));
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr aI =
    this->parameters()->findModelEntity("instance to be added");
  for (auto it = aI->begin(); it != aI->end(); it++)
  {
    auto entity = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    auto eRef = smtk::model::EntityRef(entity->modelResource(), entity->id());
    // Instances would inheret its related prototype's color
    smtk::model::Instance instance = eRef.as<smtk::model::Instance>();
    instance.setColor(instance.prototype().color());
    std::string iName = "instance_Of_" + instance.prototype().name();
    instance.setName(iName);
    result->findComponent("modified")->appendValue(instance.component());

    core.addEntity(eRef);
  }
  std::string optype = this->parameters()->findString("geometry type")->value();
  if (optype == "Hex")
  {
    model.setIntegerProperty("hex", 1);
    smtk::attribute::DoubleItemPtr thicknessI = this->parameters()->findDouble("duct thickness");
    smtk::model::FloatList tmp = { thicknessI->value(0), thicknessI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  else if (optype == "Rect")
  {
    model.setIntegerProperty("hex", 0);
    smtk::attribute::DoubleItemPtr thicknessXI = this->parameters()->findDouble("duct thickness X");
    smtk::attribute::DoubleItemPtr thicknessYI = this->parameters()->findDouble("duct thickness Y");
    smtk::model::FloatList tmp = { thicknessXI->value(0), thicknessYI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }

  // Update ducts. Duck thickness is taken care of by the model, so we only need
  // to use the "duct height" property on the model to update the z values in each
  // duct.
  // TODO: It might be a performance bottleneck since modifying ducts would
  // trigger the glyph3DMapper to render the ducts again
  smtk::model::EntityRefArray ducts =
    resource->findEntitiesByProperty("rggType", SMTK_SESSION_RGG_DUCT);
  for (auto iter = ducts.begin(); iter != ducts.end(); iter++)
  {
    smtk::model::AuxiliaryGeometry ductA = iter->as<smtk::model::AuxiliaryGeometry>();
    if (!(ductA.auxiliaryGeometries().size() > 0) || // duct subparts
      (!ductA.hasFloatProperty("z values") || ductA.floatProperty("z values").size() < 2))
    {
      smtkErrorMacro(this->log(), "Duct " << iter->name() << " does not have valid z values");
      continue;
    }
    smtk::model::FloatList& zValues = ductA.floatProperty("z values");
    double oldZO = zValues[0], newZO = ductH[0];
    double ratio = (ductH[1] - ductH[0]) / (zValues[zValues.size() - 1] - zValues[0]);
    // Substract the original z origin, multiply the size change ratio then
    // add the new z origin.
    std::transform(zValues.begin(), zValues.end(), zValues.begin(),
      [&oldZO, &newZO, &ratio](double v) { return ((v - oldZO) * ratio + newZO); });
  }

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // FIXME: Since pqCMBModelManager cannot handle modifed auxiliary geometries properly
  // (L2055 where internal_isNewGeometricBlock does not work on a modified auxgeom).
  // We just mark them as CREATED.
  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  for (auto& c : ducts)
  {
    createdItem->appendValue(c.component());
  }
  result->findComponent("modified")->appendValue(core.component());

  return result;
}

const char* EditCore::xmlDescription() const
{
  return EditCore_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
