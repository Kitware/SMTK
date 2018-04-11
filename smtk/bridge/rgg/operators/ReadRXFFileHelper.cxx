//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/ReadRXFFileHelper.h"

#include "smtk/bridge/rgg/operators/ReadRXFFile.h"

#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.cpp"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/rgg/operators/CreateAssembly.h"
#include "smtk/bridge/rgg/operators/CreateDuct.h"
#include "smtk/bridge/rgg/operators/CreateModel.h"
#include "smtk/bridge/rgg/operators/CreatePin.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"

#include <limits>

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

// FIXME: These code are duplicated in smtkRGGEditAssemblyView
namespace
{
static const double cos30 = 0.86602540378443864676372317075294;
static const double cos60 = 0.5;
static const int degreesHex[6] = { -120, -60, 0, 60, 120, 180 };
static const int degreesRec[4] = { -90, 0, 90, 180 };
// 0', 60', 120, 180', 240', 300'
static const double cosSinAngles[6][2] = { { 1.0, 0.0 }, { cos60, -cos30 }, { -cos60, -cos30 },
  { -1.0, 0.0 }, { -cos60, cos30 }, { cos60, cos30 } };

void calculateDuctMinimimThickness(
  const smtk::model::EntityRef& duct, double& thickness0, double& thickness1)
{
  smtk::model::FloatList pitches, thicknesses;
  if (duct.owningModel().hasFloatProperty("duct thickness"))
  {
    pitches = duct.owningModel().floatProperty("duct thickness");
    if (pitches.size() != 2)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "duct "
          << duct.name() << "'s owning model does not have a valid pitch");
      return;
    }
  }
  if (duct.hasFloatProperty("thicknesses(normalized)"))
  {
    thicknesses = duct.floatProperty("thicknesses(normalized)");
    if (thicknesses.size() / 2 < 1)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "duct " << duct.name() << " does not have valid thicknesses");
      return;
    }
  }
  thickness0 = std::numeric_limits<double>::max();
  thickness1 = std::numeric_limits<double>::max();
  for (auto i = 0; i < thicknesses.size() / 2; i++)
  {
    double currentT0 = pitches[0] * thicknesses[i * 2];
    double currentT1 = pitches[1] * thicknesses[i * 2 + 1];
    thickness0 = (currentT0 < thickness0) ? currentT0 : thickness0;
    thickness1 = (currentT1 < thickness1) ? currentT1 : thickness1;
  }
}

// Calculate the x,y coordinates of the current pin in the hex grid
void calculateHexPinCoordinate(
  double& x, double& y, const double& spacing, const int& ring, const int& layer)
{
  // The index order of layer is clockwise, starting from upper left corner of the hex.
  // It's RGG's order and not ideal...
  if (ring == 0)
  {
    x = y = 0;
  }
  else
  {
    int remainder = layer % ring;
    int modulus = layer / ring;

    double eL = ring * spacing;                       // edge length
    double xBT = -eL * cos60 + eL / (ring)*remainder; // x before transform
    double yBT = eL * cos30;                          // y befor transform
    // Apply rotation if needed. In order to avoid sin/cos calculation, we use
    // predefined values;
    double cosValue = cosSinAngles[modulus][0];
    double sinValue = cosSinAngles[modulus][1];

    x = xBT * cosValue - yBT * sinValue;
    y = yBT * cosValue + xBT * sinValue;
  }
}

// FIXME: This function is duplicated in smtkRGGEditCoreView
void calculateHexAssyCoordinate(
  double& x, double& y, const double& spacing, const int& ring, const int& layer)
{
  // The index order of layer is clockwise, starting from upper left corner of the hex.
  // It's RGG's order and not ideal...
  if (ring == 0)
  {
    x = y = 0;
  }
  else
  {
    int remainder = layer % ring;
    int modulus = layer / ring;

    double eL = ring * spacing;                       // edge length
    double xBT = -eL * cos60 + eL / (ring)*remainder; // x before transform
    double yBT = eL * cos30;                          // y befor transform
    // Apply rotation if needed. In order to avoid sin/cos calculation, we use
    // predefined values;
    double cosValue = cosSinAngles[modulus][0];
    double sinValue = cosSinAngles[modulus][1];

    double x0 = xBT * cosValue - yBT * sinValue;
    double y0 = yBT * cosValue + xBT * sinValue;

    // Rotate 330 degree due to the fact that the orientations do not match in
    // the render view and schema planner
    // sin330 = -cos60 and cos330 = cos30;
    x = x0 * cos30 - y0 * (-cos60);
    y = y0 * cos30 + x0 * (-cos60);
  }
}
}
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{
using namespace XMLAttribute;

namespace
{
bool read(pugi::xml_node& node, std::string attName, std::string& v)
{
  pugi::xml_attribute att = node.attribute(attName.c_str());
  if (!att)
  {
    return false;
  }
  v = std::string(att.value());
  return true;
}

bool read(pugi::xml_node& node, std::string attName, double& v)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  v = std::atof(str.c_str());
  return true;
}

bool read(pugi::xml_node& node, std::string attName, int& v)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  v = std::atoi(str.c_str());
  return true;
}

bool read(pugi::xml_node& node, std::string attName, unsigned int& v)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  v = std::stoul(str.c_str());
  return true;
}

