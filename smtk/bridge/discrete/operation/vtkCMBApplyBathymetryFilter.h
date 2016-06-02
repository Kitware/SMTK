//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBApplyBathymetryFilter
// .SECTION Description

#ifndef __smtkdiscrete_vtkCMBApplyBathymetryFilter_h
#define __smtkdiscrete_vtkCMBApplyBathymetryFilter_h

#include "smtk/bridge/discrete/Exports.h" // For export macro
#include "vtkDataSetAlgorithm.h"

class vtkPoints;

class SMTKDISCRETESESSION_EXPORT vtkCMBApplyBathymetryFilter : public vtkDataSetAlgorithm
{
public:
  static vtkCMBApplyBathymetryFilter *New();
  vtkTypeMacro(vtkCMBApplyBathymetryFilter,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Set/Get the radius of the cone to use for elevation smoothing
  vtkSetMacro(ElevationRadius,double);
  vtkGetMacro(ElevationRadius,double);

  //Description:
  //Set/Get the highest z value which the input will be set to if
  // UseHighestZValue is set to true. Default is 0.0
  vtkSetMacro(HighestZValue,double);
  vtkGetMacro(HighestZValue,double);

  // Description:
  // If "on", the input points' highest z values will be set to HighestZValue
  // Default is OFF
  vtkBooleanMacro(UseHighestZValue, bool);
  vtkSetMacro(UseHighestZValue, bool);
  vtkGetMacro(UseHighestZValue, bool);

  //Description:
  //Set/Get the lowest z value which the input will be set to if
  // UseLowestZValue is set to true. Default is 0.0
  vtkSetMacro(LowestZValue,double);
  vtkGetMacro(LowestZValue,double);

  // Description:
  // If "on", the input points' lowest z values will be set to LowestZValue
  // Default is OFF;
  vtkBooleanMacro(UseLowestZValue, bool);
  vtkSetMacro(UseLowestZValue, bool);
  vtkGetMacro(UseLowestZValue, bool);

  //Description:
  //Set/Get the z value which the input will be set to if
  // FlattenZValues is set to true. Default is 0.0
  vtkSetMacro(FlatZValue,double);
  vtkGetMacro(FlatZValue,double);

  // Description:
  // If "on", the input points' z values will be set to FlatZValue
  vtkBooleanMacro(FlattenZValues, bool);
  vtkSetMacro(FlattenZValues, bool);
  vtkGetMacro(FlattenZValues, bool);

  // Description:
  // If "on", the output is simply a shallowcopy of input
  vtkBooleanMacro(NoOP, bool);
  vtkSetMacro(NoOP, bool);
  vtkGetMacro(NoOP, bool);

  //Description:
  //Remove all connections on port 0, dataset that will be altered
  //with bathymetry
  void RemoveInputConnections();

  //Description:
  //Remove all connection on port 1, point sources
  void RemoveSourceConnections();

protected:
  vtkCMBApplyBathymetryFilter();
  ~vtkCMBApplyBathymetryFilter();

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);


  //methods for flattening mesh
  bool FlattenMesh(vtkPoints*);

  //methods for apply bathymetry
  bool ApplyBathymetry(vtkPoints *points);

  double ElevationRadius;
  double HighestZValue;
  bool UseHighestZValue;
  double LowestZValue;
  bool UseLowestZValue;
  double FlatZValue;
  bool FlattenZValues;
  bool NoOP;

  class vtkCmbInternalTerrainInfo;
  vtkCmbInternalTerrainInfo *TerrainInfo;

private:
  vtkCMBApplyBathymetryFilter(const vtkCMBApplyBathymetryFilter&);  // Not implemented.
  void operator=(const vtkCMBApplyBathymetryFilter&);  // Not implemented.
};

#endif
