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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/core/PointField.h"
#include "smtk/mesh/utility/Create.h"
#include "smtk/mesh/utility/Metrics.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/utility/InterpolateField.h"

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/resource/Component.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <array>

namespace
{
// SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

// A test attribute description that allows association and has a double item.
// clang-format off
const char* testInput = R"(
<?xml version="1.0"?>
<SMTK_AttributeResource Version="4">
  <Definitions>
    <AttDef Type="Example">
      <AssociationsDef>
        <Accepts><Resource Name="smtk::model::Resource"/></Accepts>
      </AssociationsDef>
      <ItemDefinitions>
        <Double Name="interpolation value"/>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Name="att1" Type="Example"/>
    <Att Name="att2" Type="Example"/>
    <Att Name="att3" Type="Example"/>
    <Att Name="att4" Type="Example"/>
  </Attributes>
</SMTK_AttributeResource>)";
  // clang-format on

  // A testing structure to perform regression testing against the results of
  // interpolation.
  class HistogramFieldData
{
public:
  HistogramFieldData(std::size_t nBins, double min, double max)
    : m_min(min)
    , m_max(max)
  {
    m_hist.resize(nBins, 0);
  }

  const std::vector<std::size_t>& histogram() const { return m_hist; }

protected:
  std::vector<std::size_t> m_hist;
  double m_min;
  double m_max;
};

class HistogramPointFieldData : public smtk::mesh::PointForEach, public HistogramFieldData
{
public:
  HistogramPointFieldData(std::size_t nBins, double min, double max, smtk::mesh::PointField& pf)
    : HistogramFieldData(nBins, min, max)
    , m_pointField(pf)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& /*xyz*/,
    bool& /*coordinatesModified*/) override
  {
    std::vector<double> values(pointIds.size());
    m_pointField.get(pointIds, &values[0]);
    for (auto& value : values)
    {
      std::size_t bin = static_cast<std::size_t>((value - m_min) / (m_max - m_min) * m_hist.size());

      ++m_hist[bin];
    }
  }

protected:
  smtk::mesh::PointField& m_pointField;
};
}

