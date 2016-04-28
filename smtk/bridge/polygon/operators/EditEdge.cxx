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

struct BoundingBox
{
  BoundingBox()
  {
  this->MinPnt[0] = this->MinPnt[1] = this->MinPnt[2] =
    std::numeric_limits<double>::max();
  this->MaxPnt[0] = this->MaxPnt[1] = this->MaxPnt[2] =
    std::numeric_limits<double>::min();
  }
 void addPoint(double x, double y, double z);
 void addPoint(double p[3]);
 void getBounds(double bounds[6]) const;
 void getBounds(double &xMin, double &xMax,
                                  double &yMin, double &yMax,
                                  double &zMin, double &zMax) const;

double MinPnt[3], MaxPnt[3];
};
inline void BoundingBox::addPoint(double x, double y, double z)
{
  double p[3];
  p[0] = x; p[1] = y; p[2] = z;
  this->addPoint(p);
}
inline void BoundingBox::addPoint(double p[3])
{
  for (int i = 0; i < 3; i++)
    {
    if (p[i] < this->MinPnt[i])
      {
      this->MinPnt[i] = p[i];
      }

    if (p[i] > this->MaxPnt[i])
      {
      this->MaxPnt[i] = p[i];
      }
    }
}
inline void BoundingBox::getBounds(double bounds[6]) const
{
  this->getBounds(bounds[0], bounds[1], bounds[2],
                  bounds[3], bounds[4], bounds[5]);
}
inline void BoundingBox::getBounds(double &xMin, double &xMax,
                                  double &yMin, double &yMax,
                                  double &zMin, double &zMax) const
{
  xMin = this->MinPnt[0];
  xMax = this->MaxPnt[0];
  yMin = this->MinPnt[1];
  yMax = this->MaxPnt[1];
  zMin = this->MinPnt[2];
  zMax = this->MaxPnt[2];
}

bool EditEdge::ableToOperate()
{
  bool able2Op = this->ensureSpecification();

  if(!able2Op)
    {
    return able2Op;
    }

  // for Destroy and Modify operation, we need edge is set
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("Operation");

  std::string optype = optypeItem->value();
  if(optype != "Create")
    {
    smtk::model::Model model;
    able2Op =
    // The SMTK model must be valid
      (model = this->specification()->associations()->value().as<smtk::model::Model>()).isValid()
    // The polygon model must exist
      && this->findStorage<internal::pmodel>(model.entity())
      ;
    }

  if(!able2Op)
    {
    return able2Op;
    }

  if(optype == "Remove")
    able2Op = this->specification()->findModelEntity("remove edge")
      ->value().isValid();
  else if(optype == "Edit")
    able2Op = this->specification()->findModelEntity("edit edge")
      ->value().isValid();;

  return able2Op;
}

bool internal_createModel(smtk::model::Operator::Ptr modOp,
  smtk::model::Model& model,
  smtk::attribute::DoubleItemPtr pointsItem,
  smtk::io::Logger& logger)
{
  smtk::attribute::IntItem::Ptr constructMethod = modOp->findInt("construction method");
  constructMethod->setDiscreteIndex(0); // "origin, 2 axes, and feature size"
  //modOp->findInt("model scale")->setValue(1);
  BoundingBox bx;
  std::size_t n = pointsItem->numberOfValues();
  for(size_t i = 0; i < n; i=i+3)
    {
    bx.addPoint(pointsItem->value(i),
                pointsItem->value(i+1),
                pointsItem->value(i+2));
    }
  double bds[6];
  bx.getBounds(bds);
  double diam = 0.0;
  for (int i = 0; i < 3; ++i)
    {
    diam += (bds[2*i + 1] - bds[2*i]) * (bds[2*i + 1] - bds[2*i]);
    }
  diam = sqrt(diam);
  //std::cout << "diam " << diam << "\n";

  // Use the lower-left-front bounds as the origin of the plane.
  // This keeps the projected integer coordinates small when the dataset is not
  // well-centered about the origin and makes overflow less likely.
  // However, it does mean that multiple imported polygon models in the same
  // plane will not share coordinates exactly.
  for (int i = 0; i < 3; ++i)
    {
    modOp->findDouble("origin")->setValue(i, bds[2 * i]);
    }
  // Infer a feature size from the bounds:
  modOp->findDouble("feature size")->setValue(diam / 1000.0);
  OperatorResult modResult = modOp->operate();
  if (modResult->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    smtkInfoMacro(logger, "CreateModel operator failed.");
    return false;
    }
  model = modResult->findModelEntity("created")->value();
  return model.isValid();
}

