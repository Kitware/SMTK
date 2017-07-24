//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkTerrainExtractionFilter -
// .SECTION Description
// This atypical filter wraps the (mostly) vxl functionality of cmbTokenRefine
// and cmbTerrainExtract (CL tools).  The filter executes in 3 "phases"
//   1. Setup Refine - converts points in Input to internal vxl structure.
//      After executing this phase, the input can be discarded (the filter no
//      longer needs an input).  This is also when the DataTransform is
//      calculated (for more efficient calculation, but only if
//      ComputeDataTransform is true) and when the initial scale is
//      caluclated from the input points (based on distance to closest point
//      of every point) if ComputeInitialScale is true.
//   2. Refine - along with the previous step, this phase accomplishes what
//      cmbTokenRefine preiously did.  The "filter" won't necessarily
//      generate any output, as the primary purpose of this filter is to
//      generate the input (rtvl/vxl) for the 3rd and final phase.  However,
//      vtkPolyData (just points / vertices ) output can be generated to
//      visualize the result of the Refine phase.  The output can either
//      be written to disk or can be available as a vtkMultiBlock structure,
//      where each vtkPolyData block is a refine level (controlled via
//      RefineVisualizationOutputMode flag).
//   3. Extract - the 3rd and final phase implements the functionality of
//      cmbTerrainExtract, taking the intermediate Refine result and
//      generating multiple "terrain" levels (as specified by the
//      MinExtractLevel / MaxExtractLevel variables).  The results can either
//      be written to disk or will be available as a vtkMultiBlock output,
//      controlled via WriteExtractionResultsToDisk flag.
//
// The result of the Refine phase is not released until SetupRefine is
// reexecuted, so multiple Extract combinations (min/max level) can be executed.
// Note that intermediate computation reuslt MAY be written to disk, and NOT
// maintained in memory (controlled via CacheRefineResultsToDisk).

// .SECTION Caveats
// .SECTION See Also

#ifndef __vtkTerrainExtractionFilter_h
#define __vtkTerrainExtractionFilter_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkSmartPointer.h"

#define VTK_MODE_SETUP_REFINE 0
#define VTK_MODE_REFINE 1
#define VTK_MODE_EXTRACT 2

#define VTK_OUTPUT_TYPE_XML_PD 0
#define VTK_OUTPUT_TYPE_ASCII_PTS 1
#define VTK_OUTPUT_TYPE_BINARY_PTS 2

#define VTK_REFINE_VIZ_OUTPUT_TO_DISK 0
#define VTK_REFINE_VIZ_OUTPUT_TO_OUTPUT_PORT 1
#define VTK_REFINE_VIZ_OUTPUT_OFF 2

class vtkDoubleArray;
class vtkTerrainExtractionInternal;
class vtkPoints;
class vtkPolyData;
class vtkTransform;

