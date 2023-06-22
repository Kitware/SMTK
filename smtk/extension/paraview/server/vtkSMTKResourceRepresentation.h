//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_representation_vtkSMTKResourceRepresentation_h
#define smtk_extension_paraview_representation_vtkSMTKResourceRepresentation_h

#include <vtkProperty.h> // for VTK_POINTS etc.
#include <vtkSmartPointer.h>

#include <vtkPVDataRepresentation.h>

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/paraview/server/smtkPVServerExtModule.h"
#include "smtk/extension/vtk/filter/vtkApplyTransforms.h"
#include "smtk/view/SelectionObserver.h"

#include <array>
#include <map>
#include <string>
#include <unordered_map>

class vtkSMTKWrapper;

class vtkActor;
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
 *  \brief Representation of an SMTK Resource. Renders the outputs of
 *  vtkSMTKResourceReader.
 *
 *  Input data arrives as a multiblock with four blocks:
 *
 *  |       Block                  |    Mapper     |  Actor         |
 *  | :--------------------------- | :------------ | :------------- |
 *  |   Block 0: Components        |  EntityMapper | Entities       |
 *  |   Block 1: Glyph prototypes  |  GlyphMapper  | GlyphEntities  |
 *  |   Block 2: Glyph points      |  GlyphMapper  | GlyphEntities  |
 *  |   Block 3: Images            |  Internal slice representation |
 *
 *  vtkSMSMTKResourceRepresentationProxy sets certain properties used as mapper
 *  inputs (GlyphPrototypes and GlyphPoints).
 *
 *  Each of the mappers has a selection counterpart (SelectedEntityMapper and
 *  SelectedGlyphMapper) which renders only selected entities.
 *
 *  The representation supports different coloring modes (ColorBy):
 *
 *    - ENTITY: Coloring through block attributes (vtkCompositeDataDisplayAttributes)
 *      using the smtk::model::EntityRef color.
 *
 *    - VOLUME: Coloring through block attributes using the smtk::model::EntityRef
 *      color corresponding to the 0th volume bounded by a given entity (if any).
 *
 *    - FIELD: Maps field scalars (or not) through the color LUT. This is the regular
 *      behavior in ParaView's vtkGeometryRepresentation. Properties under the
 *      PropertyGroup "Scalar Coloring" have effect only when this mode is active.
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
 *  \sa vtkSMSMTKResourceRepresentationProxy vtkCompositeDataDisplayAttributes
 */
