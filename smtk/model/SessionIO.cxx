//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionIO.h"

#include "smtk/common/ResourceSet.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/StoredResource.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem; // for path

namespace smtk
{
namespace model
{

int SessionIO::saveResource(
  const EntityRef& ent, smtk::common::ResourceSetPtr rset, StoredResourcePtr rsrc)
{
  if (!rsrc || !ent.isValid())
  {
    return 0;
  }

  if (ent.isAuxiliaryGeometry())
  {
    // We will need helpers to either
    // (a) determine where the original is and where to place copy or
    // (b) write in-memory rep to file.
    // For now, since auxiliary geometry is never modified,
    // we will do nothing and return (and handle copying elsewhere).
    return 1;
  }

  SessionRef sref = ent.owningSession();
  Operator::Ptr exportOp = sref.op("export");
  if (!exportOp)
  {
    exportOp = sref.op("save");
  }
  if (!exportOp)
  {
    exportOp = sref.op("write");
  }
  if (!exportOp)
  {
    return 0;
  }
  smtk::attribute::ModelEntityItem::Ptr assocItem = exportOp->specification()->associations();
  const EntityRefs& ents(rsrc->entities());
  if (!assocItem || !assocItem->setNumberOfValues(ents.size()))
  {
    return 0;
  }
  int ii = 0;
  for (auto rent : ents)
  {
    assocItem->setValue(ii++, rent);
  }
  smtk::attribute::FileItem::Ptr filenameItem = exportOp->findFile("filename");
  if (filenameItem)
  {
    path fname(rsrc->url());
    if (!fname.is_absolute() && !rset->linkStartPath().empty())
    {
      fname = path(rset->linkStartPath()) / fname;
    }
    filenameItem->setValue(fname.string());
    OperatorResult result = exportOp->operate();
    if (result && result->findInt("outcome")->value(0) == OPERATION_SUCCEEDED)
    {
      return 1;
    }
  }
  return 0;
}

} // namespace model
} // namespace smtk