int internal_createEdge(smtk::model::Operator::Ptr edgeOp,
  smtk::attribute::AttributePtr opSpec,
  smtk::model::EntityRefArray& createdEds,
  const smtk::model::Model& model,
  smtk::io::Logger& logger)
{
  smtk::attribute::AttributePtr spec = edgeOp->specification();
  spec->associateEntity(model);
  smtk::attribute::IntItem::Ptr constructMethod = spec->findInt("construction method");
  constructMethod->setDiscreteIndex(0); // "points coornidates"
  smtk::attribute::IntItem::Ptr numCoords = spec->findInt("coordinates");
  numCoords->setValue(3); // number of elements in coordinates
  smtk::attribute::DoubleItem::Ptr pointsItem = spec->findDouble("points");
  smtk::attribute::ConstItemPtr sourceItem = opSpec->find("edge points", smtk::attribute::ALL_CHILDREN);
  pointsItem->assign(sourceItem);
  smtk::attribute::IntItem::Ptr offsetsItem = spec->findInt("offsets");
  sourceItem = opSpec->find("edge offsets", smtk::attribute::ALL_CHILDREN);
  offsetsItem->assign(sourceItem);
  smtk::attribute::ConstStringItemPtr nameItem =
    smtk::dynamic_pointer_cast<const smtk::attribute::StringItem>(
    opSpec->find("edge name", smtk::attribute::ALL_CHILDREN));
  std::string edgeName = nameItem->value();

  OperatorResult edgeResult = edgeOp->operate();
  if (edgeResult->findInt("outcome")->value() != OPERATION_SUCCEEDED)
    {
    smtkDebugMacro(logger, "\"create edge\" op failed to creat edge with given line cells.");
    return 0;
    }
  smtk::attribute::ModelEntityItem::Ptr newEdges = edgeResult->findModelEntity("created");
  createdEds.insert(createdEds.end(), newEdges->begin(), newEdges->end());
  smtk::model::EntityRefArray::iterator it;
  int i=0;
  for(it = createdEds.begin(); i < createdEds.size() && it != createdEds.end(); ++it, ++i)
    {
    std::ostringstream nss(edgeName);
    // if more than one edge, append a number
    if(i > 0)
      nss << i;     
    it->setName(nss.str());
    }
  return createdEds.size();

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
  smtk::model::EntityRefs remEdges;
  bool newModel = false;
  // Based on operation type specified, start different operations
  smtk::attribute::StringItem::Ptr optypeItem =
    this->specification()->findString("Operation");
  std::string optype = optypeItem->value();
  if(optype == "Create") //only need model
    {
    if(!model.isValid())
      {
      smtk::model::OperatorPtr modOp = opsession->op("create model");
      if(!modOp)
        {
        smtkInfoMacro(log(), "Failed to create CreateModel op.");
        return this->createResult(OPERATION_FAILED);
        }

      // we will need to figure out the bounds of the model with given points
      smtk::attribute::DoubleItem::Ptr pointsItem =
        this->specification()->findAs<smtk::attribute::DoubleItem>(
        "edge points", smtk::attribute::ALL_CHILDREN);
      if(!internal_createModel(modOp, model, pointsItem, log()))
        {
        smtkInfoMacro(log(), "CreateModel operator failed to create a new model.");
        return this->createResult(OPERATION_FAILED);
        }
      newModel = true;
      }
    smtk::model::Operator::Ptr edgeOp = opsession->op("create edge");
    if(!edgeOp)
      {
      smtkInfoMacro(log(), "Failed to create CreateEdge op.");
      return this->createResult(OPERATION_FAILED);
      }
    int numEdges = internal_createEdge(
      edgeOp, this->specification(), newEdges, model, log());
    ok = numEdges > 0;
    }
  else if(optype == "Remove")
    {
    smtk::attribute::ModelEntityItemPtr remEdgeItem =
      this->specification()->findModelEntity("remove edge");
    }
  else if(optype == "Edit")
    {
    smtk::attribute::ModelEntityItemPtr editEdgeItem =
      this->specification()->findModelEntity("edit edge");
    }

  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    // Return the created or modified group.
    if(optype == "Create")
      {
      if(newModel)
        {
        this->addEntityToResult(result, model, CREATED);
        }
      else
        {
        this->addEntitiesToResult(result, newEdges, CREATED);
        this->addEntityToResult(result, model, MODIFIED);
        }
      }
    if(modEdges.size() > 0)
      this->addEntitiesToResult(result, modEdges, MODIFIED);

    if(optype == "Remove" && remEdges.size() > 0)
      {
      // Return the created or modified group.
      smtk::attribute::ModelEntityItem::Ptr remEntities =
        result->findModelEntity("expunged");
      remEntities->setNumberOfValues(remEdges.size());
      remEntities->setIsEnabled(true);
      smtk::model::EntityRefs::const_iterator it;
      int rid = 0;
      for (it=remEdges.begin(); it != remEdges.end(); it++)
        remEntities->setValue(rid++, *it);
      }

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
