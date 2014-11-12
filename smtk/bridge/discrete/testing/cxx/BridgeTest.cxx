//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Cursor.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/Face.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Tessellation.h"
#include "smtk/io/ExportJSON.h"

#include <fstream>

static int maxIndent = 10;

using namespace smtk::model;

void prindent(std::ostream& os, int indent, smtk::model::DescriptivePhrase::Ptr p)
{
  // Do not descend too far, as infinite recursion is possible,
  // even with the SimpleSubphraseGenerator
  if (indent > maxIndent)
    return;

  os << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
  smtk::model::FloatList rgba = p->relatedColor();
  if (rgba[3] >= 0.)
    os << " rgba(" << rgba[0] << "," << rgba[1] << "," << rgba[2] << "," << rgba[3] << ")";
  os << "\n";
  smtk::model::DescriptivePhrases sub = p->subphrases();
  indent += 2;
  for (smtk::model::DescriptivePhrases::iterator it = sub.begin(); it != sub.end(); ++it)
    {
    prindent(os, indent, *it);
    }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;

  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  std::cout << "Available bridges\n";
  StringList bridges = manager->bridgeNames();
  for (StringList::iterator it = bridges.begin(); it != bridges.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::discrete::Bridge::Ptr bridge = smtk::bridge::discrete::Bridge::create();
  manager->registerBridgeSession(bridge);

  std::cout << "Available cmb operators\n";
  StringList opnames = bridge->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::OperatorPtr readOp = bridge->op("read", manager);
  if (!readOp)
    {
    std::cerr << "No read operator\n";
    return 1;
    }

  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult result = readOp->operate();
  if (
    result->findInt("outcome")->value() !=
    smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Read operator failed\n";
    return 1;
    }

  smtk::model::ModelEntity model = result->findModelEntity("model")->value();
  manager->assignDefaultNames(); // should force transcription of every entity, but doesn't yet.

  smtk::model::DescriptivePhrase::Ptr dit;
  smtk::model::EntityPhrase::Ptr ephr = smtk::model::EntityPhrase::create()->setup(model);
  smtk::model::SimpleModelSubphrases::Ptr spg = smtk::model::SimpleModelSubphrases::create();
  ephr->setDelegate(spg);
  prindent(std::cout, 0, ephr);

  // List model operators
  smtk::model::StringList opNames = model.operatorNames();
  if (!opNames.empty())
    {
    std::cout << "\nFound operators:\n";
    for (smtk::model::StringList::const_iterator it = opNames.begin(); it != opNames.end(); ++it)
      {
      std::cout << "  " << *it << "\n";
      }
    }

  // Test a model operator (if some argument beyond filename is given)
  if (argc > 2)
    {
    // Find a face with more than 2 triangles
    smtk::model::Faces allFaces;
    smtk::model::Cursor::CursorsFromUUIDs(
      allFaces, manager,
      manager->entitiesMatchingFlags(smtk::model::FACE));
    smtk::model::Face f;
    for (smtk::model::Faces::iterator it = allFaces.begin(); it != allFaces.end(); ++it)
      {
      f = *it;
      const smtk::model::Tessellation* tess = f.hasTessellation();
      if (tess && tess->conn().size() > 8)
        break;
      }
    if (f.isValid() && f.hasTessellation()->conn().size() > 8)
      {
      std::cout << "Attempting face split\n";
      smtk::model::OperatorPtr splitFace = model.op("split face");
      splitFace->specification()->findModelEntity("face to split")->setValue(f);
      splitFace->specification()->findModelEntity("model")->setValue(
          *manager->entitiesMatchingFlagsAs<ModelEntities>(smtk::model::MODEL_ENTITY).begin());
      splitFace->specification()->findDouble("feature angle")->setValue(15.0);
      OperatorResult result = splitFace->operate();
      std::cout << "  Face is " << f.name() << " (" << f.entity() << ")\n";
      std::cout << "  " << (result->findInt("outcome")->value() == OPERATION_SUCCEEDED ? "OK" : "Failed") << "\n";
      }
    else if (f.isValid())
      {
      std::cout << "No faces to split\n";
      }

    smtk::model::CursorArray exports;
    exports.push_back(model);
    bridge->ExportEntitiesToFileOfNameAndType(
      exports, "bridgeTest.cmb", "cmb");
    std::cout << "  done\n";
    }

  std::string json = smtk::io::ExportJSON::fromModel(manager);
  if (!json.empty())
    {
    std::ofstream jsonFile("bridgeTest.json");
    jsonFile << json;
    jsonFile.close();
    }

  return 0;
}
