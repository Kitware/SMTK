//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/operators/InterpolateMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/mesh/CellField.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/PointField.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if defined(SMTK_CLANG) ||                                                                         \
  (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                 \
  defined(SMTK_MSVC)
#include <regex>
using std::regex;
using std::sregex_token_iterator;
using std::regex_replace;
using std::regex_search;
using std::regex_match;
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::sregex_token_iterator;
using boost::regex_replace;
using boost::regex_search;
using boost::regex_match;
#endif

namespace
{
// A key that corresponds to the .sbt file's values for output field type.
enum
{
  CELL_FIELD = 0,
  POINT_FIELD = 1
};

// We use inverse distance weighting via Shepard's method, implmented below.
typedef std::array<double, 3> Point;

static const double EPSILON = 1.e-10;

double euclideanDistance(const Point& p1, const Point& p2)
{
  Point diff = { { p1[0] - p2[0], p1[1] - p2[1], p1[2] - p2[2] } };
  return std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
}

class ShepardInterpolator
{
public:
  // Return the interpolated value at <p> as a weighted sum of the sources
  double operator()(const Point& p) const
  {
    double d = 0., w = 0., num = 0., denom = 0.;
    for (auto& source : this->m_sources)
    {
      d = euclideanDistance(p, source.first);
      // If d is zero, then return the value associated with the source point.
      if (d < EPSILON)
      {
        return source.second;
      }
      // Otherwise, sum the contribution from each point.
      w = std::pow(d, -1. * this->m_power);
      num += w * source.second;
      denom += w;
    }

    return num / denom;
  }

  double operator()(double x, double y, double z) const
  {
    const Point p = { { x, y, z } };
    return this->operator()(p);
  }

  void setPower(double power) { this->m_power = power; }
  void addSourcePoint(const Point& p, double value)
  {
    this->m_sources.push_back(std::make_pair(p, value));
  }

  bool readPointsFromCSV(const std::string& fileName)
  {
    std::ifstream infile(fileName.c_str());
    if (!infile.good())
    {
      return false;
    }
    std::string line;
    regex re(",");
    Point p;
    double value;
    while (std::getline(infile, line))
    {
      // passing -1 as the submatch index parameter performs splitting
      sregex_token_iterator first{ line.begin(), line.end(), re, -1 }, last;

      // Se are looking for (x, y, z, value). So, we must have at least 4
      // components.
      if (std::distance(first, last) < 4)
      {
        return false;
      }

      p[0] = std::stod(*(first++));
      p[1] = std::stod(*(first++));
      p[2] = std::stod(*(first++));
      value = std::stod(*(first++));
      this->addSourcePoint(p, value);
    }

    infile.close();
    return true;
  }

private:
  std::vector<std::pair<Point, double> > m_sources;
  double m_power = 1.;
};

class ComputeCellField : public smtk::mesh::CellForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  ComputeCellField(
    const std::function<double(double, double, double)>& dataGenerator, std::size_t nCells)
    : smtk::mesh::CellForEach(true)
    , m_dataGenerator(dataGenerator)
    , m_data(nCells)
    , m_counter(0)
  {
  }

  void forCell(const smtk::mesh::Handle&, smtk::mesh::CellType, int nPts)
  {
    double xyz[3] = { 0., 0., 0. };
    for (int i = 0; i < nPts; i++)
    {
      xyz[0] += this->coordinates()[i * 3 + 0];
      xyz[1] += this->coordinates()[i * 3 + 1];
      xyz[2] += this->coordinates()[i * 3 + 2];
    }
    for (int i = 0; i < 3; i++)
    {
      xyz[i] /= nPts;
    }
    this->m_data[this->m_counter++] = this->m_dataGenerator(xyz[0], xyz[1], xyz[2]);
  }

  const std::vector<double>& data() const { return this->m_data; }
};

class ComputePointField : public smtk::mesh::PointForEach
{
private:
  const std::function<double(double, double, double)>& m_dataGenerator;
  std::vector<double> m_data;
  std::size_t m_counter;

public:
  ComputePointField(
    const std::function<double(double, double, double)>& dataGenerator, std::size_t nPoints)
    : smtk::mesh::PointForEach()
    , m_dataGenerator(dataGenerator)
    , m_data(nPoints)
    , m_counter(0)
  {
  }

  void forPoints(const smtk::mesh::HandleRange& pointIds, std::vector<double>& xyz, bool&)
  {
    std::size_t xyzCounter = 0;
    typedef smtk::mesh::HandleRange::const_iterator c_it;
    for (c_it i = pointIds.begin(); i != pointIds.end(); ++i)
    {
      double value = this->m_dataGenerator(
        xyz[xyzCounter * 3 + 0], xyz[xyzCounter * 3 + 1], xyz[xyzCounter * 3 + 2]);
      this->m_data[this->m_counter++] = value;
      xyzCounter += 3;
    }
  }

  const std::vector<double>& data() const { return this->m_data; }
};
}