class VTKCMBFILTERING_EXPORT vtkTerrainExtractionFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkTerrainExtractionFilter, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkTerrainExtractionFilter* New();

  // Description:
  // Control which of the three modes (phases) to execute: refine and (then) extract
  vtkSetClampMacro(ExecuteMode, int, VTK_MODE_SETUP_REFINE, VTK_MODE_EXTRACT);
  vtkGetMacro(ExecuteMode, int);
  void SetExecuteModeToSetupRefine() { this->SetExecuteMode(VTK_MODE_SETUP_REFINE); };
  void SetExecuteModeToRefine() { this->SetExecuteMode(VTK_MODE_REFINE); };
  void SetExecuteModeToExtract() { this->SetExecuteMode(VTK_MODE_EXTRACT); };
  const char* GetExecuteModeAsString();

  // Description:
  // Visualization output from Refine phase can be written to disk, available
  // on the OutputPort as vtkMultiBlock of vtkPolyData blocks, or not generated
  // at all.  If written to disk, the format of the output files is specifed
  // by the OutputPtsFormat variable.
  vtkSetClampMacro(
    RefineVisualizationOutputMode, int, VTK_REFINE_VIZ_OUTPUT_TO_DISK, VTK_REFINE_VIZ_OUTPUT_OFF);
  vtkGetMacro(RefineVisualizationOutputMode, int);
  void SetRefineVisualizationOutputModeToDisk()
  {
    this->SetRefineVisualizationOutputMode(VTK_REFINE_VIZ_OUTPUT_TO_DISK);
  };
  void SetRefineVisualizationOutputModeToOutputPort()
  {
    this->SetRefineVisualizationOutputMode(VTK_REFINE_VIZ_OUTPUT_TO_OUTPUT_PORT);
  };
  void SetRefineVisualizationOutputModeToOff()
  {
    this->SetRefineVisualizationOutputMode(VTK_REFINE_VIZ_OUTPUT_OFF);
  };
  const char* GetRefineVisualizationOutputModeAsString();

  // Description:
  // Set/Get whether the computation results from the Refine phase are written
  // to disk or stored in memory.  If written to disk, this flag should still
  // be true when exectuing the Extract phase (and IntermediateResultsPath and
  // OutputBaseFileName should not be changed).
  vtkBooleanMacro(CacheRefineResultsToDisk, bool);
  vtkSetMacro(CacheRefineResultsToDisk, bool);
  vtkGetMacro(CacheRefineResultsToDisk, bool);

  // Description:
  // Set/Get whether results from Extract phase are written to disk (format of
  // the output files is specifed by the OutputPtsFormat variable) or available
  // via the output port as vtkMultiBlock with vtkPolyData leaves.
  vtkBooleanMacro(WriteExtractionResultsToDisk, bool);
  vtkSetMacro(WriteExtractionResultsToDisk, bool);
  vtkGetMacro(WriteExtractionResultsToDisk, bool);

  // Description:
  // Directory where any temporary results as well as visual result of refine
  // (if selected) are placed.  Be warned, do NOT change this between Refine
  // and Extract steps.
  vtkSetStringMacro(IntermediateResultsPath);
  vtkGetStringMacro(IntermediateResultsPath);

  // Description:
  // Set/Get the directory where Extract result files are written to disk.
  vtkSetStringMacro(OutputPath);
  vtkGetStringMacro(OutputPath);

  // Description:
  // Set/Get the base file name for any files written to disk.  Do not change
  // between Refine and Extract steps!
  vtkSetStringMacro(OutputBaseFileName);
  vtkGetStringMacro(OutputBaseFileName);

  // Description:
  // Control which of the output formats to use when writing points to a file
  vtkSetClampMacro(OutputPtsFormat, int, VTK_OUTPUT_TYPE_XML_PD, VTK_OUTPUT_TYPE_BINARY_PTS);
  vtkGetMacro(OutputPtsFormat, int);
  void SetOutputPtsFormatToXMLPolyData() { this->SetOutputPtsFormat(VTK_OUTPUT_TYPE_XML_PD); };
  void SetOutputPtsFormatToASCIIPts() { this->SetOutputPtsFormat(VTK_OUTPUT_TYPE_ASCII_PTS); };
  void SetOutputPtsFormatToBinaryPts() { this->SetOutputPtsFormat(VTK_OUTPUT_TYPE_BINARY_PTS); };
  const char* GetOutputPtsFormatAsString();

  // Description:
  // Get the # of levels generated by the Refine phase
  vtkGetMacro(NumberOfLevels, int);

  // Description:
  // Set/Get the maximum extract level.
  vtkSetClampMacro(MaxExtractLevel, int, 0, VTK_INT_MAX);
  vtkGetMacro(MaxExtractLevel, int);

  // Description:
  // Set/Get the maximum extract level.
  vtkSetClampMacro(MinExtractLevel, int, 0, VTK_INT_MAX);
  vtkGetMacro(MinExtractLevel, int);

  // Description:
  // Set/Get the InputBounds, which is needed by the Extract phase. It is
  // automatically set during the "Setup Refine" phase, so usually not
  // necessary to set.
  vtkSetVector6Macro(InputBounds, double);
  vtkGetVector6Macro(InputBounds, double);

  // Description:
  // Set/Get whether to compute an InitialScale during the SetupRefine
  // phase from the input points.  Doing so will increase the time required
  // by the SetupRefine phase, as the closest point to every input point
  // is calculated.
  // Note: If false, the InitialScale is still computed, but as 0.5% of
  // smallest of X and Y length (of bounds).
  vtkBooleanMacro(ComputeInitialScale, bool);
  vtkSetMacro(ComputeInitialScale, bool);
  vtkGetMacro(ComputeInitialScale, bool);

  // Description:
  // Set/Get the Initial scale (which ultimately is the highest resolution
  // of the extraction).  If want to use value other than calculated
  // internally (see ComputeInitialScale), must be set between the SetupRefine
  // and Refine phases.
  //vtkSetClampMacro(InitialScale, double, 0, VTK_FLOAT_MAX);
  vtkSetMacro(InitialScale, double);
  vtkGetMacro(InitialScale, double);

  // Description:
  // Set/Get whether or not to update the internal transformation of the
  // data (if not updating, will use previous transform, which is initially
  // identity).
  vtkBooleanMacro(ComputeDataTransform, bool);
  vtkSetMacro(ComputeDataTransform, bool);
  vtkGetMacro(ComputeDataTransform, bool);

  // Description:
  // Set/Get the DataTransform to be used during the Setup Refine phase
  // and (the inverse) during producing refine or extraction results.  Results
  // will be unexpected if the transform is changed after the Setup Refine
  // phase.
  void SetDataTransform(double elements[16]);
  void SetDataTransform(vtkTransform* transform);
  vtkGetVectorMacro(DataTransform, double, 16);
  void GetDataTransform(vtkTransform* transform);

  // Description:
  // Set/Get whether to use PointLocator to find nearest point in input dataset
  // for purpose of setting color and intensity of the output point (only if
  // present on the input).
  vtkBooleanMacro(DetermineIntensityAndColor, bool);
  vtkSetMacro(DetermineIntensityAndColor, bool);
  vtkGetMacro(DetermineIntensityAndColor, bool);

  // Description:
  // Set/Get whether to use a custom Mask Size when refining.
  // Valid range is 0 - 1
  // At 0, all samples are used thus more robust to noise (but slower),
  //while increasing towards 1 tends towards a more even sampling
  //(faster, but may not work well if the data is anisotropic)
  vtkSetClampMacro(MaskSize, double, 0.0, 1.0);
  vtkGetMacro(MaskSize, double);

  //BTX
