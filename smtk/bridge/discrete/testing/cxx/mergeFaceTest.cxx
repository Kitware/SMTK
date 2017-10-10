//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/bridge/discrete/Session.h"
#include "smtk/common/UUID.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/model/Face.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Volume.h"

using namespace smtk::model;
using namespace smtk::io;

int main(int argc, char* argv[])
{
  if (argc > 1)
  {
    std::ifstream file;
    file.open(argv[1]);
    if (!file.good())
    {
      std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
      return 1;
    }

    int status = 1;
    ManagerPtr mgr = Manager::create();
    smtk::bridge::discrete::Session::Ptr brg = smtk::bridge::discrete::Session::create();
    mgr->registerSession(brg);
    Operator::Ptr op;
    OperatorResult result;

    op = brg->op("read");
    op->findFile("filename")->setValue(argv[1]);
    result = op->operate();
    if (result->findInt("outcome")->value() != smtk::operation::Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "Import cmb Failed: " << argv[1] << std::endl;
      return 1;
    }
    Model modelCmb = result->findModelEntity("created")->value();

    int noVolumes = 0;
    int noFaces = 0;
    if (modelCmb.isValid())
    {
      Faces faces;
      for (const auto& cell : modelCmb.cells())
      {
        if (isVolume(cell.entityFlags()))
        {
          Volume vol = static_cast<Volume>(cell);
          noVolumes++;
          faces = vol.faces();
          noFaces += static_cast<int>(faces.size());
        }
      }
      std::cout << "Number of volumes in the model: " << noVolumes << std::endl;
      std::cout << "Number of faces in the model: " << noFaces << std::endl;
      for (decltype(faces.size()) i = 0; i < faces.size(); ++i)
      {
        smtk::common::UUID id(faces[i].entity());
        std::cout << "UUID of Face " << i << ": " << id << std::endl;
      }
      test(noVolumes == 1, "Expecting 1 volume.");
      test(noFaces == 6, "Expecting 6 faces.");
      // Merge faces 3, 4, 5, 6
      op = brg->op("merge face");
      test(op != nullptr, "No merge face operator.");
      auto modelPtr = op->specification()->findModelEntity("model");
      test(modelPtr != nullptr && modelPtr->setValue(modelCmb), "Could not associate model");
      modelPtr = op->specification()->findModelEntity("source cell");
      test(modelPtr != nullptr && modelPtr->appendValue(faces[2]), "Could not set source cell");
      test(modelPtr != nullptr && modelPtr->appendValue(faces[3]), "Could not set source cell");
      test(modelPtr != nullptr && modelPtr->appendValue(faces[4]), "Could not set source cell");
      modelPtr = op->specification()->findModelEntity("target cell");
      test(modelPtr != nullptr && modelPtr->appendValue(faces[5]), "Could not set target cell");
      result = op->operate();
      test(result->findInt("outcome")->value() == smtk::operation::Operator::OPERATION_SUCCEEDED,
        "Merge face failed");
      // Check the number of faces after merge face operation
      test(modelCmb.cells().size() == 1 && isVolume(modelCmb.cells()[0].entityFlags()),
        "Expecting 1 volume");
      Volume v = static_cast<Volume>(modelCmb.cells()[0]);
      noFaces = static_cast<int>(v.faces().size());
      std::cout << "Number of faces in the model after merge face operation: " << noFaces
                << std::endl;
      test(noFaces == 3, "Expecting 3 faces after merge face operation.");
      status = 0;
    }
    return status;
  }
  return 0;
}
