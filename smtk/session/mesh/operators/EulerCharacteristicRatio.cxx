//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/mesh/operators/EulerCharacteristicRatio.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/Model.h"

#include "smtk/session/mesh/operators/EulerCharacteristicRatio_xml.h"

using namespace smtk::model;
using namespace smtk::common;

namespace smtk
{
namespace session
{
namespace mesh
{

EulerCharacteristicRatio::Result EulerCharacteristicRatio::operateInternal()
{
  // Access the associated model.
  smtk::model::Model model = this->parameters()->associatedModelEntities<smtk::model::Models>()[0];
  if (!model.isValid())
  {
    smtkErrorMacro(this->log(), "Invalid model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  smtk::session::mesh::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::mesh::Resource>(model.component()->resource());
  smtk::session::mesh::Session::Ptr session = resource->session();

  // Access the underlying mesh resource for the model.
  smtk::mesh::ResourcePtr meshResource = resource->resource();
  if (meshResource == nullptr || !meshResource->isValid())
  {
    smtkErrorMacro(this->log(), "No mesh resource associated with this model.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  // Access the meshes from the mesh resource.
  smtk::mesh::MeshSet mesh = meshResource->meshes();

  // Compute the Euler characteristics for the model's boundary and volume.
  int eulerBoundary = smtk::mesh::utility::eulerCharacteristic(mesh.extractShell());
  int eulerVolume = smtk::mesh::utility::eulerCharacteristic(mesh);

  // Compute the ratio of these two values.
  double eulerRatio = ((double)(eulerBoundary)) / ((double)(eulerVolume));

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  // Set the double item that will hold the euler ratio.
  smtk::attribute::DoubleItemPtr eulerItem = result->findDouble("value");
  eulerItem->setValue(eulerRatio);

  std::stringstream s;
  s << "Input mesh has ratio of boundary/volume Euler characteristics = " << eulerBoundary << " / "
    << eulerVolume << " = " << eulerRatio << ".";
  smtkInfoMacro(this->log(), s.str());

  return result;
}

const char* EulerCharacteristicRatio::xmlDescription() const
{
  return EulerCharacteristicRatio_xml;
}

} // namespace mesh
} // namespace session
} // namespace smtk
