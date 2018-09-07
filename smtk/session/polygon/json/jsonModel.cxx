//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/json/jsonModel.h"

#include "smtk/session/polygon/internal/Model.h"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace internal
{
using json = nlohmann::json;
// Define how polygon resources are serialized.
void to_json(json& j, const smtk::session::polygon::internal::pmodel::Ptr& pmodel)
{
  j["type"] = "model";
  j["origin"] = std::vector<double>(pmodel->origin(), pmodel->origin() + 3);
  j["x axis"] = std::vector<double>(pmodel->xAxis(), pmodel->xAxis() + 3);
  j["y axis"] = std::vector<double>(pmodel->yAxis(), pmodel->yAxis() + 3);
  j["z axis"] = std::vector<double>(pmodel->zAxis(), pmodel->zAxis() + 3);
  j["i axis"] = std::vector<double>(pmodel->iAxis(), pmodel->iAxis() + 3);
  j["j axis"] = std::vector<double>(pmodel->jAxis(), pmodel->jAxis() + 3);
  j["feature size"] = pmodel->featureSize();

  // Encode model scale carefully since cJSON cannot store large integers faithfully:
  std::vector<int> modelScaleBytes(8);
  long long mscale = static_cast<long long>(pmodel->modelScale());
  for (int i = 0; i < 8; ++i)
  {
    modelScaleBytes[7 - i] = mscale & 0xff;
    mscale >>= 8;
  }

  j["model scale"] = modelScaleBytes;
}

void from_json(const json& j, smtk::session::polygon::internal::pmodel::Ptr& pmodel)
{
  try
  {
    std::vector<double> origin = j.at("origin");
    std::vector<double> xAxis = j.at("x axis");
    std::vector<double> yAxis = j.at("y axis");
    std::vector<double> zAxis = j.at("z axis");
    std::vector<double> iAxis = j.at("i axis");
    std::vector<double> jAxis = j.at("j axis");
    double featureSize = j["feature size"];

    std::vector<int> modelScaleBytes = j["model scale"];
    long long modelScale = 0;
    for (int i = 0; i < 8; ++i)
    {
      modelScale += (modelScaleBytes[7 - i] << (8 * i));
    }

    pmodel = smtk::session::polygon::internal::pmodel::create();
    pmodel->restoreModel(origin, xAxis, yAxis, zAxis, iAxis, jAxis, featureSize, modelScale);
  }
  catch (std::exception&)
  {
    std::cerr << "Failed to deserialize polygon model" << std::endl;
  }
}
} // namespace internal
} // namespace polygon
} // namespace session
} // namespace smtk