bool read(pugi::xml_node& node, std::string attName, bool& v)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  v = static_cast<bool>(std::atoi(str.c_str()));
  return true;
}

bool read(pugi::xml_node& node, std::string attName, double* v, int size)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  regex re(",");
  sregex_token_iterator it(str.begin(), str.end(), re, -1), last;
  for (int i = 0; (it != last && i < size); ++it, ++i)
  {
    v[i] = std::atof(it->str().c_str());
  }
  return true;
}

bool read(pugi::xml_node& node, std::string attName, std::vector<std::string>& values)
{
  std::string str;
  if (!read(node, attName, str))
    return false;
  regex re(";");
  sregex_token_iterator it(str.begin(), str.end(), re, -1), last;
  for (; it != last; ++it)
  {
    values.push_back(it->str());
  }
  return true;
}

bool readColor(pugi::xml_node& node, std::string attName, std::vector<double>& color)
{
  pugi::xml_attribute att = node.attribute(attName.c_str());
  if (!att)
  {
    return false;
  }
  std::string ts(att.value());
  std::vector<double> values;
  regex re(",");
  sregex_token_iterator it(ts.begin(), ts.end(), re, -1), last;
  for (int i = 0; (it != last && i < 4); ++it, ++i)
  {
    values.push_back(std::atof(it->str().c_str()));
  }

  color.clear();
  if (values.size() == 4)
  {
    color = values;
    return true;
  }
  else
  {
    color = { 1, 1, 1, 1 };
    return false;
  }
}

size_t materialNameToIndex(std::string materialN, const std::vector<std::string>& materialList)
{
  auto iter = std::find(materialList.begin(), materialList.end(), materialN);
  if (iter != materialList.end())
  {
    size_t pos = std::distance(materialList.begin(), iter);
    return pos;
  }
  if (!materialN.empty())
  {
    smtkErrorMacro(smtk::io::Logger().instance(), "Cannot find index for"
                                                  " material '"
        << materialN << "'. Set it to be the first defined material");
  }
  return 0;
}
}

bool ReadRXFFileHelper::parseMaterial(pugi::xml_node node, smtk::model::EntityRef model)
{
  smtk::model::StringList materialsList;
  bool r = true;
  for (auto materialN = node.child((MATERIAL_TAG.c_str())); materialN;
       materialN = materialN.next_sibling(MATERIAL_TAG.c_str()))
  {
    std::string name, label;
    std::vector<double> color;
    r &= read(materialN, NAME_TAG.c_str(), name);
    r &= read(materialN, LABEL_TAG.c_str(), label);
    r &= readColor(materialN, COLOR_TAG.c_str(), color);
    materialsList.push_back(name);
    model.setStringProperty(name, label);
    model.setFloatProperty(name, color);
  }
  model.setStringProperty("materials", materialsList);
  return r;
}

bool ReadRXFFileHelper::parseDefaults(const pugi::xml_node node, EntityRef model)
{
  bool r = true;
  pugi::xml_node defaultsN = node.child(DEFAULTS_TAG.c_str());
  double length(std::numeric_limits<double>::max()), zOrigin(0), thicknesses[2];
  read(defaultsN, LENGTH_TAG.c_str(), length);
  read(defaultsN, Z0_TAG.c_str(), zOrigin);
  if (read(defaultsN, THICKNESS_TAG.c_str(), thicknesses, 2))
  {
    smtk::model::FloatList tmp = { thicknesses[0], thicknesses[1] };
    model.setFloatProperty("duct thickness", tmp);
  }

  if (length != std::numeric_limits<double>::max())
  { // A valid default is provided
    smtk::model::FloatList z0z1 = { zOrigin, zOrigin + length };
    // Copy the logic from EditCore op
    // Update ducts. Duck thickness is taken care of by the model, so we only need
    // to use the "duct height" property on the model to update the z values in each
    // duct.
    model.setFloatProperty("duct height", z0z1);
    // TODO: It might be a performance bottleneck since modifying ducts would
    // trigger the glyph3DMapper to render the ducts again
    smtk::model::EntityRefArray ducts =
      model.manager()->findEntitiesByProperty("rggType", SMTK_BRIDGE_RGG_DUCT);
    for (auto iter = ducts.begin(); iter != ducts.end(); iter++)
    {
      smtk::model::AuxiliaryGeometry ductA = iter->as<smtk::model::AuxiliaryGeometry>();
      if (!(ductA.auxiliaryGeometries().size() > 0) || // duct subparts
        (!ductA.hasFloatProperty("z values") || ductA.floatProperty("z values").size() < 2))
      {
        continue;
      }
      smtk::model::FloatList& zValues = ductA.floatProperty("z values");
      double oldZO = zValues[0], newZO = zOrigin;
      double ratio = (length) / (zValues[zValues.size() - 1] - zValues[0]);
      // Substract the original z origin, multiply the size change ratio then
      // add the new z origin.
      std::transform(zValues.begin(), zValues.end(), zValues.begin(),
        [&oldZO, &newZO, &ratio](double v) { return ((v - oldZO) * ratio + newZO); });
    }
  }
  return r;
}

