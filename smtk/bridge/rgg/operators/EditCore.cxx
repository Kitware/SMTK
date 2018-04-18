//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditCore.h"
#include "smtk/bridge/rgg/operators/CreateModel.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"

#include "smtk/bridge/rgg/EditCore_xml.h"

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditCore::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);

  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  smtk::model::EntityRefArray entities = this->associatedEntitiesAs<smtk::model::EntityRefArray>();
  if (entities.empty() || !entities[0].isGroup())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non group type core");
    return result;
  }
  if (!sess)
  {
    return result;
  }

  smtk::model::Group core = entities[0].as<smtk::model::Group>();
  smtk::model::Model model = core.owningModel();
  CreateModel::populateCore(this, core);
  smtk::model::FloatList ductH = core.owningModel().floatProperty("duct height");
  if (ductH.size() != 2)
  {
    smtkErrorMacro(this->log(), "The core does not have a valid duct height value");
    return result;
  }

  // Remove and add instances
  smtk::attribute::ModelEntityItemPtr dI = this->findModelEntity("instance to be deleted");
  for (auto it = dI->begin(); it != dI->end(); it++)
  {
    core.removeEntity(*it);
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  smtk::attribute::ModelEntityItemPtr aI = this->findModelEntity("instance to be added");
  for (auto it = aI->begin(); it != aI->end(); it++)
  {
    // Instances would inheret its related prototype's color
    smtk::model::Instance instance = it->as<smtk::model::Instance>();
    instance.setColor(instance.prototype().color());
    std::string iName = "instance_Of_" + instance.prototype().name();
    instance.setName(iName);
    this->addEntityToResult(result, instance, MODIFIED);

    core.addEntity(*it);
  }
  std::string optype = this->findString("geometry type")->value();
  if (optype == "Hex")
  {
    model.setIntegerProperty("hex", 1);
    smtk::attribute::DoubleItemPtr thicknessI = this->findDouble("duct thickness");
    smtk::model::FloatList tmp = { thicknessI->value(0), thicknessI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  else if (optype == "Rect")
  {
    model.setIntegerProperty("hex", 0);
    smtk::attribute::DoubleItemPtr thicknessXI = this->findDouble("duct thickness X");
    smtk::attribute::DoubleItemPtr thicknessYI = this->findDouble("duct thickness Y");
    smtk::model::FloatList tmp = { thicknessXI->value(0), thicknessYI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }

  // Update ducts. Duck thickness is taken care of by the model, so we only need
  // to use the "duct height" property on the model to update the z values in each
  // duct.
  // TODO: It might be a performance bottleneck since modifying ducts would
  // trigger the glyph3DMapper to render the ducts again
  smtk::model::EntityRefArray ducts =
    sess->manager()->findEntitiesByProperty("rggType", SMTK_BRIDGE_RGG_DUCT);
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

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  // FIXME: Since pqCMBModelManager cannot handle modifed auxiliary geometries properly
  // (L2055 where internal_isNewGeometricBlock does not work on a modified auxgeom).
  // We just mark them as CREATED.
  this->addEntitiesToResult(result, ducts, CREATED);
  this->addEntityToResult(result, core, MODIFIED);

  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditCore, rgg_edit_core,
  "edit core", EditCore_xml, smtk::bridge::rgg::Session);
