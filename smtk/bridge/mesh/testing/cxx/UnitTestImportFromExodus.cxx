//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/mesh/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

namespace
{
std::string dataRoot = SMTK_DATA_DIR;

void UniqueEntities(const smtk::model::EntityRef& root, std::set<smtk::model::EntityRef>& unique)
{
  smtk::model::EntityRefArray children = (root.isModel()
      ? root.as<smtk::model::Model>().cellsAs<smtk::model::EntityRefArray>()
      : (root.isCellEntity()
            ? root.as<smtk::model::CellEntity>().boundingCellsAs<smtk::model::EntityRefArray>()
            : (root.isGroup() ? root.as<smtk::model::Group>().members<smtk::model::EntityRefArray>()
                              : smtk::model::EntityRefArray())));

  for (smtk::model::EntityRefArray::const_iterator it = children.begin(); it != children.end();
       ++it)
  {
    if (unique.find(*it) == unique.end())
    {
      unique.insert(*it);
      UniqueEntities(*it, unique);
    }
  }
}

void ParseModelTopology(smtk::model::Model& model, std::size_t* count)
{
  std::set<smtk::model::EntityRef> unique;
  UniqueEntities(model, unique);

  for (auto&& entity : unique)
  {
    if (entity.dimension() >= 0 && entity.dimension() <= 3)
    {
      count[entity.dimension()]++;
      float r = static_cast<float>(entity.dimension()) / 3;
      float b = static_cast<float>(1.) - r;
      const_cast<smtk::model::EntityRef&>(entity).setColor(
        (r < 1. ? r : 1.), 0., (b < 1. ? b : 1.), 1.);
    }
  }
}
}

int UnitTestImportFromExodus(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available sessions\n";
  smtk::model::StringList sessions = manager->sessionTypeNames();
  for (smtk::model::StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::mesh::Session::Ptr session = smtk::bridge::mesh::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  smtk::model::StringList opnames = session->operatorNames();
  for (smtk::model::StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::OperatorPtr importOp = session->op("import");
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  std::string importFilePath(dataRoot);
  importFilePath += "/model/3d/genesis/gun-1fourth.gen";

  importOp->specification()->findFile("filename")->setValue(importFilePath);
  importOp->specification()->findVoid("construct hierarchy")->setIsEnabled(false);

  smtk::model::OperatorResult importOpResult = importOp->operate();

  if (importOpResult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Import operator failed\n";
    return 1;
  }

  smtk::model::Model model = importOpResult->findModelEntity("model")->value();

  if (!model.isValid())
  {
    std::cerr << "Import operator returned an invalid model\n";
    return 1;
  }

  std::size_t count[4] = { 0, 0, 0, 0 };
  ParseModelTopology(model, count);

  std::cout << count[3] << " volumes" << std::endl;
  test(count[3] == 1, "There should be one volume");
  std::cout << count[2] << " faces" << std::endl;
  test(count[2] == 5, "There should be five faces");
  std::cout << count[1] << " edges" << std::endl;
  test(count[1] == 0, "There should be no lines");
  std::cout << count[0] << " vertex groups" << std::endl;
  test(count[0] == 0, "There should be no vertex groups");

  return 0;
}