bool ReadRXFFileHelper::parseDuctsAndCoreProperty(const pugi::xml_node rootNode,
  smtk::model::EntityRef model, smtk::model::EntityRefArray& newDucts)
{
  bool r(true);
  // Get the geometry type
  pugi::xml_node assemblyNode = rootNode.child(ASSEMBLY_TAG.c_str());
  std::string geomType;
  read(assemblyNode, GEOMETRY_TAG.c_str(), geomType);
  std::transform(geomType.begin(), geomType.end(), geomType.begin(), ::tolower);
  int geomTypeInt = (geomType == "hexagonal") ? 1 : 0;
  model.setIntegerProperty("hex", geomTypeInt);
  pugi::xml_node ductsN = rootNode.child(DUCTS_TAG.c_str());
  pugi::xml_node ductN = ductsN.child(DUCT_CELL_TAG.c_str());

  // Use the first duct to calculate Z origin, Z Max, thickness0 and thickness1
  {
    std::vector<double> zOzMthicknesses = { -1.0, -1.0, -1.0, -1.0 };
    bool isHeightThicknessSet(false);
    for (pugi::xml_node dLN = ductN.child(DUCT_LAYER_TAG.c_str()); dLN;
         dLN = dLN.next_sibling(DUCT_LAYER_TAG.c_str()))
    {
      // X, y, z0 and z1
      double location[4];
      read(dLN, LOC_TAG.c_str(), location, 4);
      if (!isHeightThicknessSet)
      {
        double thickness[2]; // Duct pitches
        r &= read(dLN, THICKNESS_TAG.c_str(), thickness, 2);
        zOzMthicknesses[0] = location[2]; // z0
        zOzMthicknesses[1] = location[3]; // z1
        zOzMthicknesses[2] = thickness[0];
        zOzMthicknesses[3] = thickness[1];
        isHeightThicknessSet = true;
      }
      // Update z origin and z max
      zOzMthicknesses[0] = (zOzMthicknesses[0] <= location[2]) ? zOzMthicknesses[0] : location[2];
      zOzMthicknesses[1] = (zOzMthicknesses[1] >= location[3]) ? zOzMthicknesses[1] : location[3];
    }
    smtk::model::FloatList zOzM = { zOzMthicknesses[0], zOzMthicknesses[1] };
    model.setFloatProperty("duct height", zOzM);
    // thickness/ thicknessX and thicknessY
    smtk::model::FloatList thicknesses = { zOzMthicknesses[2], zOzMthicknesses[3] };
    model.setFloatProperty("duct thickness", thicknesses);
  }

  for (; ductN; ductN = ductN.next_sibling(DUCT_CELL_TAG.c_str()))
  {
    r &= ReadRXFFileHelper::parseDuctNodeAndCreate(ductN, model, newDucts);
    if (r == false)
    {
      std::string name;
      r &= read(ductN, NAME_TAG.c_str(), name);
      smtkErrorMacro(smtk::io::Logger(), "Encounter erros when parsing duct " << name);
    }
  }
  return r;
}

bool ReadRXFFileHelper::parsePins(
  pugi::xml_node pinsNode, smtk::model::EntityRef model, smtk::model::EntityRefArray& newPins)
{
  bool r(true);
  for (auto pinN = pinsNode.child(PINCELL_TAG.c_str()); pinN;
       pinN = pinN.next_sibling(PINCELL_TAG.c_str()))
  {
    r &= ReadRXFFileHelper::parsePin(pinN, model, newPins);
  }
  return r;
}

bool ReadRXFFileHelper::parseAssemblies(pugi::xml_node rootNode, EntityRef model,
  EntityRefArray& newAssys, EntityRefArray& newAssyInstances, StringToEnt& labelToPin,
  StringToEnt& nameToDuct)
{
  bool r(true);
  for (pugi::xml_node aNode = rootNode.child(ASSEMBLY_TAG.c_str()); aNode;
       aNode = aNode.next_sibling(ASSEMBLY_TAG.c_str()))
  {
    r &= ReadRXFFileHelper::parseAssembly(
      aNode, model, newAssys, newAssyInstances, labelToPin, nameToDuct);
  }
  return r;
}