namespace smtk
{
namespace model
{

bool InterpolateMesh::ableToOperate()
{
  if (!this->ensureSpecification())
  {
    return false;
  }

  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");
  if (!meshItem || meshItem->numberOfValues() == 0)
  {
    return false;
  }

  return true;
}

smtk::model::OperatorResult InterpolateMesh::operateInternal()
{
  // Access the mesh to elevate
  smtk::attribute::MeshItem::Ptr meshItem = this->specification()->findMesh("mesh");

  // Access the data set name
  smtk::attribute::StringItem::Ptr nameItem = this->specification()->findString("dsname");

  // Access the output interpolation Field type
  smtk::attribute::IntItem::Ptr modeItem = this->specification()->findInt("interpmode");

  // Access the interpolation points
  smtk::attribute::GroupItem::Ptr interpolationPointsItem = this->findGroup("points");

  // Access the interpolation power parameter
  smtk::attribute::DoubleItem::Ptr powerItem = this->findDouble("power");

  // Construct an instance of our interpolator and set its parameters
  ShepardInterpolator interpolator;
  interpolator.setPower(powerItem->value());

  // Access the points CSV file name, if it is enabled
  smtk::attribute::FileItem::Ptr ptsFileItem = this->specification()->findFile("ptsfile");
  if (ptsFileItem->isEnabled())
  {
    bool success = interpolator.readPointsFromCSV(ptsFileItem->value(0));
    if (!success)
    {
      smtkErrorMacro(this->log(), "Could not read CSV file.");
      return this->createResult(OPERATION_FAILED);
    }
  }

  for (std::size_t i = 0; i < interpolationPointsItem->numberOfGroups(); i++)
  {
    smtk::attribute::DoubleItemPtr pointItem =
      smtk::dynamic_pointer_cast<smtk::attribute::DoubleItem>(interpolationPointsItem->item(i, 0));

    Point p = { { pointItem->value(0), pointItem->value(1), pointItem->value(2) } };
    interpolator.addSourcePoint(p, pointItem->value(3));
  }

  // Access the attribute associated with the modified meshes
  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  smtk::attribute::MeshItem::Ptr modifiedMeshes = result->findMesh("mesh_modified");
  modifiedMeshes->setNumberOfValues(meshItem->numberOfValues());

  // Access the attribute associated with the changed tessellation
  smtk::attribute::ModelEntityItem::Ptr modifiedEntities = result->findModelEntity("tess_changed");
  modifiedEntities->setNumberOfValues(meshItem->numberOfValues());

  std::function<double(double, double, double)> fn = [&](
    double x, double y, double z) { return interpolator(x, y, z); };

  // apply the interpolator to the meshes and populate the result attributes
  for (std::size_t i = 0; i < meshItem->numberOfValues(); i++)
  {
    smtk::mesh::MeshSet mesh = meshItem->value(i);

    if (modeItem->value(0) == CELL_FIELD)
    {
      ComputeCellField computeCellField(fn, mesh.cells().size());
      smtk::mesh::for_each(mesh.cells(), computeCellField);
      mesh.createCellField(nameItem->value(0), 1, &computeCellField.data()[0]);
    }
    else
    {
      ComputePointField computePointField(fn, mesh.points().size());
      smtk::mesh::for_each(mesh.points(), computePointField);
      mesh.createPointField(nameItem->value(0), 1, &computePointField.data()[0]);
    }

    modifiedMeshes->appendValue(mesh);

    smtk::model::EntityRefArray entities;
    bool entitiesAreValid = mesh.modelEntities(entities);
    if (entitiesAreValid && !entities.empty())
    {
      smtk::model::Model model = entities[0].owningModel();
      this->addEntityToResult(result, model, MODIFIED);
      modifiedEntities->appendValue(model);
    }
  }

  return result;
}
}
}

#include "smtk/model/InterpolateMesh_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::InterpolateMesh, interpolate_mesh,
  "interpolate mesh", InterpolateMesh_xml, smtk::model::Session);
