//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "EditEdge.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

#include "smtk/bridge/polygon/EditEdge_xml.h"

#include "smtk/io/ExportJSON.h"
#include <sstream>
#include "cJSON.h"
#include <limits>

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace polygon {

bool EditEdge::ableToOperate()
{
  bool able2Op = this->ensureSpecification();

  if(!able2Op)
    {
    return able2Op;
    }

  smtk::model::Model model;
  able2Op =
  // The SMTK model must be valid
    (model = this->specification()->associations()->value().as<smtk::model::Model>()).isValid()
  // The polygon model must exist
    && this->findStorage<internal::pmodel>(model.entity())
    ;

  if(!able2Op)
    {
    return able2Op;
    }

  able2Op = this->specification()->findModelEntity("edge")
    ->value().isValid();;

  return able2Op;
}

OperatorResult EditEdge::operateInternal()
{
  Session* opsession = this->polygonSession();
  // ableToOperate should have verified that model is valid
  smtk::model::Model model = this->specification()->associations()
    ->value().as<smtk::model::Model>();
  bool ok = false;
  smtk::model::EntityRefArray newEdges;
  smtk::model::EntityRefArray modEdges;
  smtk::model::EntityRefArray expungedEnts;

  // TODO:
    // 1. Replace the existing edge points with the new edge-points.
    // 2. Check validity of new edge-points among themselves and against existing edges,
    //    create new model vertices if necessary, such as when the new edge-points
    //    intersect among themselves or exisitng edges 
    //
    // 3. Copy all properties and attributes of existing edge to the new edges if any generated
    // 4. Make sure all the edges still have valid topological relationships
  smtk::attribute::ModelEntityItemPtr editEdgeItem =
    this->specification()->findModelEntity("edge");
  smtk::attribute::DoubleItem::Ptr pointsItem =
    this->specification()->findAs<smtk::attribute::DoubleItem>(
    "points", smtk::attribute::ALL_CHILDREN);

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    if(newEdges.size() > 0)
      this->addEntitiesToResult(result, newEdges, CREATED);
    if(expungedEnts.size() > 0)
      result->findModelEntity("expunged")->setValues(expungedEnts.begin(), expungedEnts.end());
    if(modEdges.size() > 0)
      this->addEntitiesToResult(result, modEdges, MODIFIED);

    }

  return result;
}

    } // namespace polygon
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::EditEdge,
  polygon_edit_edge,
  "edit edge",
  EditEdge_xml,
  smtk::bridge::polygon::Session);