bool ReadRXFFileHelper::parseCore(pugi::xml_node rootNode, EntityRef model,
  EntityRefArray& newCoreInstances, StringToEnt& labelToAssy)
{
  bool r(true);
  EntsAndCoords entsAndCoords;
  smtk::model::EntityRefArray coreArray =
    model.manager()->findEntitiesByProperty("rggType", SMTK_BRIDGE_RGG_CORE);
  smtk::model::Group core;
  bool isHex(false);
  if (coreArray.size() > 0 && coreArray[0].owningModel().hasIntegerProperty("hex"))
  {
    isHex = coreArray[0].owningModel().integerProperty("hex")[0];
    core = coreArray[0].as<smtk::model::Group>();
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "An invalid core is provided to parse"
                                       " core info");
    return false;
  }

  pugi::xml_node latticeNode = rootNode.child(LATTICE_TAG.c_str());
  // ring and layer/x and y in the hex/rect schema planner
  LabelToLayout assyLabelToLayout;
  r &= ReadRXFFileHelper::parseLattice(latticeNode, assyLabelToLayout, core);
  smtk::model::IntegerList latticeSize = core.owningModel().integerProperty("lattice size");
  // Copy the logic from EditCore op
  smtk::model::FloatList spacing = model.floatProperty("duct thickness");
  double baseX, baseY;
  if (!isHex)
  { // Use the cartesian coordinate where the starting point is located
    // at left bottom
    baseX = -1 * spacing[0] * (static_cast<double>(latticeSize[0]) / 2 - 0.5);
    baseY = -1 * spacing[1] * (static_cast<double>(latticeSize[1]) / 2 - 0.5);
  }
  else
  { // Spacing is the allowable max distance between two adjacent assembly centers
    // Use the cartesian coordinate where the starting point is located at
    // the origin point.
    baseX = baseY = 0.0; // Ignored by calculateHexAssyCoordinate for now
  }

  // Map pins&ducts to their placements
  std::vector<std::string> assyUUIDs;
  for (auto iter = assyLabelToLayout.begin(); iter != assyLabelToLayout.end(); iter++)
  { // For each assembly, retrieve its pins&duct info, apply the right transformation
    // then add it into pinDuctToLayout map

    smtk::model::EntityRef assy;
    if (labelToAssy.find(iter->first) == labelToAssy.end())
    {
      continue;
    }
    assy = labelToAssy[iter->first];
    std::vector<long> layout = iter->second;
    smtk::model::FloatList coordinates;
    size_t numberOfPairs = layout.size() / 2;
    coordinates.reserve(numberOfPairs * 3);
    std::string ductUUID;
    if (assy.hasStringProperty("associated duct"))
    {
      ductUUID = assy.stringProperty("associated duct")[0];
    }
    else
    {
      smtkErrorMacro(smtk::io::Logger(), "The assembly "
          << assy.name() << "does"
                            "not have a valid assocated duct, skipping it in the core");
      continue;
    }

    smtk::model::StringList pinIds;
    if (!assy.hasStringProperty("pins") && ductUUID.empty())
    {
      smtkErrorMacro(smtk::io::Logger(), "The assembly "
          << assy.name() << "does"
                            "not have pin ids and duct id cached, skipping it in the core");
      continue;
    }
    else
    {
      pinIds = assy.stringProperty("pins");
    }

    size_t pointSize = layout.size() / 2;
    for (size_t index = 0; index < pointSize; index++)
    {
      double x, y;
      if (isHex)
      {
        calculateHexAssyCoordinate(x, y, spacing[0], layout[2 * index], layout[2 * index + 1]);
      }
      else
      {
        // In schema planner, x and y axis are following Qt's pattern.
        // Here we just follow the traditional coordinate convension
        x = baseX + spacing[0] * layout[2 * index];
        y = baseY + spacing[1] * layout[2 * index + 1];
      }

      coordinates.push_back(x);
      coordinates.push_back(y);
      coordinates.push_back(0);
      // For each (x,y) pair, add it to every pin and duct in the current assy
      auto addTransformCoordsToMap = [&entsAndCoords, &x, &y](
        const smtk::model::EntityRef& ent, std::vector<double>& coordinates) {
        // Apply transformation
        for (size_t i = 0; i < coordinates.size(); i++)
        {
          if (i % 3 == 0)
          { // X
            coordinates[i] += x;
          }
          if (i % 3 == 1)
          { // Y
            coordinates[i] += y;
          }
        }

        if (entsAndCoords.find(ent) != entsAndCoords.end())
        { // TODO: Possible performance bottleneck
          entsAndCoords[ent].insert(
            entsAndCoords[ent].end(), coordinates.begin(), coordinates.end());
        }
        else
        {
          entsAndCoords[ent] = coordinates;
        }
      };
      // Duct
      smtk::model::EntityRef duct = smtk::model::EntityRef(assy.manager(), ductUUID);
      std::vector<double> ductCoords = { 0, 0, 0 };
      addTransformCoordsToMap(duct, ductCoords);
      // Pins
      for (const auto pinId : pinIds)
      {
        smtk::model::EntityRef pin = smtk::model::EntityRef(assy.manager(), pinId);
        if (!assy.hasFloatProperty(pinId))
        {
          smtkErrorMacro(smtk::io::Logger(), "Assembly "
              << assy.name() << "does"
                                "not have pin "
              << pin.name() << "'s coordinates, skipping it in the core");
          continue;
        }
        std::vector<double> pinCoords = assy.floatProperty(pinId);
        addTransformCoordsToMap(pin, pinCoords);
      }
    }
    std::string assyUUID = assy.entity().toString();
    model.setIntegerProperty(assyUUID, layout);
    model.setFloatProperty(assyUUID, coordinates);
    assyUUIDs.push_back(assyUUID);
  }
  model.setStringProperty("assemblies", assyUUIDs);
  // Glyph the duct and pins
  ReadRXFFileHelper::createInstances(entsAndCoords, core, newCoreInstances);
  // TODO: Add support for unknown attribute
  return r;
}

