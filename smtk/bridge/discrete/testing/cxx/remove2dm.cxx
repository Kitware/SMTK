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
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

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

    op = brg->op("import");
    op->findFile("filename")->setValue(argv[1]);
    result = op->operate();
    if (result->findInt("outcome")->value() != Operator::OPERATION_SUCCEEDED)
    {
      std::cerr << "Import 2dm Failed: " << argv[1] << std::endl;
      return 1;
    }
    Model model2dm = result->findModelEntity("created")->value();

    if (model2dm.isValid())
    {
      // Remove model
      op = brg->op("remove model");
      test(op != nullptr, "No remove model operator.");
      test(op->specification()->associateEntity(model2dm), "Could not associate model.");
      result = op->operate();
      test(result->findInt("outcome")->value() == Operator::OPERATION_SUCCEEDED,
        "close model failed.");
      // Make sure the model is removed
      SessionRef session(mgr, brg->sessionId());
      test(session.models<Models>().empty(), "Expecting no models after close.");
      status = 0;
    }

    return status;
  }

  return 0;
}
