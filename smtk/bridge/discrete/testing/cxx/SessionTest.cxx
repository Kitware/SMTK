//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SimpleModelSubphrases.h"
#include "smtk/model/Tessellation.h"

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

  std::cout << "Available sessions\n";
  StringList sessions = manager->sessionTypeNames();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::bridge::discrete::Session::Ptr session = smtk::bridge::discrete::Session::create();
  manager->registerSession(session);

  std::cout << "Available cmb operators\n";
  StringList opnames = session->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  smtk::model::OperatorPtr readOp = session->op("read");
  if (!readOp)
  {
    std::cerr << "No read operator\n";
    return 1;
  }

  readOp->specification()->findFile("filename")->setValue(std::string(argv[1]));
  std::cout << "Importing " << argv[1] << "\n";
  smtk::model::OperatorResult opresult = readOp->operate();
  if (opresult->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
  {
    std::cerr << "Read operator failed\n";
    return 1;
  }

  smtk::model::Model model = opresult->findModelEntity("created")->value();
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
    smtk::model::EntityRef::EntityRefsFromUUIDs(
      allFaces, manager, manager->entitiesMatchingFlags(smtk::model::FACE));
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
      smtk::model::OperatorPtr splitFace = session->op("split face");
      auto faceToSplit = splitFace->specification()->findModelEntity("face to split");
      faceToSplit->setNumberOfValues(1);
      faceToSplit->setValue(f);
      splitFace->specification()->findModelEntity("model")->setValue(
        *manager->entitiesMatchingFlagsAs<Models>(smtk::model::MODEL_ENTITY).begin());
      splitFace->specification()->findDouble("feature angle")->setValue(15.0);
      OperatorResult result = splitFace->operate();
      std::cout << "  Face is " << f.name() << " (" << f.entity() << ")\n";
      std::cout << "  " << (result->findInt("outcome")->value() ==
                                 smtk::operation::Operator::OPERATION_SUCCEEDED
                               ? "OK"
                               : "Failed")
                << "\n";
    }
    else if (f.isValid())
    {
      std::cout << "No faces to split\n";
    }

    smtk::model::EntityRefArray exports;
    exports.push_back(model);
    session->ExportEntitiesToFileOfNameAndType(exports, "sessionTest.cmb", "cmb");
    std::cout << "  done\n";
  }

  std::string json = smtk::io::SaveJSON::fromModelManager(manager);
  if (!json.empty())
  {
    std::ofstream jsonFile("sessionTest.json");
    jsonFile << json;
    jsonFile.close();
  }

  return 0;
}
