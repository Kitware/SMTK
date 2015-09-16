#include "smtk/bridge/polygon/operators/CreateModel.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/CreateModel_xml.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

smtk::model::OperatorResult CreateModel::operateInternal()
{
  // Discover how the user wants to specify scaling.
  smtk::attribute::IntItem::Ptr constructionMethodItem = this->findInt("construction method");
  int method = constructionMethodItem->discreteIndex(0);

  smtk::attribute::DoubleItem::Ptr originItem = this->findDouble("origin");
  smtk::attribute::DoubleItem::Ptr xAxisItem = this->findDouble("x axis");
  smtk::attribute::DoubleItem::Ptr yAxisItem = this->findDouble("y axis");
  smtk::attribute::DoubleItem::Ptr zAxisItem = this->findDouble("z axis");
  smtk::attribute::DoubleItem::Ptr featureSizeItem = this->findDouble("feature size");
  smtk::attribute::IntItem::Ptr modelScaleItem = this->findInt("model scale");

  internal::model::Ptr storage = internal::model::create();
  bool ok = true;
  // These case values match CreateModel.sbt indices (and enum values):
  switch (method)
    {
  case 0: // origin, 2 axes, and feature size
      {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> y_axis(yAxisItem->begin(), yAxisItem->end());
      ok = storage->computeModelScaleAndNormal(
        origin, x_axis, y_axis, featureSizeItem->value(0), this->log());
      }
    break;
  case 1: // origin, normal, x axis, and feature size
      {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> z_axis(zAxisItem->begin(), zAxisItem->end());
      ok = storage->computeModelScaleAndYAxis(
        origin, x_axis, z_axis, featureSizeItem->value(0), this->log());
      }
    break;
  case 2: // origin, 2 axes, and model scale
      {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> y_axis(yAxisItem->begin(), yAxisItem->end());
      ok = storage->computeFeatureSizeAndNormal(
        origin, x_axis, y_axis, modelScaleItem->value(0), this->log());
      }
    break;
  default:
    ok = false;
    smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
    break;
    }

  smtk::model::OperatorResult result;
  if (ok)
    {
    smtk::bridge::polygon::Session* sess = this->polygonSession();
    smtk::model::Manager::Ptr mgr;
    if (sess)
      {
      mgr = sess->manager();
      smtk::model::Model model = mgr->addModel(/* par. dim. */ 2, /* emb. dim. */ 3, "model");
      storage->setId(model.entity());
      sess->addStorage(model.entity(), storage);
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
  smtk::bridge::polygon::CreateModel,
  polygon_create_model,
  "create model",
  CreateModel_xml,
  smtk::bridge::polygon::Session);