class SMTKPVSERVEREXT_EXPORT vtkSMTKResourceRepresentation : public vtkPVDataRepresentation
{
public:
  static vtkSMTKResourceRepresentation* New();
  vtkTypeMacro(vtkSMTKResourceRepresentation, vtkPVDataRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSMTKResourceRepresentation(const vtkSMTKResourceRepresentation&) = delete;
  vtkSMTKResourceRepresentation& operator=(const vtkSMTKResourceRepresentation&) = delete;

  /// The visual state of a single component
  struct State
  {
    int m_visibility;
  };

  /// A map from component UUIDs to user-provided state
  using ComponentStateMap = std::map<smtk::common::UUID, State>;
  /// A map from UUIDs to vtkDataObjects rendered by this representation (across all its actors)
  using RenderableDataMap = std::map<smtk::common::UUID, vtkDataObject*>;
  /// The type of a function used to update display attributes based on selections.
  using StyleFromSelectionFunction = std::function<
    bool(smtk::view::SelectionPtr, RenderableDataMap&, vtkSMTKResourceRepresentation*)>;

  //@{
  /**
   * \sa vtkPVDataRepresentation
   */
  int ProcessViewRequest(
    vtkInformationRequestKey* request_type,
    vtkInformation* inInfo,
    vtkInformation* outInfo) override;
  int RequestData(vtkInformation* info, vtkInformationVector** inVec, vtkInformationVector* outVec)
    override;
  bool AddToView(vtkView* view) override;
  bool RemoveFromView(vtkView* view) override;
  void SetVisibility(bool val) override;
  //@}

  /**
   * Resource rendering properties. Forwarded to the relevant vtkProperty instances.
   */
  void SetMapScalars(int val);
  void SetInterpolateScalarsBeforeMapping(int val);
  void SetPointSize(double val);
  void SetLineWidth(double val);
  void SetLineColor(double r, double g, double b);
  void SetLookupTable(vtkScalarsToColors* val);
  void SetEdgeVisibility(int val);
  void SetRenderLinesAsTubes(int val);
  void SetRenderPointsAsSpheres(int val);
  void SetVertexVisibility(int val);

  //@{
  /**
   * Selection properties. Forwarded to the relevant vtkProperty instances.
   */
  virtual void SetDefaultEdgeColor(double r, double g, double b);
  virtual void SetDefaultEdgeColor(const double* rgb);
  virtual double* GetDefaultEdgeColor();

  virtual void SetDefaultFaceColor(double r, double g, double b);
  virtual void SetDefaultFaceColor(const double* rgb);
  virtual double* GetDefaultFaceColor();

  vtkSetVector3Macro(SelectionColor, double);
  vtkGetVector3Macro(SelectionColor, double);
  vtkSetVector3Macro(HoverColor, double);
  vtkGetVector3Macro(HoverColor, double);
  void SetSelectionPointSize(double val);
  void SetSelectionLineWidth(double val);
  void SetSelectionRenderStyle(int style);
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

  vtkSetStringMacro(ActiveAssembly);
  vtkGetStringMacro(ActiveAssembly);
  //@}

  //@{
  /**
   * Block properties for tessellation entities (Block 0: Components).
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

  /*!\brief Selection/highlighting appearance.
   */
  //@{
  /// Apply the \a selectionValue to the given \a data.
  ///
  /// A \a selectionValue of -1 indicates that both the unselected and selected
  /// blocks corresponding to \a data should be hidden.
  //
  /// A \a selectionValue of 0 indicates the unselected block should be shown.
  //
  /// A \a selectionValue larger than 0 indicates the selected block should be
  /// shown and colored according to \a selectionValue.
  void SetSelectedState(vtkDataObject* data, int selectionValue, bool isGlyph);
  //@}

  /**
   * Block properties for instance placements (Block 2: Glyph Points).
   */
  //@{
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
    ENTITY = 0,
    VOLUME,
    FIELD,
  };

  vtkSetClampMacro(ColorBy, int, ENTITY, FIELD);
  vtkGetMacro(ColorBy, int);
  /**
   * Overload to set color mode using a string. Accepted strings are:
   * "Entity", "Volume" (and soon "Group").
   */
  void SetColorBy(const char* type);

  /// This method is used internally to notify the representation
  /// to rebbuild display properties in response to a change in the
  /// SMTK selection. You should never call it yourself.
  void SelectionModified();

  /**
   * Internal attributes are set through color block proxy properties.
   */
  void SetUseInternalAttributes(bool enable);
  //@}

  virtual void SetWrapper(vtkSMTKWrapper*);
  vtkGetObjectMacro(Wrapper, vtkSMTKWrapper);

  void SetResource(const smtk::resource::ResourcePtr& res);
  smtk::resource::ResourcePtr GetResource() const;

  void GetEntityVisibilities(std::map<smtk::common::UUID, int>& visdata);
  bool SetEntityVisibility(smtk::resource::PersistentObjectPtr ent, bool visible);