protected:
  vtkTerrainExtractionFilter();
  ~vtkTerrainExtractionFilter() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int FillInputPortInformation(int, vtkInformation*) override;

private:
  vtkTerrainExtractionFilter(const vtkTerrainExtractionFilter&); // Not implemented.
  void operator=(const vtkTerrainExtractionFilter&);             // Not implemented.

  void TokenRefineAnalyze(const char* baseFileName, vtkMultiBlockDataSet* output);
  bool TerrainExtract(const char* baseIntermediateFileName, vtkMultiBlockDataSet* output);
  void AppendOutputs();

  int DetermineStartingSplitLevel(unsigned int maxMemoryMB);

  vtkTerrainExtractionInternal* Internal;

  bool CacheRefineResultsToDisk;
  bool WriteExtractionResultsToDisk;
  int RefineVisualizationOutputMode;
  char* OutputPath;
  char* IntermediateResultsPath;
  char* OutputBaseFileName;
  int OutputPtsFormat;
  int ExecuteMode;
  int MaxExtractLevel;
  int MinExtractLevel;
  int NumberOfLevels;

  bool ComputeDataTransform;
  bool ComputeInitialScale;
  double InitialScale;

  double MaskSize;

  bool DetermineIntensityAndColor;

  double InputBounds[6];
  double DataTransform[16];
  //ETX
};

inline const char* vtkTerrainExtractionFilter::GetExecuteModeAsString(void)
{
  if (this->ExecuteMode == VTK_MODE_SETUP_REFINE)
  {
    return "Setup Refine";
  }
  else if (this->ExecuteMode == VTK_MODE_REFINE)
  {
    return "Refine";
  }
  else if (this->ExecuteMode == VTK_MODE_EXTRACT)
  {
    return "Extract";
  }
  return "Unknown";
}

inline const char* vtkTerrainExtractionFilter::GetOutputPtsFormatAsString(void)
{
  if (this->OutputPtsFormat == VTK_OUTPUT_TYPE_XML_PD)
  {
    return "XML PolyData";
  }
  else if (this->OutputPtsFormat == VTK_OUTPUT_TYPE_ASCII_PTS)
  {
    return "ASCII Pts";
  }
  else if (this->OutputPtsFormat == VTK_OUTPUT_TYPE_BINARY_PTS)
  {
    return "Binary Pts";
  }
  return "Unknown";
}

inline const char* vtkTerrainExtractionFilter::GetRefineVisualizationOutputModeAsString(void)
{
  if (this->RefineVisualizationOutputMode == VTK_REFINE_VIZ_OUTPUT_TO_DISK)
  {
    return "Disk";
  }
  else if (this->RefineVisualizationOutputMode == VTK_REFINE_VIZ_OUTPUT_TO_OUTPUT_PORT)
  {
    return "Output Port";
  }
  else if (this->RefineVisualizationOutputMode == VTK_REFINE_VIZ_OUTPUT_OFF)
  {
    return "Off";
  }
  return "Unknown";
}

#endif
