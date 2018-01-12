//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_vtkSMTKModelRepresentation_h
#define smtk_extension_paraview_representation_vtkSMTKModelRepresentation_h
#include <unordered_map>

#include <vtkProperty.h> // for VTK_POINTS etc.
#include <vtkSmartPointer.h>

#include <vtkPVDataRepresentation.h>

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/paraview/server/Exports.h"

#include <array>
#include <string>
#include <unordered_map>

class vtkActor;
class vtkPVCacheKeeper;
class vtkCompositeDataSet;
class vtkCompositeDataDisplayAttributes;
class vtkCompositePolyDataMapper2;
class vtkDataObject;
class vtkGlyph3DMapper;
class vtkMapper;
class vtkMultiBlockDataSet;
class vtkScalarsToColors;
class vtkSelection;
class vtkTexture;

/**
 *  \brief Representation of an SMTK Model. Renders the outputs of
 *  vtkSMTKModelReader.
 *
 *  Input data arrives through 3 ports:
 *
 *          Input                      Mapper        Actor
 *    - Port 0: Model entities    |  EntityMapper | Entities
 *    - Port 1: Glyph prototypes  |  GlyphMapper  | GlyphEntities
 *    - Port 2: Glyph points      |  GlyphMapper  | GlyphEntities
 *
 *  vtkSMSMTKModelRepresentationProxy sets certain properties used as mapper
 *  inputs (GlyphPrototypes and GlyphPoints).
 *
 *  Each of the model mappers has a selection counterpart (SelectedEntityMapper
 *  and SelectedGlyphMapper) which renders only selected entities.
 *
 *  The representation supports different coloring modes (ColorBy):
 *
 *    - SCALARS: Maps scalars (or not) through the color LUT. This is the regular
 *      behavior in ParaView's vtkGeometryRepresentation. Properties under the
 *      PropertyGroup "Scalar Coloring" have effect only when this mode is active.
 *
 *    - ENTITY: Coloring through block attributes (vtkCompositeDataDisplayAttributes)
 *      using the smtk::model::EntityRef color.
 *
 *    - VOLUME: Coloring through block attributes using the smtk::model::EntityRef
 *      color corresponding to the 0th volume bounded by a given entity (if any).
 *
 *  And different representation subtypes:
 *
 *    - POINTS
 *    - WIREFRAME
 *    - SURFACE
 *    - SURFACE_WITH_EDGES
 *
 *  \note
 *  Additionally, block attributes can be set through ParaView's block inspector
 *  widget. Because block attributes in a mapper are referenced to each block by
 *  DataObject pointers and since DataObjects may change after updating the pipeline,
 *  this class maintains map members (Block* and Instance*) using flat-index as a key.
 *  ApplyEntityAttributes and ApplyGlyphBlockAttributes update the mapper's actual
 *  attributes with those cached in the maps. This is done after the data has updated
 *  (multi-block node pointers change after an update). Coloring using these internal
 *  attributes can be enabled/disabled through UseInternalAttributes.
 *
 *  \sa vtkSMSMTKModelRepresentationProxy vtkCompositeDataDisplayAttributes
 */