  /**\brief Look for generator functions that alters a representation's appearance based on a selection.
    *
    * This is used to update block display attributes (per-block color, visibility,
    * and opacity) on all of the representation's actors based on the active selection.
    * This function is not called from within a selection's observer callback;
    * instead it is called the next time the representation is about to be rendered.
    * (This way, representations which are not visible do no work maintaining visual properties.)
    *
    * New style function suppliers are registered with vtkSMTKRepresentationStyleSupplier.
    * If there are none, then the default handler is used.
    *
    * The default value of this function is vtkSMTKResourceRepresentation::ApplyDefaultStyle().
    */
  bool ApplyStyle(
    smtk::view::SelectionPtr seln,
    RenderableDataMap& renderables,
    vtkSMTKResourceRepresentation* rep);
  /// The default selection-style function
  static bool ApplyDefaultStyle(
    smtk::view::SelectionPtr seln,
    RenderableDataMap& renderables,
    vtkSMTKResourceRepresentation* rep);

  /// A helper used by ApplyDefaultStyle to handle a single component.
  bool SelectComponentFootprint(
    smtk::resource::PersistentObject* item,
    int selnBits,
    RenderableDataMap& renderables);

  /// A helper used by ApplyDefaultStyle to handle model entity components.
  bool SelectComponentFootprint(
    const smtk::model::EntityRefs& items,
    int selnBits,
    RenderableDataMap& renderables);

  /// Return the map from persistent-object UUID to user-specified state.
  ///
  /// This is read only. If you want to modify component state,
  /// call SetEntityVisibility().
  const ComponentStateMap& GetComponentState() const { return this->ComponentState; }

  /// Return the prop ID assigned to the actor that renders tessellated components.
  int GetEntitiesActorPickId() const { return this->EntitiesActorPickId; }

  /// Return the prop ID assigned to the actor that renders glyph components.
  int GetGlyphEntitiesActorPickId() const { return this->GlyphEntitiesActorPickId; }

  /// Return the prop ID assigned to the actor that renders selected tessellated components.
  int GetSelectedEntitiesActorPickId() const { return this->SelectedEntitiesActorPickId; }

  /// Return the prop ID assigned to the actor that renders selected glyph components.
  int GetSelectedGlyphEntitiesActorPickId() const { return this->SelectedGlyphEntitiesActorPickId; }

  /// Accessor for clients to use with vtkSMTKRepresentationStyleSupplier.
  vtkCompositeDataDisplayAttributes* GetEntityMapperDisplayAttributes();
  vtkCompositeDataDisplayAttributes* GetSelectedEntityMapperDisplayAttributes();

  /**\brief Fetch the representation's actors.
    *
    * These are used to configure pickers so that only geometry
    * associated with an SMTK resource can be picked.
    */
  vtkSmartPointer<vtkActor> GetEntitiesActor() { return this->Entities; }
  vtkSmartPointer<vtkActor> GetSelectedEntitiesActor() { return this->SelectedEntities; }
  vtkSmartPointer<vtkActor> GetGlyphEntitiesActor() { return this->GlyphEntities; }
  vtkSmartPointer<vtkActor> GetSelectedGlyphEntitiesActor() { return this->SelectedGlyphEntities; }

protected:
  vtkSMTKResourceRepresentation();
  ~vtkSMTKResourceRepresentation() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  void SetupDefaults();
  void SetOutputExtent(vtkAlgorithmOutput* output, vtkInformation* inInfo);
  void ConfigureGlyphMapper(vtkGlyph3DMapper* mapper);

