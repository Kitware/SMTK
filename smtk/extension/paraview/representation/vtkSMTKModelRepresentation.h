//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef vtkSMTKModelRepresentation_h
#define vtkSMTKModelRepresentation_h

#include "smtk/extension/paraview/representation/Exports.h"

#include "vtkGeometryRepresentation.h"
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositePolyDataMapper2;
class vtkDataObject;
class vtkGlyph3DMapper;
class vtkMultiBlockDataSet;
class vtkSelection;

/**
 *  \brief Representation of an SMTK Model.
 *  Renders the outputs of vtkSMTKModelReader.
 *   - Port 0: Model entities
 *   - Port 1: Glyph prototypes
 *   - Port 2: Glyph points
 *
 *  vtkSMSMTKModelRepresentationProxy sets certain properties used as
 *  mapper inputs (GlyphPrototypes and GlyphPoints).
 */
class SMTKREPRESENTATIONPLUGIN_EXPORT vtkSMTKModelRepresentation : public vtkGeometryRepresentation
{
public:
  static vtkSMTKModelRepresentation* New();
  vtkTypeMacro(vtkSMTKModelRepresentation, vtkGeometryRepresentation);
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
   * \sa vtkGeometryRepresentation
   */
  void SetMapScalars(int val) override;

protected:
  vtkSMTKModelRepresentation();
  ~vtkSMTKModelRepresentation();

  int FillInputPortInformation(int port, vtkInformation* info) override;
  void SetupDefaults() override;
  void SetOutputExtent(vtkAlgorithmOutput* output, vtkInformation* inInfo);
  void ConfigureGlyphMapper(vtkGlyph3DMapper* mapper);

  void UpdateSelection(vtkMultiBlockDataSet* data);
  vtkDataObject* FindNode(vtkMultiBlockDataSet* data, const std::string& uuid);

  vtkSmartPointer<vtkCompositePolyDataMapper2> EntityMapper;
  vtkSmartPointer<vtkCompositePolyDataMapper2> SelectedEntityMapper;
  vtkSmartPointer<vtkGlyph3DMapper> GlyphMapper;
  vtkSmartPointer<vtkGlyph3DMapper> SelectedGlyphMapper;

  vtkSmartPointer<vtkActor> Entities;
  vtkSmartPointer<vtkActor> SelectedEntities;
  vtkSmartPointer<vtkActor> GlyphEntities;
  vtkSmartPointer<vtkActor> SelectedGlyphEntities;

  double SelectionColor[3] = { 1., 0., 0. };

private:
  vtkSMTKModelRepresentation(const vtkSMTKModelRepresentation&) = delete;
  void operator=(const vtkSMTKModelRepresentation&) = delete;
};

#endif
