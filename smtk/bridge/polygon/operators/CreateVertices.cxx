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