  void UpdateRenderableData(vtkMultiBlockDataSet* modelData, vtkMultiBlockDataSet* instanceData);
  void UpdateDisplayAttributesFromSelection(
    vtkMultiBlockDataSet* modelData,
    vtkMultiBlockDataSet* instanceData);
  void UpdateSelection(
    vtkMultiBlockDataSet* data,
    vtkCompositeDataDisplayAttributes* blockAttr,
    vtkActor* actor);
  vtkDataObject* FindNode(vtkMultiBlockDataSet* data, const smtk::common::UUID& uuid);

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
   * Update the active coloring mode (field coloring, etc.).
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
  void ColorByVolume(vtkMultiBlockDataSet* data);
  void ColorByEntity(vtkMultiBlockDataSet* data);
  void ColorByField();

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
    vtkDataObject* dataObject,
    double bounds[6],
    vtkCompositeDataDisplayAttributes* cdAttributes);
  //@}

  /**\brief Provides access to the SMTK selection and to resource components.
    *
    * The selection is used to change the visual style of entities.
    * Additionally, resource and component relationships are used
    * + to determine the color for some non-visual entity types such as groups or models; and
    * + in certain coloring modes, such as when coloring by volume.
    */
  vtkSMTKWrapper* Wrapper{ nullptr };
  /// If Wrapper is non-null, SelectionObserver is the handle of an observer of Wrapper->GetSelection().
  smtk::view::SelectionObservers::Key SelectionObserver;
  /**
   * Provides access to entities in the model. This is useful when coloring by
   * certain modes (e.g. in order to query the color of a volume with a given UUID).
   */
  std::weak_ptr<smtk::resource::Resource> Resource;

  /// Map from component ids to their state (which is currently only visibility, but may be expanded)
  ComponentStateMap ComponentState;

  double DataBounds[6];
  int Representation = SURFACE;
  int ColorBy = FIELD;
  bool UpdateColorBy = false;
  bool UseInternalAttributes = false;

  vtkNew<vtkApplyTransforms> ApplyTransforms;
  vtkNew<vtkMultiBlockDataSet> CurrentData;
  vtkSmartPointer<vtkCompositePolyDataMapper2> EntityMapper;
  vtkSmartPointer<vtkCompositePolyDataMapper2> SelectedEntityMapper;

  vtkSmartPointer<vtkGlyph3DMapper> GlyphMapper;
  vtkSmartPointer<vtkGlyph3DMapper> SelectedGlyphMapper;

  vtkSmartPointer<vtkActor> Entities;
  vtkSmartPointer<vtkActor> SelectedEntities;
  vtkSmartPointer<vtkActor> GlyphEntities;
  vtkSmartPointer<vtkActor> SelectedGlyphEntities;

  // IDs assigned by vtkPVRenderView for hardware picking:
  int EntitiesActorPickId{ -1 };
  int SelectedEntitiesActorPickId{ -1 };
  int GlyphEntitiesActorPickId{ -1 };
  int SelectedGlyphEntitiesActorPickId{ -1 };

  /**
   * Rendering properties shared between Entities and GlyphEntities
   */
  vtkSmartPointer<vtkProperty> Property;
  double Ambient = 0.;
  double Diffuse = 1.;
  double Specular = 0.;

  double DefaultEdgeColor[3] = { 0., 0., 0. };
  double DefaultFaceColor[3] = { 1., 1., 1. };
  double SelectionColor[3] = { 1., 0.48235, 0.0 }; // { 1., 0.6, 1. };
  double HoverColor[3] = { 0.7, 0.483, 0.5873 };

  //@{
  /**
   * Block attributes for components.
   */
  bool BlockAttrChanged = false;
  vtkTimeStamp BlockAttributeTime;
  std::unordered_map<unsigned int, bool> BlockVisibilities;
  std::unordered_map<unsigned int, double> BlockOpacities;
  std::unordered_map<unsigned int, std::array<double, 3>> BlockColors;
  //@}

  //@{
  /**
   * Block attributes for instance points (GlyphPoints).
   */
  bool InstanceAttrChanged = false;
  vtkTimeStamp InstanceAttributeTime;
  std::unordered_map<unsigned int, bool> InstanceVisibilities;
  std::unordered_map<unsigned int, std::array<double, 3>> InstanceColors;
  //@}

  /// Hold a map of renderable objects whose style is altered by an SMTK selection.
  RenderableDataMap RenderableData;
  /// Timestamp for when RenderableData was last updated
  vtkTimeStamp RenderableTime;
  /// Timestamp for when the SMTK application Selection was last modified.
  vtkTimeStamp SelectionTime;
  /// Timestamp for when highlighting styles related to the selection were last applied.
  vtkTimeStamp ApplyStyleTime;

  /// The name of the active assembly.
  char* ActiveAssembly{ nullptr };
};

#endif
