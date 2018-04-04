//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/ExportMaterialList.h"

#include "smtk/bridge/rgg/Material.h"
#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/ExportMaterialList_xml.h"

#include <fstream>

namespace smtk
{
namespace bridge
{
namespace rgg
{

bool ExportMaterialList::ableToOperate()
{
  std::cout << "Specification?" << std::endl;
  if (!this->ensureSpecification())
  {
    std::cout << "no" << std::endl;
    return false;
  }
  std::cout << "yes" << std::endl;

  // Access the associated model
  smtk::model::EntityRef model = this->associatedEntitiesAs<smtk::model::EntityRefArray>()[0];

  std::cout << "hasStringProperty: " << model.hasStringProperty(Material::label) << std::endl;

  return model.hasStringProperty(Material::label);
}

smtk::model::OperatorResult ExportMaterialList::operateInternal()
{
  // Access the associated model
  smtk::model::EntityRef model = this->associatedEntitiesAs<smtk::model::EntityRefArray>()[0];

  // Access the name of the file to be written.
  std::string filename = this->specification()->findAs<attribute::FileItem>("filename")->value();

  smtk::model::StringList& materialDescriptions = model.stringProperty(Material::label);

  std::ofstream outfile(filename.c_str());

  for (auto& materialDescription : materialDescriptions)
  {
    outfile << materialDescription << "\n";
  }
  outfile.close();

  return this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::ExportMaterialList,
  rgg_write_materials, "write materials", ExportMaterialList_xml, smtk::bridge::rgg::Session);
