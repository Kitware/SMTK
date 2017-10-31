//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/resource/Metadata.h"

namespace smtk
{
namespace resource
{
namespace testing
{

ResourceArray loadTestResources(ManagerPtr rsrcMgr, int argc, char* argv[])
{
  smtk::model::Manager::Metadata modelResourceMetadata("model");
  modelResourceMetadata.create = [](
    const smtk::common::UUID&) { return smtk::model::Manager::create(); };
  rsrcMgr->registerResource<smtk::model::Manager>(modelResourceMetadata);

  ResourceArray result;
  if (argc < 2)
  {
    std::cerr << "No files to load.\n";
    return result;
  }

  auto modelMgr = smtk::model::Manager::create();
  auto nativeSess = modelMgr->createSession("native");
  auto loadOp = nativeSess.op("load smtk model");
  auto fname = loadOp->findFile("filename");
  fname->setValue(0, argv[1]);
  auto loadRes = loadOp->operate();
  modelMgr->setLocation(argv[1]);
  result.push_back(dynamic_pointer_cast<Resource>(modelMgr));

  return result;
}
}
}
}
