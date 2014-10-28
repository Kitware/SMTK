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
#include "smtk/io/ExportJSON.h"

#include "smtk/model/Bridge.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/Manager.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/AutoInit.h"

#include <fstream>

using namespace smtk::io;
using namespace smtk::model;
using namespace smtk::common;

smtkComponentInitMacro(smtk_cgm_bridge);
smtkComponentInitMacro(smtk_cgm_create_sphere_operator);

int main()
{
  Manager::Ptr mgr = Manager::create();
  Bridge::Ptr brg = mgr->createBridge("cgm");
  Operator::Ptr op = brg->op("create sphere", mgr);
  //op->ensureSpecification();
  //op->spec()->findDouble("radius");
  OperatorResult result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    std::cerr << "Fail\n";
    return 1;
    }

  smtk::attribute::ModelEntityItem::Ptr bodies = result->findModelEntity("bodies");
  std::cout << "Created " << bodies->value().flagSummary() << "\n";
  std::cout << "   with " << bodies->value().as<ModelEntity>().cells().size() << " cells\n";
  std::ofstream json("/tmp/sphere.json");
  json << ExportJSON::fromModel(mgr);
  json.close();
  //ExportJSON::fromModel(

  return 0;
}