bool ReadRXFFileHelper::parseDuctNodeAndCreate(
  pugi::xml_node node, smtk::model::EntityRef model, smtk::model::EntityRefArray& newDucts)
{
  bool r = true;
  smtk::model::ManagerPtr mgr = model.manager();
  smtk::model::StringList materialList = model.stringProperty("materials");
  smtk::model::AuxiliaryGeometry duct;

  duct = mgr->addAuxiliaryGeometry(model.as<smtk::model::Model>(), 3);
  newDucts.push_back(duct);

  duct.setStringProperty("rggType", SMTK_BRIDGE_RGG_DUCT);
  model.setStringProperty("latest duct", duct.entity().toString());
  duct.setVisible(false);

  std::string name;
  r &= read(node, NAME_TAG.c_str(), name);
  duct.setName(name);

  smtk::model::IntegerList numMaterialsPerSeg, ductMaterials;
  smtk::model::FloatList zValues, thicknesses;

  int ductLayerCount(0);
  for (pugi::xml_node dLN = node.child(DUCT_LAYER_TAG.c_str()); dLN;
       dLN = dLN.next_sibling(DUCT_LAYER_TAG.c_str()), ductLayerCount++)
  {
    // Cache current z0 and z1
    double xyZ0Z1[4];
    if (!read(dLN, LOC_TAG.c_str(), xyZ0Z1, 4))
    {
      smtkErrorMacro(
        smtk::io::Logger().instance(), "Duct " << name << " has an invalid location, skip it.");
      continue;
    }
    zValues.push_back(xyZ0Z1[2]);
    zValues.push_back(xyZ0Z1[3]);

    int count(0);
    for (auto mN = dLN.child(MATERIAL_LAYER_TAG.c_str()); mN;
         mN = mN.next_sibling(MATERIAL_LAYER_TAG.c_str()), count++)
    {
      std::string mName;
      read(mN, MATERIAL_TAG.c_str(), mName);
      size_t pos = materialNameToIndex(mName, materialList);
      ductMaterials.push_back(pos);

      double radius[2];
      read(mN, THICKNESS_TAG.c_str(), radius, 2);
      thicknesses.push_back(radius[0]);
      thicknesses.push_back(radius[1]);
    }
    numMaterialsPerSeg.push_back(count);
  }
  duct.setFloatProperty("z values", zValues);
  duct.setIntegerProperty("materials", ductMaterials);
  duct.setFloatProperty("thicknesses(normalized)", thicknesses);
  // Helper property for segments which would be used as an offset hint
  duct.setIntegerProperty("material nums per segment", numMaterialsPerSeg);

  auto assignColor = [](size_t index, smtk::model::AuxiliaryGeometry& aux) {
    smtk::model::FloatList rgba;
    smtk::bridge::rgg::CreateModel::getMaterialColor(index, rgba, aux.owningModel());
    aux.setColor(rgba);
  };

  // Create auxgeom placeholders for layers and parts
  size_t materialIndex(0);
  for (std::size_t i = 0; i < ductLayerCount; i++)
  {
    for (std::size_t j = 0; j < numMaterialsPerSeg[i]; j++)
    {
      // Create an auxo_geom for every layer in current segment
      AuxiliaryGeometry subLayer = mgr->addAuxiliaryGeometry(duct, 3);
      std::string subLName = name + SMTK_BRIDGE_RGG_DUCT_SEGMENT + std::to_string(i) +
        SMTK_BRIDGE_RGG_DUCT_LAYER + std::to_string(j);
      subLayer.setName(subLName);
      subLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_DUCT);
      subLayer.setVisible(false);
      assignColor(ductMaterials[materialIndex++], subLayer);
      newDucts.push_back(subLayer.as<EntityRef>());
    }
  }
  if (!r)
  {
    smtkErrorMacro(smtk::io::Logger(), "Encounter errors when paring duct " << name);
  }
  return r;
}

