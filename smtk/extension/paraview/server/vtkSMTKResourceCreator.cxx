//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceCreator.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/SessionRef.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/ReadResource.h"

#include "smtk/resource/Manager.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/json/jsonResource.h"

#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

vtkStandardNewMacro(vtkSMTKResourceCreator);

vtkSMTKResourceCreator::vtkSMTKResourceCreator()
{
  this->TypeName = nullptr;
  this->Parameters = nullptr;

  // Ensure this object's MTime > this->ModelSource's MTime so first RequestData() call
  // results in the filter being updated:
  this->Modified();
}

vtkSMTKResourceCreator::~vtkSMTKResourceCreator()
{
  this->SetTypeName(nullptr);
  this->SetParameters(nullptr);
}

void vtkSMTKResourceCreator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "TypeName: " << this->TypeName << "\n";
  os << indent << "Parameters: " << this->Parameters << "\n";
}

smtk::resource::ResourcePtr vtkSMTKResourceCreator::GenerateResource() const
{
  if (!this->Resource.expired() && this->Wrapper)
  {
    this->Wrapper->GetResourceManager()->remove(this->GetResource());
  }

  if (!this->TypeName || !this->Parameters)
  {
    return smtk::resource::ResourcePtr();
  }

  smtk::resource::Manager::Ptr rsrcMgr;
  if (this->Wrapper != nullptr)
  {
    rsrcMgr = this->Wrapper->GetResourceManager();
  }

  smtk::operation::Manager::Ptr operMgr;
  if (this->Wrapper != nullptr)
  {
    operMgr = this->Wrapper->GetOperationManager();
  }

  if (!operMgr)
  {
    return smtk::resource::ResourcePtr();
  }

  auto oper = operMgr->create(std::string(this->TypeName));
  if (!oper)
  {
    return smtk::resource::ResourcePtr();
  }

  json j;

  try
  {
    j = json::parse(this->Parameters);
  }
  catch (std::exception&)
  {
    return smtk::resource::ResourcePtr();
  }
  auto parameters = oper->parameters();
  std::vector<smtk::attribute::ItemExpressionInfo> itemExpressionInfo;
  std::vector<smtk::attribute::AttRefInfo> attRefInfo;
  std::set<const smtk::attribute::ItemDefinition*> convertedAttDefs;
  smtk::attribute::from_json(j, parameters, itemExpressionInfo, attRefInfo, convertedAttDefs);

  auto result = oper->operate();
  if (
    result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }

  return result->findResource("resourcesCreated")->value(0);
}
