//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/operators/EulerCharacteristic.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/Metrics.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace mesh
{

smtk::model::OperatorResult EulerCharacteristic::operateInternal()
{
  // ableToOperate should have verified that mesh(s) are set
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Compute the Euler characteristic.
  int euler = eulerCharacteristic(meshItem->value());

  // Create a result object to store the output.
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  // Set the int item that will hold the euler characteristic.
  smtk::attribute::IntItemPtr eulerItem = result->findInt("value");
  eulerItem->setValue(euler);

  std::stringstream s;
  s << "Input mesh has Euler characteristic = " << euler << ".";
  smtkInfoMacro(this->log(), s.str());

  return result;
}
}
}

#include "smtk/mesh/EulerCharacteristic_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::mesh::EulerCharacteristic, euler_characteristic,
  "euler characteristic", EulerCharacteristic_xml, smtk::model::Session);