class SMTKPVSERVEREXTPLUGIN_EXPORT vtkSMTKModelRepresentation : public vtkPVDataRepresentation
{
public:
  static vtkSMTKModelRepresentation* New();
  vtkTypeMacro(vtkSMTKModelRepresentation, vtkPVDataRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * \sa vtkPVDataRepresentation
   */
  int ProcessViewRequest(vtkInformationRequestKey* request_type, vtkInformation* inInfo,
    vtkInformation* outInfo) override;
  int RequestData(
    vtkInformation* info, vtkInformationVector** inVec, vtkInformationVector* outVec) override;
  bool AddToView(vtkView* view) override;
  bool RemoveFromView(vtkView* view) override;
  void SetVisibility(bool val) override;
  //@}

  /**
   * Model rendering properties. Forwarded to the relevant vtkProperty instances.
   */
  void SetMapScalars(int val);
  void SetInterpolateScalarsBeforeMapping(int val);
  void SetPointSize(double val);
  void SetLineWidth(double val);
  void SetLineColor(double r, double g, double b);
  void SetLookupTable(vtkScalarsToColors* val);

  //@{
  /**
   * Selection properties. Forwarded to the relevant vtkProperty instances.
   */
  vtkSetVector3Macro(SelectionColor, double);
  vtkGetVector3Macro(SelectionColor, double);
  void SetSelectionPointSize(double val);
  void SetSelectionLineWidth(double val);
  void SetOpacity(double val);
  void SetPosition(double x, double y, double z);
  void SetScale(double x, double y, double z);
  void SetOrientation(double x, double y, double z);
  void SetOrigin(double x, double y, double z);
  void SetUserTransform(const double matrix[16]);
  void SetSpecularPower(double val);
  void SetSpecular(double val);
  void SetDiffuse(double val);
  void SetAmbient(double val);
  void SetPickable(int val);
  void SetTexture(vtkTexture* val);
  //@}

  //@{
  /**
   * Block properties for tessellation entities (Port 0: Model Entities).
   */
  void SetBlockVisibility(unsigned int index, bool visible);
  bool GetBlockVisibility(unsigned int index) const;
  void RemoveBlockVisibility(unsigned int index, bool = true);
  void RemoveBlockVisibilities();
  void SetBlockColor(unsigned int index, double r, double g, double b);
  void SetBlockColor(unsigned int index, double* color);
  double* GetBlockColor(unsigned int index);
  void RemoveBlockColor(unsigned int index);
  void RemoveBlockColors();
  void SetBlockOpacity(unsigned int index, double opacity);
  void SetBlockOpacity(unsigned int index, double* opacity);
  double GetBlockOpacity(unsigned int index);
  void RemoveBlockOpacity(unsigned int index);
  void RemoveBlockOpacities();
  //@}

  //@{
  /**
   * Block properties for instance placements (Port 2: Glyph Points).
   */
  void SetInstanceVisibility(unsigned int index, bool visible);
  bool GetInstanceVisibility(unsigned int index) const;
  void RemoveInstanceVisibility(unsigned int index, bool);
  void RemoveInstanceVisibilities();
  void SetInstanceColor(unsigned int index, double r, double g, double b);
  void SetInstanceColor(unsigned int index, double* color);
  double* GetInstanceColor(unsigned int index);
  void RemoveInstanceColor(unsigned int index);
  void RemoveInstanceColors();
  //@}

  //@{
  /**
   * Set the representation Subtype. This adds VTK_SURFACE_WITH_EDGES to those
   * defined in vtkProperty.
   */
  enum RepresentationTypes
  {
    POINTS = VTK_POINTS,
    WIREFRAME = VTK_WIREFRAME,
    SURFACE = VTK_SURFACE,
    SURFACE_WITH_EDGES = 3
  };

  vtkSetClampMacro(Representation, int, POINTS, SURFACE_WITH_EDGES);
  vtkGetMacro(Representation, int);
  /**
   * Overload to set representation type using string. Accepted strings are:
   * "Points", "Wireframe", "Surface" and "Surface With Edges".
   */
  void SetRepresentation(const char* type);
  //@}

  //@{
  /**
   * Set color-by mode.
   */
  enum ColorByType
  {
    SCALARS = 0,
    ENTITY,
    VOLUME
  };

  vtkSetClampMacro(ColorBy, int, SCALARS, VOLUME);
  vtkGetMacro(ColorBy, int);
  /**
   * Overload to set color mode using a string. Accepted strings are:
   * "Entity", "Volume" (and soon "Group").
   */
  void SetColorBy(const char* type);

  /**
   * Internal attributes are set through color block proxy properties.
   */
  void SetUseInternalAttributes(bool enable);
  //@}

  void SetResource(smtk::resource::ResourcePtr res);

  void GetEntityVisibilities(std::map<smtk::common::UUID, int>& visdata);
  bool SetEntityVisibility(smtk::model::EntityPtr ent, bool visible);

protected:
  vtkSMTKModelRepresentation();
  ~vtkSMTKModelRepresentation();

  int FillInputPortInformation(int port, vtkInformation* info) override;
  void SetupDefaults();
  void SetOutputExtent(vtkAlgorithmOutput* output, vtkInformation* inInfo);
  void ConfigureGlyphMapper(vtkGlyph3DMapper* mapper);

  void UpdateSelection(
    vtkMultiBlockDataSet* data, vtkCompositeDataDisplayAttributes* blockAttr, vtkActor* actor);
  vtkDataObject* FindNode(vtkMultiBlockDataSet* data, const std::string& uuid);

  /**
   * Clear the current selection stored in the mapper's
   * vtkCompositeDisplayDataAttributes. For vtkCompositePolyDataMapper2,
   * setting the top node as false is enough since the state of the top
   * node will stream down to its nodes.  Glyph3DMapper does not behave as
   * vtkCompositePolyDataMapper2, hence it is necessary to update the block
   * visibility of each node directly.
   */
  void ClearSelection(vtkMapper* mapper);

  /**
   * Update the active coloring mode (scalar coloring, etc.).
   */
  void UpdateColoringParameters(vtkDataObject* data);

  /**
   * Update the active representation subtype.
   */
  void UpdateRepresentationSubtype();

  /**
   * Entities bounding a volume entity (if any), use the color defined by
   * the bounded entity.  Currently uses only volume 0.
   */
  void ColorByVolume(vtkCompositeDataSet* data);
  void ColorByEntity(vtkMultiBlockDataSet* data);
  void ColorByScalars();

  //@{
  /**
   * Block attributes in a mapper are referenced to each block through DataObject
   * pointers. Since DataObjects may change after updating the pipeline, this class
   * maintains an additional map using the flat-index as a key.  This method updates
   * the mapper's attributes with those cached in this representation; This is done
   * after the data has updated (multi-block nodes change after an update).
   *
   * \sa vtkGeometryRepresentation
   */
  void ApplyInternalBlockAttributes();
  void ApplyEntityAttributes(vtkMapper* mapper);
  void ApplyGlyphBlockAttributes(vtkGlyph3DMapper* mapper);
  //@}

  //@{
  /**
   * Compute bounds of the input model taking into account instance placements
   * (and corresponding glyph offsets) and entity tessellation bounds.
   */
  bool GetModelBounds();
  bool GetEntityBounds(
    vtkDataObject* dataObject, double bounds[6], vtkCompositeDataDisplayAttributes* cdAttributes);
  //@}

  /**
   * Provides access to entities in the model. This is useful when coloring by
   * certain modes (e.g. in order to query the color of a volume with a given UUID).
   */
  smtk::resource::ResourcePtr Resource;

  double DataBounds[6];
  int Representation = SURFACE;
  int ColorBy = SCALARS;
  bool UpdateColorBy = false;
  bool UseInternalAttributes = false;

  vtkSmartPointer<vtkCompositePolyDataMapper2> EntityMapper;
  vtkSmartPointer<vtkCompositePolyDataMapper2> SelectedEntityMapper;
  vtkSmartPointer<vtkPVCacheKeeper> EntityCacheKeeper;

  vtkSmartPointer<vtkGlyph3DMapper> GlyphMapper;
  vtkSmartPointer<vtkGlyph3DMapper> SelectedGlyphMapper;

  vtkSmartPointer<vtkActor> Entities;
  vtkSmartPointer<vtkActor> SelectedEntities;
  vtkSmartPointer<vtkActor> GlyphEntities;
  vtkSmartPointer<vtkActor> SelectedGlyphEntities;

  /**
   * Rendering properties shared between Entities and GlyphEntities
   */
  vtkSmartPointer<vtkProperty> Property;
  double Ambient = 0.;
  double Diffuse = 1.;
  double Specular = 0.;

  double SelectionColor[3] = { 1., 0., 1. };

  //@{
  /**
   * Block attributes for model entities.
   */
  bool BlockAttrChanged = false;
  vtkTimeStamp BlockAttributeTime;
  std::unordered_map<unsigned int, bool> BlockVisibilities;
  std::unordered_map<unsigned int, double> BlockOpacities;
  std::unordered_map<unsigned int, std::array<double, 3> > BlockColors;
  //@}

  //@{
  /**
   * Block attributes for instance points (GlyphPoints).
   */
  bool InstanceAttrChanged = false;
  vtkTimeStamp InstanceAttributeTime;
  std::unordered_map<unsigned int, bool> InstanceVisibilities;
  std::unordered_map<unsigned int, std::array<double, 3> > InstanceColors;
  //@}

private:
  vtkSMTKModelRepresentation(const vtkSMTKModelRepresentation&) = delete;
  void operator=(const vtkSMTKModelRepresentation&) = delete;
};

#endif
