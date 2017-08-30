//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/mesh/operators/EulerCharacteristicRatio.h"

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Metrics.h"

#include "smtk/model/Model.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace bridge
{
namespace mesh
{

smtk::model::OperatorResult EulerCharacteristicRatio::operateInternal()
{
  // Access the associated model.
  smtk::model::Model model =
    this->specification()->associatedModelEntities<smtk::model::Models>()[0];
  if (!model.isValid())
  {
    smtkErrorMacro(this->log(), "Invalid model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  // Access the underlying mesh collection for the model.
  smtk::mesh::CollectionPtr collection =
    this->session()->meshManager()->findCollection(model.entity())->second;
  if (!collection->isValid())
  {
    smtkErrorMacro(this->log(), "No collection associated with this model.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  // Access the meshes from the collection.
  smtk::mesh::MeshSet mesh = collection->meshes();

  // Compute the Euler characteristics for the model's boundary and volume.
  int eulerBoundary = eulerCharacteristic(mesh.extractShell());
  int eulerVolume = eulerCharacteristic(mesh);

  // Compute the ratio of these two values.
  double eulerRatio = ((double)(eulerBoundary)) / ((double)(eulerVolume));

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  // Set the double item that will hold the euler ratio.
  smtk::attribute::DoubleItemPtr eulerItem = result->findDouble("value");
  eulerItem->setValue(eulerRatio);

  std::stringstream s;
  s << "Input mesh has ratio of boundary/volume Euler characteristics = " << eulerBoundary << " / "
    << eulerVolume << " = " << eulerRatio << ".";
  smtkInfoMacro(this->log(), s.str());

  return result;
}

} // namespace mesh
} // namespace bridge
} // namespace smtk

#include "smtk/mesh/EulerCharacteristicRatio_xml.h"

smtkImplementsModelOperator(SMTKMESHSESSION_EXPORT, smtk::bridge::mesh::EulerCharacteristicRatio,
  euler_characteristic_ratio, "euler characteristic ratio", EulerCharacteristicRatio_xml,
  smtk::bridge::mesh::Session);
