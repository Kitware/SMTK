#include "smtk/bridge/polygon/operators/CreateFaces.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateFaces_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult CreateFaces::operateInternal()
{
  // Discover how the user wants to specify scaling.
  smtk::attribute::IntItem::Ptr constructionMethodItem = this->findInt("construction method");
  int method = constructionMethodItem->discreteIndex(0);

  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  smtk::attribute::IntItem::Ptr offsetsItem = this->findInt("offsets");

  smtk::attribute::ModelEntityItem::Ptr edgesItem = this->findModelEntity("edges");

  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();

  internal::pmodel::Ptr storage; // Look up from session = internal::pmodel::create();
  bool ok = true;
  // These case values match CreateFaces.sbt indices (and enum values):
  switch (method)
    {
  case 0: // points, coordinates, offsets
      {
      // create holes (as specified by offsets)
      // create polygon_with_holes
      //   make sure to *not* generate keyhole edges
      // traverse resulting polygon
      //   identify pre-existing edges and split them as required?
      }
    break;
  case 1: // edges, offsets
      {
      // create holes
      // create polygon_with_holes
      //   make sure to *not* generate keyhole edges
      // traverse resulting polygon
      //   identify pre-existing edges and split them as required?
      }
    break;
  case 2: // all non-overlapping
      {
      // Create a union-find struct
      // for each "model" vertex
      //   for each edge attached to each vertex
      //     add 2 union-find entries (UFEs), 1 per co-edge
      //     merge adjacent pairs of UFEs
      //     store UFEs on edges
      // For each loop, discover nestings and merge UFEs
      // For each edge
      //   For each unprocessed (nesting-wise) UFE
      //     Discover nesting via ray test
      //     Merge parent and child UFEs (if applicable)
      //     Add an (edge, coedge sign) tuple to a "face" identified by the given UFE
      // FIXME: Test for self-intersections?
      // FIXME: Deal w/ pre-existing faces?
      }
    break;
  default:
    ok = false;
    smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
    break;
    }

  smtk::model::OperatorResult result;
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
  smtk::bridge::polygon::CreateFaces,
  polygon_create_faces,
  "create faces",
  CreateFaces_xml,
  smtk::bridge::polygon::Session);
