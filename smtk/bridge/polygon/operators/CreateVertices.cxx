#include "smtk/bridge/polygon/operators/CreateVertices.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateVertices_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult CreateVertices::operateInternal()
{
  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();

  smtk::bridge::polygon::Session* sess = this->polygonSession();
  smtk::model::OperatorResult result;
  if (sess)
    {
    smtk::model::Manager::Ptr mgr = sess->manager();
    internal::pmodel::Ptr storage =
      sess->findStorage<internal::pmodel>(
        modelItem->value(0).entity());
    std::vector<double> pcoords(pointsItem->begin(), pointsItem->end());
    smtk::model::Vertices verts =
      storage->findOrAddModelVertices(
        mgr, pcoords, coordinatesItem->value(0));
    result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(result, verts, CREATED);
    }
  /*
  if (ok)
    {
    smtk::bridge::polygon::Session* sess = this->polygonSession();
    smtk::model::Manager::Ptr mgr;
    if (sess)
      {
      mgr = sess->manager();
      smtk::model::Model model = mgr->addModel(/ * par. dim. * / 2, / * emb. dim. * / 3, "model");
      storage->setId(model.entity());
      result = this->createResult(smtk::model::OPERATION_SUCCEEDED);
      this->addEntityToResult(result, model, CREATED);
      model.setFloatProperty("x axis", smtk::model::FloatList(storage->xAxis(), storage->xAxis() + 3));
      model.setFloatProperty("y axis", smtk::model::FloatList(storage->yAxis(), storage->yAxis() + 3));
      model.setFloatProperty("normal", smtk::model::FloatList(storage->zAxis(), storage->zAxis() + 3));
      model.setFloatProperty("origin", smtk::model::FloatList(storage->origin(), storage->origin() + 3));
      model.setFloatProperty("feature size", storage->featureSize());
      model.setIntegerProperty("model scale", storage->modelScale());
      }
    }
  */
  if (!result)
    {
    result = this->createResult(smtk::model::OPERATION_FAILED);
    }

  return result;
}

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::CreateVertices,
  polygon_create_vertices,
  "create vertices",
  CreateVertices_xml,
  smtk::bridge::polygon::Session);