bool ReadRXFFileHelper::parsePin(pugi::xml_node pinNode, EntityRef model, EntityRefArray& newPins)
{
  bool r = true;
  smtk::model::ManagerPtr mgr = model.manager();
  smtk::model::StringList materialList = model.stringProperty("materials");
  smtk::model::AuxiliaryGeometry pin = mgr->addAuxiliaryGeometry(model.as<smtk::model::Model>(), 3);
  pin.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);
  newPins.push_back(pin);
  model.setStringProperty("latest pin", pin.entity().toString());
  pin.setIntegerProperty("cut away", 0);
  pin.setVisible(false);

  double zOrigin = std::numeric_limits<double>::max();

  std::string name, label, cellMaterial;
  std::vector<double> color;
  smtk::model::IntegerList subMaterials, piecesSegType;
  smtk::model::FloatList radiusNs, typeParas;

  r &= read(pinNode, NAME_TAG.c_str(), name);
  pin.setName(name);

  r &= read(pinNode, LABEL_TAG.c_str(), label);
  // Make sure that the label is unique
  if (model.hasStringProperty("pin labels list"))
  {
    smtk::model::StringList& labels = model.stringProperty("pin labels list");
    int count = 0;
    while (std::find(std::begin(labels), std::end(labels), label) != std::end(labels))
    { // need to generate a new label
      label = "PC" + std::to_string(count);
      count++;
    }
    // Update the label list
    labels.push_back(label);
  }
  else
  {
    model.setStringProperty("pin labels list", label);
  }
  pin.setStringProperty("label", label);

  r &= readColor(pinNode, LEGEND_COLOR_TAG.c_str(), color);
  pin.setColor(color);

  // Cell material
  read(pinNode, MATERIAL_TAG.c_str(), cellMaterial);
  size_t cMPos = materialNameToIndex(cellMaterial, materialList);
  pin.setIntegerProperty("cell material", cMPos);

  // Materials
  // TODO: In XRF file, materials are defined for each cylinder and frustum.
  // However, For each pin, it has only one material definition just along radius.
  // All subparts share the same material. If NEAMS workflow has a different
  // settings, we should change it here. For now I just calculate it once.
  bool layerIsCalculated(false);
  auto calculatePinMaterial = [&materialList, &subMaterials, &radiusNs](pugi::xml_node node) {
    for (auto mN = node.child(MATERIAL_LAYER_TAG.c_str()); mN;
         mN = mN.next_sibling(MATERIAL_LAYER_TAG.c_str()))
    {
      std::string mName;
      read(mN, MATERIAL_TAG.c_str(), mName);
      size_t pos = materialNameToIndex(mName, materialList);

      double radius[2];
      // Though RXF define two values, SMTK would only consume one value
      // since the cross section is a cirle
      read(mN, THICKNESS_TAG.c_str(), radius, 2);
      subMaterials.push_back(pos);
      radiusNs.push_back(radius[0]);
    }
  };

  // TODO: FIXME: For now I just blindy follow the rgg logic: Read all cylinders
  // then frustums. Order is ignored. Intermix would be a problem here.
  for (pugi::xml_node cylinderN = pinNode.child(CYLINDER_TAG.c_str()); cylinderN;
       cylinderN = cylinderN.next_sibling(CYLINDER_TAG.c_str()))
  {
    double radius(0);
    read(cylinderN, RADIUS_TAG.c_str(), radius);
    // pin sub parts
    double xyZ0Z1[4];
    if (!read(cylinderN, LOC_TAG.c_str(), xyZ0Z1, 4))
    {
      smtkErrorMacro(
        smtk::io::Logger().instance(), "Pin " << name << " has an invalid location, skip it.");
      continue;
    }
    // TODO: now pin does not support non origin point value
    // Update zOrigin. It should only happen one time
    if (zOrigin > xyZ0Z1[2])
    {
      zOrigin = xyZ0Z1[2];
    }
    double height = xyZ0Z1[3] - xyZ0Z1[2];
    // Cylinder or Frustum
    piecesSegType.push_back(0); // Cylinder
    // length, base radius and top radius
    typeParas.push_back(height);
    typeParas.push_back(radius);
    typeParas.push_back(radius);
    if (!layerIsCalculated)
    {
      calculatePinMaterial(cylinderN);
      layerIsCalculated = true;
    }
  }

  for (pugi::xml_node frustumN = pinNode.child(FRUSTRUM_TAG.c_str()); frustumN;
       frustumN = frustumN.next_sibling(FRUSTRUM_TAG.c_str()))
  {
    double radius[2];
    read(frustumN, RADIUS_TAG.c_str(), radius, 2);
    // pin sub parts
    double xyZ0Z1[4];
    if (!read(frustumN, LOC_TAG.c_str(), xyZ0Z1, 4))
    {
      smtkErrorMacro(
        smtk::io::Logger().instance(), "Pin " << name << " has an invalid location, skip it.");
      continue;
    }
    // TODO: now pin does not support non origin point value
    // Update zOrigin. It should only happen one time
    if (zOrigin > xyZ0Z1[2])
    {
      zOrigin = xyZ0Z1[2];
    }
    double height = xyZ0Z1[3] - xyZ0Z1[2];
    // Cylinder or Frustum
    piecesSegType.push_back(1); // Frustum
    // length, base radius and top radius
    typeParas.push_back(height);
    typeParas.push_back(radius[0]);
    typeParas.push_back(radius[1]);
    if (!layerIsCalculated)
    {
      calculatePinMaterial(frustumN);
      layerIsCalculated = true;
    }
  }
  pin.setIntegerProperty("pieces", piecesSegType);
  pin.setFloatProperty("pieces", typeParas);

  pin.setIntegerProperty("layer materials", subMaterials);
  pin.setFloatProperty("layer materials", radiusNs);
  // Get the max radius and cache it for create assembly purpose
  double maxRadius(-1);
  for (size_t i = 0; i < typeParas.size() / 3; i++)
  {
    maxRadius = std::max(maxRadius, std::max(typeParas[i * 3 + 1], typeParas[i * 3 + 2]));
  }
  pin.setFloatProperty("max radius", maxRadius);

  // TODO: Z origin
  pin.setFloatProperty("z origin", zOrigin);

  auto assignColor = [](size_t index, smtk::model::AuxiliaryGeometry& aux) {
    smtk::model::FloatList rgba;
    smtk::bridge::rgg::CreateModel::getMaterialColor(index, rgba, aux.owningModel());
    aux.setColor(rgba);
  };
  // Create auxgeom placeholders for layers and parts
  size_t numParts(piecesSegType.size()), numLayers(radiusNs.size());
  for (std::size_t i = 0; i < numParts; i++)
  {
    for (std::size_t j = 0; j < numLayers; j++)
    {
      // Create an auxo_geom for current each unit part&layer
      AuxiliaryGeometry subLayer = mgr->addAuxiliaryGeometry(pin, 3);
      std::string subLName = name + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(i) +
        SMTK_BRIDGE_RGG_PIN_LAYER + std::to_string(j);
      subLayer.setName(subLName);
      subLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);
      subLayer.setVisible(false);
      assignColor(subMaterials[j], subLayer);
      newPins.push_back(subLayer.as<EntityRef>());
    }
    if (cMPos) // 0 index means no material
    {          // Append a material layer after the last layer
      AuxiliaryGeometry materialLayer = mgr->addAuxiliaryGeometry(pin, 3);
      std::string materialName =
        name + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(i) + SMTK_BRIDGE_RGG_PIN_MATERIAL;
      materialLayer.setName(materialName);
      materialLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);
      materialLayer.setVisible(false);
      assignColor(cMPos, materialLayer);
      newPins.push_back(materialLayer.as<EntityRef>());
    }
  }
  if (!r)
  {
    smtkErrorMacro(smtk::io::Logger(), "Encounter errors when paring pin " << name);
  }
  return r;
}

