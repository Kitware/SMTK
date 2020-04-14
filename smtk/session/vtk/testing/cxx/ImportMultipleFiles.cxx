//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/model/Model.h"

#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/operators/Import.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <string>

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
}

int ImportMultipleFiles(int /*argc*/, char* /*argv*/ [])
{
  std::vector<std::string> files({ "/model/3d/exodus/disk_out_ref.ex2", "/model/3d/exodus/knee.ex2",
    "/model/3d/exodus/sx_bar_hex.exo" });

  // Construct a new resource and session
  smtk::session::vtk::Resource::Ptr resource = smtk::session::vtk::Resource::create();
  smtk::session::vtk::Session::Ptr session = smtk::session::vtk::Session::create();
  resource->setSession(session);

  // Create an import operator
  smtk::session::vtk::Import::Ptr importOp = smtk::session::vtk::Import::create();
  if (!importOp)
  {
    std::cerr << "No import operator\n";
    return 1;
  }

  // Associate with the resource into which to import
  importOp->parameters()->associate(resource);

  // Set the file paths
  auto filenameItem = importOp->parameters()->findFile("filename");
  filenameItem->setNumberOfValues(files.size());
  for (std::size_t i = 0; i < files.size(); ++i)
  {
    filenameItem->setValue(i, SMTK_DATA_DIR + files[i]);
  }

  // Execute the operation
  smtk::operation::Operation::Result importOpResult = importOp->operate();

  // Retrieve the resulting models
  smtk::model::Models models =
    resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

  // Check that all models were successfully read
  test(models.size() == files.size());

  return 0;
}
