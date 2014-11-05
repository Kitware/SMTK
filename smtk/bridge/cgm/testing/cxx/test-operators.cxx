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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/AutoInit.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <fstream>

using namespace smtk::io;
using namespace smtk::model;
using namespace smtk::common;

smtkComponentInitMacro(smtk_cgm_bridge);
smtkComponentInitMacro(smtk_cgm_boolean_union_operator);
smtkComponentInitMacro(smtk_cgm_create_sphere_operator);
smtkComponentInitMacro(smtk_cgm_create_prism_operator);

int main(int argc, char* argv[])
{
  Manager::Ptr mgr = Manager::create();
  Bridge::Ptr brg = mgr->createBridge("cgm");
  StringList err(1);
  err[0] = "0.001"; brg->setup("tessellation maximum relative chord error", err);
  err[0] = "2.0"; brg->setup("tessellation maximum angle error", err);
  Operator::Ptr op;
  OperatorResult result;

  op = brg->op("create sphere", mgr);
  op->findDouble("radius")->setValue(argc > 1 ? atof(argv[1]) : 10.);
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    std::cerr << "Sphere Fail\n";
    return 1;
    }
  ModelEntity sphere = result->findModelEntity("bodies")->value();

  op = brg->op("create prism", mgr);
  op->findInt("number of sides")->setValue(6);
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    std::cerr << "Prism Fail\n";
    return 1;
    }
  ModelEntity prism = result->findModelEntity("bodies")->value();

  op = brg->op("union", mgr);
  op->ensureSpecification();
  test(op->associateEntity(sphere), "Could not associate sphere to union operator");
  test(op->associateEntity(prism), "Could not associate prism to union operator");
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    std::cerr << "Union Fail\n";
    return 1;
    }

  smtk::attribute::ModelEntityItem::Ptr bodies = result->findModelEntity("bodies");
  std::cout << "Created " << bodies->value().flagSummary() << "\n";
  std::cout << "   with " << bodies->value().as<ModelEntity>().cells().size() << " cells\n";
  std::ofstream json("/tmp/sphere.json");
  json << ExportJSON::fromModel(mgr);
  json.close();

  return 0;
}