bool ReadRXFFileHelper::parseAssembly(pugi::xml_node assyNode, EntityRef model,
  EntityRefArray& newAssys, EntityRefArray& newAssyInstances, StringToEnt& labelToPin,
  StringToEnt& nameToDuct)
{
  bool r(true);
  // Entities that should be glyphed and their coordinates during create
  // assembly process(sub parts of duct and pins)
  EntsAndCoords entsAndCoords;
  smtk::model::ManagerPtr mgr = model.manager();
  smtk::model::Group assembly = mgr->addGroup(0, "group"); // Assign the name later
  newAssys.push_back(assembly);

  smtk::model::Model modelM = model.as<smtk::model::Model>();
  modelM.addGroup(assembly);
  BitFlags mask(0);
  mask |= smtk::model::AUX_GEOM_ENTITY;
  mask |= smtk::model::INSTANCE_ENTITY;
  assembly.setMembershipMask(mask);
  assembly.setStringProperty("rggType", SMTK_BRIDGE_RGG_ASSEMBLY);
  assembly.setVisible(false);

  std::string name, label;
  std::vector<double> color;
  r &= read(assyNode, LABEL_TAG.c_str(), label);
  r &= readColor(assyNode, LEGEND_COLOR_TAG.c_str(), color);
  // Make sure that the label is unique
  if (model.hasStringProperty("assembly labels list"))
  {
    smtk::model::StringList& labels = model.stringProperty("assembly labels list");
    int count = 0;
    while (std::find(std::begin(labels), std::end(labels), label) != std::end(labels))
    { // need to generate a new label
      label = "A" + std::to_string(count);
      count++;
    }
    // Update the label list
    labels.push_back(label);
  }
  else
  {
    model.setStringProperty("assembly labels list", label);
  }
  name = "assembly_" + label;
  assembly.setName(name);
  assembly.setStringProperty("label", label);
  assembly.setColor(color);

  int centerPins(1);
  r &= read(assyNode, CENTER_PINS_TAG.c_str(), centerPins);
  assembly.setIntegerProperty("center pins", centerPins);

  double pitches[2];
  r &= read(assyNode, PITCH_TAG.c_str(), pitches, 2);
  std::vector<double> pitchesInV = { pitches[0], pitches[1] };
  assembly.setFloatProperty("pitches", pitchesInV);

  double degree(0);
  r &= read(assyNode, ROTATE_TAG.c_str(), degree);
  assembly.setIntegerProperty("z axis", degree);

  std::string ductName;
  smtk::model::EntityRef duct;
  r &= read(assyNode, DUCT_TAG.c_str(), ductName);
  if (nameToDuct.find(ductName) == nameToDuct.end())
  {
    smtkErrorMacro(smtk::io::Logger(), "Assembly " << name << " does not have "
                                                              "a valid duct, skipping it.");
    return false;
  }
  else
  {
    duct = nameToDuct[ductName];
  }

  assembly.setStringProperty("associated duct", duct.entity().toString());
  smtk::model::FloatList ductCoords = { 0, 0, 0 };
  entsAndCoords[duct] = ductCoords;

  // Layout and lattice size
  pugi::xml_node latticeNode = assyNode.child(LATTICE_TAG.c_str());
  // pinLabel -> [x,y,x,y,x,y] where x y is the index in the rect schema planner or
  // ring and layer/x and y in the hex/rect schema planner
  LabelToLayout pinLabelsToLayouts;
  r &= ReadRXFFileHelper::parseLattice(latticeNode, pinLabelsToLayouts, assembly);

  // Convert labelToLayout into pins and their coordinates
  // Copy the logic from smtkRGGEditAssemblyView::apply function
  bool isHex = assembly.owningModel().integerProperty("hex")[0];
  double thickness0(std::numeric_limits<double>::max()),
    thickness1(std::numeric_limits<double>::max());
  calculateDuctMinimimThickness(duct, thickness0, thickness1);
  std::vector<double> spacing = { 0, 0 };
  double baseX, baseY;
  if (!isHex)
  { // Use the cartesian coordinate where the starting point is located
    // at left bottom
    spacing[0] = pitches[0];
    spacing[1] = pitches[1];
    baseX = -1 * thickness0 / 2 + spacing[0] / 2;
    baseY = -1 * thickness1 / 2 + spacing[0] / 2;
  }
  else
  { // Spacing is the allowable max distance between two adjacent pin centers
    // Use the cartesian coordinate where the starting point is located at
    // the origin point.
    spacing[0] = spacing[1] = pitches[0];
    baseX = baseY = 0.0; // Ignored by calculateHexPinCoordinate for now
  }

  smtk::model::StringList pinIds;
  for (auto& pLIter : pinLabelsToLayouts)
  {
    std::string cL = pLIter.first;
    if (labelToPin.find(cL) == labelToPin.end())
    { // Skip a pin that does not exist. Ideally it should only be XX which
      // is a placeholder for nothing
      continue;
    }
    smtk::model::EntityRef pin = labelToPin[cL];
    std::string pinId = pin.entity().toString();
    pinIds.push_back(pinId);
    smtk::model::IntegerList layout = pLIter.second;
    smtk::model::FloatList coordinates;
    size_t numberOfPairs = layout.size() / 2;
    coordinates.reserve(numberOfPairs * 3);
    for (size_t index = 0; index < numberOfPairs; index++)
    {
      double x, y;
      if (isHex)
      {
        calculateHexPinCoordinate(x, y, spacing[0], layout[2 * index], layout[2 * index + 1]);
      }
      else
      { // Question
        // In schema planner, x and y axis are exchanged. Here we just follow the traditional coordinate convension
        x = baseX + spacing[0] * layout[2 * index];
        y = baseY + spacing[1] * layout[2 * index + 1];
      }
      coordinates.push_back(x);
      coordinates.push_back(y);
      coordinates.push_back(0);
    }
    assembly.setIntegerProperty(pinId, layout);
    assembly.setFloatProperty(pinId, coordinates);
    // Add the pin into entsAndCoords so that we can glyph it
    entsAndCoords[pin] = coordinates;
  }
  assembly.setStringProperty("pins", pinIds);
  // Glyph the duct and pins
  ReadRXFFileHelper::createInstances(entsAndCoords, assembly, newAssyInstances);

  // TODO: Add support for unknown attribute
  if (!r)
  {
    smtkErrorMacro(smtk::io::Logger(), "Encounter errors when paring assembly " << name);
  }
  return r;
}