int TestInterpolateAnnotatedValues(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Import a mesh session resource.
  smtk::session::mesh::Resource::Ptr resource;
  {
    auto importOp = smtk::session::mesh::Import::create();

    if (!importOp)
    {
      std::cerr << "No import operator\n";
      return 1;
    }

    std::string importFilePath(data_root);
    importFilePath += "/mesh/2d/test2D.2dm";

    importOp->parameters()->findFile("filename")->setValue(importFilePath);
    importOp->parameters()->findVoid("construct hierarchy")->setIsEnabled(false);

    smtk::operation::Operation::Result importOpResult = importOp->operate();

    if (importOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }

    // Retrieve the resulting resource
    smtk::attribute::ResourceItemPtr resourceItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
        importOpResult->findResource("resource"));

    // Access the generated resource
    resource = std::dynamic_pointer_cast<smtk::session::mesh::Resource>(resourceItem->value());

    // Retrieve the resulting model
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        importOpResult->findComponent("model"));

    // Access the generated model
    smtk::model::Entity::Ptr modelEntity =
      std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

    smtk::model::Model model = modelEntity->referenceAs<smtk::model::Model>();

    if (!model.isValid())
    {
      std::cerr << "Import operator returned an invalid model\n";
      return 1;
    }
  }

  // Access all of the model's faces
  smtk::model::EntityRefs currentEnts =
    resource->entitiesMatchingFlagsAs<smtk::model::EntityRefs>(smtk::model::FACE);
  if (currentEnts.empty())
  {
    std::cerr << "No faces!" << std::endl;
    return 1;
  }

  // Arrange the model faces with a consistent ordering
  std::vector<smtk::model::EntityRef> eRefs(currentEnts.begin(), currentEnts.end());
  std::sort(eRefs.begin(), eRefs.end(),
    [](const smtk::model::EntityRef& lhs, const smtk::model::EntityRef& rhs) {
      return lhs.name() < rhs.name();
    });

  // Construct the test attribute resource
  smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
  smtk::attribute::Resource& attResource(*resptr.get());
  smtk::io::Logger logger;
  smtk::io::AttributeReader reader;

  if (reader.readContents(resptr, testInput, logger))
  {
    std::cerr << "Encountered Errors while reading input data\n";
    std::cerr << logger.convertToString();
    return 2;
  }

  // Access the attribute resource's definitions
  std::vector<smtk::attribute::DefinitionPtr> defs;
  attResource.definitions(defs);
  if (defs.size() != 1)
  {
    std::cerr << "Unexpected number of definitions: " << defs.size() << "\n";
    std::cerr << logger.convertToString();
    return 2;
  }

  // Access the attribute resource's attributes
  std::vector<smtk::attribute::AttributePtr> atts;
  attResource.attributes(atts);
  if (atts.size() != 4)
  {
    std::cerr << "Unexpected number of attributes: " << atts.size() << "\n";
    std::cerr << logger.convertToString();
    return 2;
  }

  // For each attribute, associate the attribute to a model face and assign it
  // a value
  for (std::size_t i = 0; i < 4; i++)
  {
    atts[i]->associate(eRefs[i].component());
    atts[i]->findDouble("interpolation value")->setValue(i + 1.);
  }

  // Construct a grid of sample points
  std::array<double, 6> extent = smtk::mesh::utility::extent(resource->resource()->meshes());
  std::array<double, 3> origin = { extent[0], extent[2], extent[4] };
  std::array<double, 3> size = { (extent[1] - extent[0]), (extent[3] - extent[2]),
    (extent[5] - extent[4]) };

  // Offset the grid from the model surface
  origin[2] += .25;

  smtk::mesh::ResourcePtr gridResource = smtk::mesh::Resource::create();

  // Construct a mapping from a unit box to the input box
  std::function<std::array<double, 3>(std::array<double, 3>)> fn = [&](std::array<double, 3> x) {
    return std::array<double, 3>(
      { { origin[0] + size[0] * x[0], origin[1] + size[1] * x[1], origin[2] + size[2] * x[2] } });
  };

  std::array<std::size_t, 2> disc = { { 50, 20 } };

  std::array<smtk::mesh::MeshSet, 5> sampleGrid =
    smtk::mesh::utility::createUniformGrid(gridResource, disc, fn);

  // Remove superfluous meshes (we are only interested in the surface mesh, not
  // its edges)
  for (std::size_t i = 1; i < 5; i++)
  {
    gridResource->removeMeshes(sampleGrid[i]);
  }

  // Access the points of the sample grid
  std::vector<double> pts;
  sampleGrid[0].points().get(pts);

  // Compute the interpolation weights for the sample grid points
  std::vector<smtk::model::Weights> weights = smtk::model::computeWeights(pts, defs[0]);

  // Compute the interpolated values for the sample grid points
  std::vector<double> values =
    smtk::model::inverseDistanceWeighting(weights, "interpolation value", 2);

  // For testing, assign the interpolated values to the sample grid as a point
  // field
  sampleGrid[0].createPointField("interpolation", 1, smtk::mesh::FieldType::Double, &values[0]);

  bool debug = false;
  if (debug)
  {
    //write out the mesh.
    std::string write_path(write_root);
    write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

    smtk::io::WriteMesh write;
    bool result = write(write_path, gridResource);
    if (!result)
    {
      test(result, "failed to properly write out a valid hdf5 resource");
    }
  }

  // Histogram the resulting points and compare against expected values.
  std::vector<std::size_t> histogram;
  smtk::mesh::PointField pointField = gridResource->meshes().pointField("interpolation");
  HistogramPointFieldData histogramPointFieldData(10, .5, 4.5, pointField);
  smtk::mesh::for_each(gridResource->meshes().points(), histogramPointFieldData);
  histogram = histogramPointFieldData.histogram();

  std::array<std::size_t, 10> expected = { { 0, 300, 88, 106, 69, 54, 113, 102, 239, 0 } };

  std::size_t counter = 0;
  for (auto& bin : histogram)
  {
    if (bin != expected[counter++])
    {
      std::cerr << "\"generate hotstart data\" operator produced unexpected results\n";
      return 1;
    }
  }

  return 0;
}