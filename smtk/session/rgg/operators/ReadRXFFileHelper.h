//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ReadRXFFileHelper - A helper class to read rxf files
// Cores are composed of a bunch of assemblies.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_session_rgg_ReadRXFFileHelper_h
#define __smtk_session_rgg_ReadRXFFileHelper_h

#include "smtk/session/rgg/Exports.h"

#include "smtk/PublicPointerDefs.h"

namespace pugi
{
class xml_node;
}
using StringToEnt = std::map<std::string, smtk::model::EntityRef>;
using LabelToLayout = std::map<std::string, std::vector<long> >;
using EntsAndCoords = std::map<smtk::model::EntityRef, std::vector<double> >;

namespace smtk
{
namespace session
{
namespace rgg
{

//TAGS
namespace XMLAttribute
{
const std::string CORE_TAG = "NuclearCore";
const std::string MATERIALS_TAG = "Materials";
const std::string MATERIAL_TAG = "Material";
const std::string DESCRIPTION_TAG = "Description";
const std::string NAME_TAG = "Name";
const std::string LABEL_TAG = "Label";
const std::string COLOR_TAG = "Color";
const std::string DUCT_CELL_TAG = "DuctCell";
const std::string DUCT_LAYER_TAG = "DuctLayer";
const std::string LEGEND_COLOR_TAG = "LegendColor";
const std::string LOC_TAG = "Loc";
const std::string THICKNESS_TAG = "Thickness";
const std::string MATERIAL_LAYER_TAG = "MaterialLayer";
const std::string CYLINDER_TAG = "Cylinder";
const std::string FRUSTRUM_TAG = "Frustrum";
const std::string RADIUS_TAG = "Radius";
const std::string TYPE_TAG = "Type";
const std::string SUB_TYPE_TAG = "SubType";
const std::string GRID_TAG = "Grid";
const std::string DUCT_TAG = "Duct";
const std::string GEOMETRY_TAG = "Geometry";
const std::string CENTER_PINS_TAG = "CenterPins";
const std::string PITCH_TAG = "Pitch";
const std::string VALUE_TAG = "Value";
const std::string AXIS_TAG = "Axis";
const std::string DIRECTION_TAG = "Direction";
const std::string PARAMETERS_TAG = "Parameters";
const std::string MOVE_TAG = "Move";
const std::string CENTER_TAG = "Center";
const std::string UNKNOWN_TAG = "Unknown";
const std::string DUCTS_TAG = "Ducts";
const std::string PINS_TAG = "Pins";
const std::string DEFAULTS_TAG = "Defaults";
const std::string PINCELL_TAG = "PinCell";
const std::string NEUMANN_VALUE_TAG = "NeumannValue";
const std::string LENGTH_TAG = "Length";
const std::string Z0_TAG = "Z0";
const std::string STR_TAG = "Str";
const std::string LATTICE_TAG = "Lattice";
const std::string SIDE_TAG = "Side";
const std::string ID_TAG = "Id";
const std::string EQUATION_TAG = "Equation";
const std::string SIZE_TAG = "Size";
const std::string DIVISIONS_TAG = "Divisions";
const std::string AXIAL_MESH_SIZE_TAG = "AxialMeshSize";
const std::string EDGE_INTERVAL_TAG = "EdgeInterval";
const std::string MESH_TYPE_TAG = "MeshType";
const std::string ASSEMBLY_TAG = "Assembly";
const std::string BACKGROUND_TAG = "Background";
const std::string MODE_TAG = "Mode";
const std::string CYLINDER_RADIUS_TAG = "CylinderRadius";
const std::string CYLINDER_OUTER_SPACING_TAG = "CylinderOuterSpacing";
const std::string BACKGROUND_FILENAME_TAG = "BackgroundFileName";
const std::string MESH_FILENAME_TAG = "MeshFileName";
const std::string ROTATE_TAG = "Rotate";
const std::string ASSEMBLY_LINK_TAG = "AssemblyAlternative";
const std::string BOUNDARY_LAYER_TAG = "BoundaryLayer";
const std::string BIAS_TAG = "Bias";
const std::string INTERVAL_TAG = "Interval";
const std::string BLANK_TAG = "Blank";
}

/**\brief Load a rxl file into rgg session
  */
class SMTKRGGSESSION_EXPORT ReadRXFFileHelper
{
public:
  static bool parseMaterial(const pugi::xml_node node, smtk::model::EntityRef model);
  // Since core properties(geometry type, z origin, height, thickness)
  // are stored in ducts, we handle it here
  static bool parseDuctsAndCoreProperty(
    const pugi::xml_node node, smtk::model::EntityRef model, smtk::model::EntityRefArray& newDucts);

  static bool parsePins(
    pugi::xml_node pinsNode, smtk::model::EntityRef model, smtk::model::EntityRefArray& newPins);

  // Newly created assemblies should be added into newAssys and the
  // corresponding instances should be added into newAssyInstances
  static bool parseAssemblies(pugi::xml_node rootNode, smtk::model::EntityRef model,
    smtk::model::EntityRefArray& newAssys, smtk::model::EntityRefArray& newAssyInstances,
    StringToEnt& labelToPin, StringToEnt& nameToDuct);

  static bool parseDefaults(const pugi::xml_node node, smtk::model::EntityRef model);

  static bool parseCore(pugi::xml_node rootNode, smtk::model::EntityRef model,
    smtk::model::EntityRefArray& newInstances, StringToEnt& labelToAssy);

protected:
  // Helper function for parseDuctsAndCoreProperty
  static bool parseDuctNodeAndCreate(
    pugi::xml_node ductNode, smtk::model::EntityRef model, smtk::model::EntityRefArray& newDucts);
  // Helper function for parsePins
  static bool parsePin(
    pugi::xml_node pinNode, smtk::model::EntityRef model, smtk::model::EntityRefArray& newPins);
  // Helper function for parseAssemblies
  static bool parseAssembly(pugi::xml_node assyNode, smtk::model::EntityRef model,
    smtk::model::EntityRefArray& newAssys, smtk::model::EntityRefArray& newAssyCoreInstances,
    StringToEnt& labelToPin, StringToEnt& nameToDuct);
  // Helper function to parse a lattice
  static bool parseLattice(
    pugi::xml_node latticeNode, LabelToLayout& lTG, smtk::model::Group assembly);

  // Using the entities in entsAndCoords to create instances which would be
  // added into the group target and pushed back into the array container
  static void createInstances(const EntsAndCoords& entsAndCoords, smtk::model::Group& target,
    smtk::model::EntityRefArray& container);
};

} // namespace rgg
} //namespace session
} // namespace smtk

#endif // __smtk_session_rgg_ReadRXLFileHelper_h