bool ReadRXFFileHelper::parseLattice(
  pugi::xml_node latticeNode, LabelToLayout& lTG, smtk::model::Group target)
{
  bool r(true), isCore(false);
  if (target.hasStringProperty("rggType") &&
    target.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_CORE)
  {
    isCore = true;
  }
  // TODO: Support type
  unsigned int type;
  r &= read(latticeNode, TYPE_TAG.c_str(), type);
  int subtype;
  r &= read(latticeNode, SUB_TYPE_TAG.c_str(), subtype);

  std::vector<std::string> labelsPerRing;
  r &= read(latticeNode, GRID_TAG.c_str(), labelsPerRing);
  int j(0);
  for (int i = 0; i < labelsPerRing.size(); i++)
  { // Per ring
    std::string labels = labelsPerRing[i];
    regex re(",");
    sregex_token_iterator it(labels.begin(), labels.end(), re, -1), last;
    for (j = 0; it != last; ++it, j++)
    { // per location
      std::string currentLabel = it->str();
      if (lTG.find(currentLabel) != lTG.end())
      {
        lTG[currentLabel].push_back(static_cast<long>(i));
        lTG[currentLabel].push_back(static_cast<long>(j));
      }
      else
      {
        std::vector<long> initialLayout = { i, j };
        lTG[currentLabel] = initialLayout;
      }
    }
  }
  bool isHex = target.owningModel().integerProperty("hex")[0];
  smtk::model::IntegerList latticeSize;
  if (isHex)
  {
    latticeSize.push_back(labelsPerRing.size());
    latticeSize.push_back(labelsPerRing.size());
  }
  else
  { //TODO: Check if the right order is used here
    latticeSize.push_back(j);
    latticeSize.push_back(labelsPerRing.size());
  }
  if (isCore)
  { // Core group is just a place holder. All infos are stored in the model
    target.owningModel().setIntegerProperty("lattice size", latticeSize);
  }
  else
  {
    target.setIntegerProperty("lattice size", latticeSize);
  }
  return r;
}

void ReadRXFFileHelper::createInstances(const EntsAndCoords& entsAndCoords,
  smtk::model::Group& target, smtk::model::EntityRefArray& container)
{
  bool isCore(false);
  if (target.hasStringProperty("rggType") &&
    target.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_CORE)
  {
    isCore = true;
  }
  smtk::model::ManagerPtr mgr = target.manager();
  for (auto eIter : entsAndCoords)
  {
    smtk::model::EntityRef ent = eIter.first;
    std::vector<double> coordinates = eIter.second;
    smtk::model::AuxiliaryGeometry aux = ent.as<smtk::model::AuxiliaryGeometry>();
    smtk::model::AuxiliaryGeometries children = aux.auxiliaryGeometries();
    // Each sub part should be glyphed
    for (size_t i = 0; i < children.size(); i++)
    {
      smtk::model::EntityRef prototype =
        smtk::model::EntityRef(children[i].manager(), children[i].entity());
      Instance instance = mgr->addInstance(prototype);
      instance.setColor(prototype.color());
      std::string iName = "instance_Of_" + instance.prototype().name();
      instance.setName(iName);
      target.addEntity(instance);
      container.push_back(instance);
      instance.setRule("tabular");
      instance.setFloatProperty("placements", coordinates);
      if (!isCore)
      { // Core related instances should be visible by default
        instance.setVisible(false);
      }
    }
  }
}

} // namespace rgg
} //namespace bridge
} // namespace smtk
