//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkTerrainExtractionFilter.h"
//
#include "vtkAppendPolyData.h"
#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiThreader.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkTimerLog.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"

//need to align the dataset on the z rotation
#include "vtkMath.h"
#include "vtkOBBTree.h"
#include "vtkTransform.h"

// refine
#include <rgtl/rgtl_serialize_ostream.hxx>
#include <rtvl/rtvl_refine.hxx>
#include <rtvl/rtvl_tokens.hxx>

#include <vcl_fstream.h>
#include <vcl_memory.h>
#include <vcl_string.h>

// extract
#include <rgtl/rgtl_serialize_istream.hxx>
#include <rtvl/rtvl_tensor.hxx>
#include <rtvl/rtvl_tensor_d.hxx>
#include <rtvl/rtvl_vote.hxx>
#include <rtvl/rtvl_votee.hxx>
#include <rtvl/rtvl_votee_d.hxx>
#include <rtvl/rtvl_voter.hxx>
#include <rtvl/rtvl_weight_smooth.hxx>

#include <rgtl/rgtl_object_array_points.hxx>
#include <rgtl/rgtl_octree_cell_bounds.hxx>
#include <rgtl/rgtl_octree_objects.hxx>

#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector_fixed.h>

#include <vcl_map.h>

#include "smtk/extension/vtk/io/vtkLIDARPtsWriter.h"
#include "smtk/extension/vtk/reader/vtkLIDARReader.h"
#include <vtksys/Glob.hxx>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkTerrainExtractionFilter);

class TerrainPoint
{
public:
  bool known;
  int level;
  double scale;
  double z;
  vnl_vector_fixed<double, 3> normal;
  TerrainPoint()
  {
    this->known = false;
    this->level = 0;
    this->scale = 0;
    this->z = 0;
    this->normal.fill(0.0);
  }
};

class TerrainLevelBlock
{
public:
  TerrainLevelBlock()
  {
    this->Offset[0] = this->Offset[1] = 0;
    this->NumberOfSubBlocks = 0;
    this->SplitLevel = 0;
    this->Ni = this->Nj = 0;
  };
  ~TerrainLevelBlock();

  unsigned int Ni;
  unsigned int Nj;
  unsigned int Offset[2];
  double Origin[2];
  double Spacing[2];
  double Bounds[4];
  int NumberOfSubBlocks;
  int SplitLevel;
  TerrainLevelBlock*(SubBlock[4]);
  vcl_vector<TerrainPoint> Terrain;
  TerrainPoint& GetPoint(unsigned int i, unsigned int j) { return this->Terrain[j * this->Ni + i]; }
  TerrainPoint& GetPoint(unsigned int index[2])
  {
    return this->Terrain[index[1] * this->Ni + index[0]];
  }
  TerrainPoint& GetPointWithOffset(unsigned int i, unsigned int j)
  {
    return this->Terrain[(j + this->Offset[1]) * this->Ni + i + this->Offset[0]];
  }
  TerrainPoint& GetPointWithOffset(unsigned int index[2])
  {
    return this->Terrain[(index[1] + this->Offset[1]) * this->Ni + index[0] + this->Offset[0]];
  }
  void Contribute(int i, int j, double w, double x, double y, double& z,
    vnl_vector_fixed<double, 3>& n, double& tw, int& level, double& scale)
  {
    if (i >= 0 && i < int(this->Ni) && j >= 0 && j < int(this->Nj))
    {
      TerrainPoint& tp = this->GetPoint(i, j);
      if (tp.known)
      {
        if (tp.normal[2] < 0.01)
        {
          vcl_cerr << "bad normal!!!" << vcl_endl << "  n = " << tp.normal << vcl_endl;
          return;
        }
        double px = this->Origin[0] + this->Spacing[0] * (i + this->Offset[0]);
        double py = this->Origin[1] + this->Spacing[1] * (j + this->Offset[1]);
        double pz = tp.z;
        double nx = tp.normal[0];
        double ny = tp.normal[1];
        double nz = tp.normal[2];
        // Contribute the height of this plane at the given (x,y) location.
        z += w * (pz + ((px - x) * nx + (py - y) * ny) / nz);
        n += w * tp.normal;
        scale += w * tp.scale;
        tw += w;
        if (tp.level > level)
        {
          level = tp.level;
        }
      }
    }
  }
};

TerrainLevelBlock::~TerrainLevelBlock()
{
  for (int i = 0; i < this->NumberOfSubBlocks; i++)
  {
    if (this->SubBlock[i])
    {
      delete this->SubBlock[i];
    }
  }
}

class vtkTerrainExtractionInternal
{
public:
  vtkTerrainExtractionInternal();
  ~vtkTerrainExtractionInternal();

  vcl_vector<rtvl_tokens<3> > CachedTokens;

  rtvl_refine<3>* Refine;
  rtvl_tokens<3> Tokens;
  unsigned int LevelIndex;
  typedef vcl_multimap<double, int> SegmentVotersType;

  struct ThreadSpecificData
  {
    SegmentVotersType SegmentVoters;
    vnl_vector_fixed<double, 3> LastNormal;
    unsigned int SegmentIJ[2];
    double SegmentXY[2];
    double SegmentRange[2];
    unsigned int PreviousClosestPt;
    vtkUnsignedCharArray* RGBScalars;
    vtkFloatArray* IntensityArray;
    int* PointMapping;
  };

  ThreadSpecificData ThreadData;

  double TerrainOrigin[2];

  void WritePoints(vcl_string fileName, int outputPtsFormat, vtkPolyData* outputPD);
  vtkPolyData* Visualize(vcl_string& outFileName, int outputPtsFormat, bool cacheToDisk,
    rtvl_tokens<3>& tokens, unsigned int level);
  void DetermineZRotation(vtkPolyData* data);
  bool TestRotatedBounds(vtkPolyData* data, double angle, double& bestArea);
  vtkTransform* Transform;
  vtkTransform* InverseTransform;
  vcl_vector<vcl_string> TemporaryFiles;
  void DeleteTemporaryFiles();

  unsigned int ComputeMemoryRequirement(
    int level, int minLevel, bool splitThisLevel, double size[2]);

  bool UpdateProgress(int currentLevel, double* bounds);
  bool TerrainExtractSubLevel(TerrainLevelBlock* prevLevelBlock, int extractLevel);

  void BuildLevelBlockTree(TerrainLevelBlock* levelBlockTree, int currentLevel, bool split);
  void SetupBlockExtents(TerrainLevelBlock* childBlock);

  void ExtractNextLevel(TerrainLevelBlock* levelBlock, TerrainLevelBlock* prevLevelBlock,
    unsigned int startRow, unsigned int endRow);
  void Extract2D(TerrainLevelBlock* levelBlock, TerrainLevelBlock* prevLevelBlock,
    vtkPoints* outPoints, vtkDoubleArray* outScales, rtvl_weight_smooth<3>& tvw);

  bool ExtractSegmentInit(TerrainLevelBlock* levelBlock, ThreadSpecificData& threadData,
    rgtl_octree_objects<2>& objects2D);
  void ExtractSegmentSearch(TerrainLevelBlock* levelBlock, ThreadSpecificData& threadData,
    vtkPoints* outPoints, vtkDoubleArray* outScales, rtvl_weight_smooth<3>& tvw);
  vtkPolyData* ExtractSave(TerrainLevelBlock* levelBlock, int extractLevel, bool levelSplit,
    vtkPoints* extractOutPoints, vtkDoubleArray* outScales);

  struct Location
  {
    double z;
    double saliency;
    double constraint;
  };

  bool ExtractSegmentVote(
    Location& loc, ThreadSpecificData& threadData, rtvl_weight_smooth<3>& tvw);
  bool ExtractSegmentRefine(TerrainLevelBlock* levelBlock, Location a, Location c,
    ThreadSpecificData& threadData, vtkPoints* outPoints, vtkDoubleArray* outScales,
    rtvl_weight_smooth<3>& tvw);
  bool ExtractSegmentLocalMax(TerrainLevelBlock* levelBlock, Location a, Location c,
    ThreadSpecificData& threadData, vtkPoints* outPoints, vtkDoubleArray* outScales,
    rtvl_weight_smooth<3>& tvw);

  int InitialExtractSplitLevel;
  vtkSmartPointer<vtkTimerLog> Timer;
  int MaximumLevelTime;
  double InputBounds[6];
  int MinExtractLevel;
  vtkSmartPointer<vtkPoints> OutPoints;
  vtkSmartPointer<vtkDoubleArray> OutScales;
  vtkTerrainExtractionFilter* Main;
  std::string BaseFileName;
  std::string OutputPath;
  int* OutputSplitCount;
  double* LevelScales;
  vcl_string* OutputFileNameBase;
  vtkUnsignedCharArray* InputRGBScalars;
  vtkUnsignedCharArray* RGBScalars;
  vtkFloatArray* InputIntensityArray;
  vtkFloatArray* IntensityArray;
  vtkPointLocator* PointLocator;
};

vtkTerrainExtractionInternal::vtkTerrainExtractionInternal()
{
  this->Transform = vtkTransform::New();
  this->InverseTransform = vtkTransform::New();
  this->Timer = vtkSmartPointer<vtkTimerLog>::New();
  this->MaximumLevelTime = 60;
  this->OutPoints = vtkSmartPointer<vtkPoints>::New();
  this->OutScales = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutScales->SetName("Scale");
  this->Refine = 0;
  this->OutputSplitCount = 0;
  this->LevelScales = 0;
  this->OutputFileNameBase = 0;
  this->RGBScalars = 0;
  this->IntensityArray = 0;
  this->PointLocator = 0;
}

vtkTerrainExtractionInternal::~vtkTerrainExtractionInternal()
{
  this->Transform->Delete();
  this->InverseTransform->Delete();
  this->DeleteTemporaryFiles();
  if (this->Refine)
  {
    delete this->Refine;
  }
  if (this->OutputSplitCount)
  {
    delete[] this->OutputSplitCount;
    delete[] this->OutputFileNameBase;
    delete[] this->LevelScales;
  }
  if (this->RGBScalars)
  {
    this->RGBScalars->Delete();
  }
  if (this->IntensityArray)
  {
    this->IntensityArray->Delete();
  }
  if (this->PointLocator)
  {
    this->PointLocator->Delete();
  }
}

void vtkTerrainExtractionInternal::DeleteTemporaryFiles()
{
  vcl_vector<vcl_string>::iterator tempFileIter = this->TemporaryFiles.begin();
  for (; tempFileIter != this->TemporaryFiles.end(); tempFileIter++)
  {
    vtksys::SystemTools::RemoveFile(tempFileIter->c_str());
  }
  this->TemporaryFiles.clear();
}

vtkTerrainExtractionFilter::vtkTerrainExtractionFilter()
{
  this->Internal = new vtkTerrainExtractionInternal();
  this->Internal->Main = this;

  this->ExecuteMode = VTK_MODE_REFINE;
  this->OutputPtsFormat = VTK_OUTPUT_TYPE_XML_PD;
  this->OutputPath = 0;
  this->IntermediateResultsPath = 0;
  this->OutputBaseFileName = 0;
  this->SetOutputBaseFileName("TerrainExtract");
  this->NumberOfLevels = 0;
  this->CacheRefineResultsToDisk = false;
  this->WriteExtractionResultsToDisk = false;
  this->RefineVisualizationOutputMode = VTK_REFINE_VIZ_OUTPUT_OFF;
  this->ComputeInitialScale = true;
  this->ComputeDataTransform = true;
  this->InitialScale = -1;
  this->DetermineIntensityAndColor = true;
  this->MaskSize = 1.0; //default pulled from rtvl_refine
}

vtkTerrainExtractionFilter::~vtkTerrainExtractionFilter()
{
  this->SetOutputPath(0);
  this->SetOutputBaseFileName(0);
  delete this->Internal;
}

void vtkTerrainExtractionFilter::SetDataTransform(double elements[16])
{
  memcpy(this->DataTransform, elements, sizeof(double) * 16);
  this->Internal->Transform->SetMatrix(elements);
  this->Internal->InverseTransform->DeepCopy(this->Internal->Transform);
  this->Internal->InverseTransform->Inverse();
}

void vtkTerrainExtractionFilter::SetDataTransform(vtkTransform* transform)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      this->DataTransform[i * 4 + j] = transform->GetMatrix()->GetElement(i, j);
    }
  }
  this->Internal->Transform->DeepCopy(transform);
  this->Internal->InverseTransform->DeepCopy(this->Internal->Transform);
  this->Internal->InverseTransform->Inverse();
}

void vtkTerrainExtractionFilter::GetDataTransform(vtkTransform* transform)
{
  transform->DeepCopy(this->Internal->Transform);
}

//
// Clip through data generating surface.
//
int vtkTerrainExtractionFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkMultiBlockDataSet* output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vcl_string baseIntermediateFileName;
  if (this->OutputPath)
  {
    this->Internal->OutputPath = this->OutputPath;
  }
  else
  {
    this->Internal->OutputPath = vtksys::SystemTools::GetCurrentWorkingDirectory();
  }
  this->Internal->OutputPath += "/";
  if (this->IntermediateResultsPath)
  {
    baseIntermediateFileName = this->IntermediateResultsPath;
    baseIntermediateFileName += "/";
  }
  else
  {
    baseIntermediateFileName = this->Internal->OutputPath;
  }
  if (this->OutputBaseFileName)
  {
    this->Internal->BaseFileName = this->OutputBaseFileName;
    baseIntermediateFileName += this->OutputBaseFileName;
  }
  else
  {
    this->Internal->BaseFileName = "TerrainExtract";
    baseIntermediateFileName += "TerrainExtract";
  }

  if (this->ExecuteMode == VTK_MODE_SETUP_REFINE)
  {
    // get the input and output
    vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPoints* inputPoints = input->GetPoints();
    vtkIdType n = inputPoints->GetNumberOfPoints();

    //determine the transform needed for each point
    if (this->ComputeDataTransform)
    {
      this->Internal->DetermineZRotation(input);
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          this->DataTransform[i * 4 + j] = this->Internal->Transform->GetMatrix()->GetElement(i, j);
        }
      }
    }
    vcl_vector<double> points(n * 3);
    int offset = 0;
    vtkBoundingBox bbox;
    for (vtkIdType i = 0; i < n; i++)
    {
      offset = i * 3;
      inputPoints->GetPoint(i, &points[offset]);
      this->Internal->Transform->TransformPoint(&points[offset], &points[offset]);
      bbox.AddPoint(&points[offset]);
    }

    bbox.GetBounds(this->InputBounds);

    if (this->Internal->Refine)
    {
      delete this->Internal->Refine;
    }
    this->Internal->Refine = new rtvl_refine<3>(n, &points[0], this->InputBounds);
    this->Internal->Refine->set_mask_size(this->MaskSize);
    if (this->ComputeInitialScale)
    {
      this->InitialScale = this->Internal->Refine->compute_scale();
    }
    else // choose .5% of shortest side (X or Y)
    {
      double xScale = (this->InputBounds[1] - this->InputBounds[0]) * 0.005;
      double yScale = (this->InputBounds[3] - this->InputBounds[2]) * 0.005;
      this->InitialScale = xScale < yScale ? xScale : yScale;
    }

    // setup PointLocator for determining intensity and color for output points
    // (if the info exists on the input); note, however, that may not end up using
    // if the user does not have DetermineIntensityAndColor == true
    vtkDataArray* scalars =
      input->GetPointData() ? input->GetPointData()->GetScalars("Color") : NULL;

    this->Internal->InputRGBScalars = scalars ? vtkUnsignedCharArray::SafeDownCast(scalars) : NULL;

    this->Internal->InputIntensityArray = input->GetPointData()
      ? vtkFloatArray::SafeDownCast(input->GetPointData()->GetArray("Intensity"))
      : NULL;

    if (this->Internal->InputRGBScalars)
    {
      if (!this->Internal->RGBScalars)
      {
        this->Internal->RGBScalars = vtkUnsignedCharArray::New();
        this->Internal->RGBScalars->SetName("Color");
        this->Internal->RGBScalars->SetNumberOfComponents(3);
      }
      this->Internal->RGBScalars->Reset();
    }
    else if (this->Internal->RGBScalars)
    {
      this->Internal->RGBScalars->Delete();
      this->Internal->RGBScalars = 0;
    }
    if (this->Internal->InputIntensityArray)
    {
      if (!this->Internal->IntensityArray)
      {
        this->Internal->IntensityArray = vtkFloatArray::New();
        this->Internal->IntensityArray->SetName("Intensity");
      }
      this->Internal->IntensityArray->Reset();
    }
    else if (this->Internal->IntensityArray)
    {
      this->Internal->IntensityArray->Delete();
      this->Internal->IntensityArray = 0;
    }
    // let's just be sure that we have no PointLocator unless we
    // specifically create one because the input has either
    // IntensityArray or RGBScalars
    if (this->Internal->PointLocator)
    {
      this->Internal->PointLocator->Delete();
      this->Internal->PointLocator = 0;
    }
    if (this->Internal->IntensityArray || this->Internal->RGBScalars)
    {
      this->Internal->PointLocator = vtkPointLocator::New();
      this->Internal->PointLocator->SetDataSet(input);
      double* inputBounds = input->GetBounds();
      double length[3] = { inputBounds[1] - inputBounds[0], inputBounds[3] - inputBounds[2],
        inputBounds[5] - inputBounds[4] };
      double totalLength = length[0] + length[1] + length[2];
      int totalDivisions = 50 * 50 * 50;
      int divisions[3];
      divisions[0] = int(vcl_floor(pow(totalDivisions, length[0] / totalLength)));
      divisions[1] = int(vcl_floor(pow(totalDivisions, length[1] / totalLength)));
      divisions[2] = int(vcl_floor(pow(totalDivisions, length[2] / totalLength)));
      this->Internal->PointLocator->SetDivisions(divisions);
    }
  }
  else if (this->ExecuteMode == VTK_MODE_REFINE)
  {
    this->NumberOfLevels = 0;
    this->Internal->Refine->set_scale(this->InitialScale);
    this->Internal->Refine->build_tree();
    this->TokenRefineAnalyze(baseIntermediateFileName.c_str(), output);
  }
  else // VTK_MODE_EXTRACT
  {
    if (this->MinExtractLevel >= this->NumberOfLevels)
    {
      vtkErrorMacro("Minimum extraction level is too large (invalid): " << this->MinExtractLevel);
      return 0;
    }
    if (this->MaxExtractLevel >= this->NumberOfLevels)
    {
      vtkErrorMacro("Maximum extraction level is too large (invalid): " << this->MaxExtractLevel);
      return 0;
    }
    if (this->MaxExtractLevel < this->MinExtractLevel)
    {
      vtkErrorMacro("Maximum extraction level is less than minimum level: "
        << this->MaxExtractLevel << " < " << this->MinExtractLevel);
      return 0;
    }

    // if DetermineIntensityAndColor is false then make sure we "clear"
    // objects used during the extraction for determining color and intensity
    if (!this->DetermineIntensityAndColor)
    {
      if (this->Internal->PointLocator)
      {
        this->Internal->PointLocator->Delete();
        this->Internal->PointLocator = 0;
      }
      if (this->Internal->IntensityArray)
      {
        this->Internal->IntensityArray->Delete();
        this->Internal->IntensityArray = 0;
      }
      if (this->Internal->RGBScalars)
      {
        this->Internal->RGBScalars->Delete();
        this->Internal->RGBScalars = 0;
      }
    }
    else if (this->Internal->PointLocator)
    {
      this->Internal->PointLocator->BuildLocator();
    }

    /*bool abort = */ this->TerrainExtract(baseIntermediateFileName.c_str(), output);
    delete this->Internal->Refine;
    this->Internal->Refine = 0;

    this->Internal->DeleteTemporaryFiles();
    this->AppendOutputs();

    delete[] this->Internal->OutputSplitCount;
    delete[] this->Internal->OutputFileNameBase;
    delete[] this->Internal->LevelScales;
    this->Internal->OutputSplitCount = 0;
    this->Internal->OutputFileNameBase = 0;
    this->Internal->LevelScales = 0;
  }

  return 1;
}

void vtkTerrainExtractionFilter::AppendOutputs()
{
  double maxPoints = 1e7;
  vcl_string fileExtension;
  vtkNew<vtkAppendPolyData> appendPolyData;
  if (this->GetOutputPtsFormat() == VTK_OUTPUT_TYPE_XML_PD)
  {
    fileExtension = ".vtp";
  }
  else
  {
    if (this->GetOutputPtsFormat() == VTK_OUTPUT_TYPE_BINARY_PTS)
    {
      fileExtension = ".bin";
    }
    fileExtension += ".pts"; //always append .pts, regardless of ASCII or Binary
  }

  vcl_string outputPath = vtksys::SystemTools::GetParentDirectory(
    vtksys::SystemTools::GetFilenamePath(this->Internal->OutputFileNameBase[0]).c_str());
  outputPath += "/";
  for (int level = this->MinExtractLevel; level <= this->MaxExtractLevel; level++)
  {
    if (this->Internal->OutputSplitCount[level] == 1)
    {
      break;
    }

    double scale = this->Internal->LevelScales[level];

    // how many total points... if all the output files for the level are written
    double totalPoints = ((this->InputBounds[1] - this->InputBounds[0]) / scale) *
      ((this->InputBounds[3] - this->InputBounds[2]) / scale);
    int splitDepth = 0;
    while (totalPoints > maxPoints)
    {
      splitDepth++;
      totalPoints /= (0x01 << splitDepth * 2);
    }

    int blockSize = this->Internal->OutputSplitCount[level] >> splitDepth;
    int outputCount = 0;

    char buf[64];
    sprintf(buf, "Appending outputs for Level %d", level);
    this->SetProgressText(buf);
    this->UpdateProgress(0);
    double blockRatio = 1.0 / (0x01 << (2 * splitDepth));
    // 50% of output block for reading (rest for append+write)
    double fileReadDelta = 0.5 * blockRatio / (blockSize * blockSize);

    for (int i = 0; i < this->Internal->OutputSplitCount[level]; i += blockSize)
    {
      for (int j = 0; j < this->Internal->OutputSplitCount[level]; j += blockSize)
      {
        vcl_string outputFileName = outputPath +
          vtksys::SystemTools::GetFilenameName(this->Internal->OutputFileNameBase[level]);
        appendPolyData->RemoveAllInputs();

        for (int m = 0; m < blockSize; m++)
        {
          for (int n = 0; n < blockSize; n++)
          {
            int index = (j + n) * this->Internal->OutputSplitCount[level] + (i + m);
            vcl_string fileName = this->Internal->OutputFileNameBase[level];
            sprintf(buf, "_%d", index);
            fileName += buf + fileExtension;
            if (vtksys::SystemTools::FileExists(fileName.c_str()))
            {
              this->Internal->TemporaryFiles.push_back(fileName);
              if (this->GetOutputPtsFormat() == VTK_OUTPUT_TYPE_XML_PD)
              {
                vtkNew<vtkXMLPolyDataReader> reader;
                reader->SetFileName(fileName.c_str());
                reader->Update();
                appendPolyData->AddInputConnection(reader->GetOutputPort());
              }
              else
              {
                vtkNew<vtkLIDARReader> reader;
                reader->SetFileName(fileName.c_str());
                reader->Update();
                appendPolyData->AddInputConnection(reader->GetOutputPort());
              }
            }
            this->UpdateProgress(this->GetProgress() + fileReadDelta);
          }
        }
        if (appendPolyData->GetNumberOfInputConnections(0) > 0)
        {
          if (splitDepth > 0)
          {
            sprintf(buf, "_%d", outputCount++);
            outputFileName += buf;
          }
          outputFileName += fileExtension;
          appendPolyData->Update();
          if (this->GetOutputPtsFormat() == VTK_OUTPUT_TYPE_XML_PD)
          {
            vtkNew<vtkXMLPolyDataWriter> writer;
            writer->ReleaseDataFlagOn();
            writer->SetFileName(outputFileName.c_str());
            writer->SetInputConnection(appendPolyData->GetOutputPort());
            writer->Write();
          }
          else
          {
            vtkNew<vtkLIDARPtsWriter> lidarWriter;
            lidarWriter->ReleaseDataFlagOn();
            lidarWriter->SetFileName(outputFileName.c_str());
            lidarWriter->AddInputConnection(appendPolyData->GetOutputPort());
            lidarWriter->Write();
          }
        }
        this->UpdateProgress(this->GetProgress() + 0.5 * blockRatio);
      }
    }

    this->Internal->DeleteTemporaryFiles(); // clear any files we've combined into a larger file
    // cleanup the directory we created for the "small" output files
    vcl_string originalOutputPath =
      vtksys::SystemTools::GetFilenamePath(this->Internal->OutputFileNameBase[level]);
    vcl_string globString = originalOutputPath + "/*";
    vtksys::Glob glob;
    glob.FindFiles(globString);
    if (glob.GetFiles().size() == 0)
    {
      vtksys::SystemTools::RemoveADirectory(originalOutputPath.c_str());
    }
  }
}

void vtkTerrainExtractionFilter::TokenRefineAnalyze(
  const char* baseFileName, vtkMultiBlockDataSet* /*output*/)
{
  bool haveLevel = true;
  unsigned int level;

  // delete temporary files, per chance we're doing another refine with the same object
  this->Internal->DeleteTemporaryFiles();
  unsigned int estimatedLevels = this->Internal->Refine->estimate_refine_levels();
  for (level = 0; haveLevel; level++)
  {
    char buf[32];
    sprintf(buf, "Refining Level %d of %d", level, estimatedLevels);
    this->SetProgressText(buf);

    // how many chunks are we going to do this level in, considering the
    // memory limitation we set

    // initialize_refine computes the # of chunks based on the memory
    // limit (in KB)... and perhaps a minimum # of chunks (since no feedback
    // regarding progress except on a chunk by chunk basis)
    int numberOfBlocks = this->Internal->Refine->initialize_refine_level(20, 2);
    // this used to occur in the "SETUP_REFINE" step during (at the end of) the
    // initialization of the refine structure

    for (int i = 0; i < numberOfBlocks; i++)
    {
      vcl_string tvlBaseFileName = baseFileName;
      sprintf(buf, "_%02u_%d", level, i);
      tvlBaseFileName += buf;
      int numOutputs = this->Internal->Refine->refine_next_block(tvlBaseFileName.c_str());
      for (int j = 0; j < numOutputs; j++)
      {
        sprintf(buf, "_%d.tvl", j);
        vcl_string tvlFileName = tvlBaseFileName + buf;
        this->Internal->TemporaryFiles.push_back(tvlFileName);
      }

      // just going to gues that there are roughly 15 levels... and
      // that each level is same amount of work (which is certainly not the case)
      this->UpdateProgress((i + 1.0) / numberOfBlocks);
      if (this->GetAbortExecute())
      {
        this->Internal->DeleteTemporaryFiles();
        delete this->Internal->Refine;
        this->Internal->Refine = 0;
        return;
      }
    }

    haveLevel = this->Internal->Refine->next_scale();
  }

  this->NumberOfLevels = level;
}

void vtkTerrainExtractionInternal::WritePoints(
  vcl_string fileName, int outputPtsFormat, vtkPolyData* outputPD)
{
  if (outputPtsFormat == VTK_OUTPUT_TYPE_XML_PD)
  {
    vtkNew<vtkXMLPolyDataWriter> writer;
    fileName += ".vtp";
    writer->SetFileName(fileName.c_str());
    writer->SetInputData(outputPD);
    writer->Write();
  }
  else
  {
    vtkNew<vtkLIDARPtsWriter> writer;
    if (outputPtsFormat == VTK_OUTPUT_TYPE_BINARY_PTS)
    {
      fileName += ".bin";
    }
    fileName += ".pts"; //always append .pts, regardless of ASCII or Binary
    writer->SetFileName(fileName.c_str());
    writer->SetInputData(outputPD);
    writer->Write();
  }
}

vtkPolyData* vtkTerrainExtractionInternal::Visualize(vcl_string& outFileName, int outputPtsFormat,
  bool cacheToDisk, rtvl_tokens<3>& tokens, unsigned int /*level*/)
{
  vtkIdType n = tokens.points.get_number_of_points();

  vtkPolyData* outPD = vtkPolyData::New();

  vtkNew<vtkPoints> outPoints;
  outPoints->SetNumberOfPoints(n);

  vtkNew<vtkCellArray> verts;

  vtkNew<vtkDoubleArray> outSurfaceness;
  outSurfaceness->SetName("TVSurfaceness");
  outSurfaceness->SetNumberOfTuples(n);

  outPD->SetPoints(outPoints.GetPointer());
  outPD->GetPointData()->SetScalars(outSurfaceness.GetPointer());
  outPD->SetVerts(verts.GetPointer());

  for (vtkIdType i = 0; i < n; i++)
  {
    double p[3];
    tokens.points.get_point(i, p);
    this->InverseTransform->TransformPoint(p, p);
    outPoints->SetPoint(i, p);
    verts->InsertNextCell(1, &i);

    rtvl_tensor<3> const& tensor = tokens.tokens[i];
    double surfaceness = tensor.saliency(0);
    outSurfaceness->SetTypedTuple(i, &surfaceness);
  }

  if (cacheToDisk)
  {
    this->WritePoints(outFileName, outputPtsFormat, outPD);
    outPD->Delete();
    return 0;
  }

  return outPD;
}

bool vtkTerrainExtractionInternal::TestRotatedBounds(
  vtkPolyData* data, double angle, double& bestArea)
{
  double center[3], point3D[3];
  data->GetCenter(center);
  this->Transform->Identity();
  this->Transform->Translate(center);
  this->Transform->RotateZ(angle);
  this->Transform->Translate(-center[0], -center[1], -center[2]);
  vtkBoundingBox bbox;

  for (vtkIdType i = 0; i < data->GetNumberOfPoints(); i++)
  {
    data->GetPoint(i, point3D);
    this->Transform->TransformPoint(point3D, point3D);
    bbox.AddPoint(point3D);
  }
  double newArea = bbox.GetLength(0) * bbox.GetLength(1);
  if (newArea < bestArea)
  {
    bestArea = newArea;
    return true;
  }
  return false;
}

void vtkTerrainExtractionInternal::DetermineZRotation(vtkPolyData* data)
{
  //data surface area
  double dataBounds[6];
  data->GetBounds(dataBounds);
  double bestArea = (dataBounds[1] - dataBounds[0]) * (dataBounds[3] - dataBounds[2]);
  double bestAngle = 0;

  bestAngle = this->TestRotatedBounds(data, -2.5, bestArea) ? -2.5 : bestAngle;
  bestAngle = this->TestRotatedBounds(data, 2.5, bestArea) ? 2.5 : bestAngle;
  if (bestAngle == 0)
  {
    this->Transform->Identity();
    //invert the transform for output now
    this->InverseTransform->Identity();
    return;
  }
  double angleDelta = bestAngle;
  for (int i = 0; i < 33; i++) // at most 85 degrees total (82.5 + original 2.5)
  {
    if (!this->TestRotatedBounds(data, bestAngle + angleDelta, bestArea))
    {
      break;
    }
    bestAngle += angleDelta;
  }

  // finally, take one more half-step in each direction
  bestAngle =
    this->TestRotatedBounds(data, bestAngle - 1.25, bestArea) ? bestAngle - 1.25 : bestAngle;
  bestAngle =
    this->TestRotatedBounds(data, bestAngle + 1.25, bestArea) ? bestAngle + 1.25 : bestAngle;

  double centre[3];
  data->GetCenter(centre);
  this->Transform->Identity();
  this->Transform->Translate(centre);
  this->Transform->RotateZ(bestAngle);
  this->Transform->Translate(-centre[0], -centre[1], -centre[2]);
  this->InverseTransform->DeepCopy(this->Transform);
  this->InverseTransform->Inverse();
}

bool vtkTerrainExtractionInternal::UpdateProgress(int currentLevel, double* bounds)
{
  double currentProgress = this->Main->GetProgress();
  double factor = (bounds[1] - bounds[0]) / (this->InputBounds[1] - this->InputBounds[0]);
  currentProgress += (factor * factor) / pow(2.0, currentLevel - this->MinExtractLevel + 1);
  this->Main->UpdateProgress(currentProgress);
  if (this->Main->GetAbortExecute())
  {
    return true;
  }
  return false;
}

bool vtkTerrainExtractionInternal::TerrainExtractSubLevel(
  TerrainLevelBlock* prevLevelBlock, int extractLevel)
{
  bool abort = false;

  if (extractLevel == this->InitialExtractSplitLevel)
  {
    this->BuildLevelBlockTree(prevLevelBlock, extractLevel, true);
  }

  double scale = this->LevelScales[extractLevel];
  rtvl_weight_smooth<3> tvw(scale);

  // Create the weight profile for this scale.
  if (extractLevel > this->InitialExtractSplitLevel)
  {
    TerrainLevelBlock* levelBlock = new TerrainLevelBlock;
    memcpy(levelBlock->Bounds, this->InputBounds, sizeof(double) * 4);
    levelBlock->Origin[0] = this->InputBounds[0];
    levelBlock->Origin[1] = this->InputBounds[2];

    double const factor = 1;
    levelBlock->Spacing[0] = levelBlock->Spacing[1] = scale * factor;
    levelBlock->Ni =
      1 + int(vcl_ceil((this->InputBounds[1] - this->InputBounds[0]) / levelBlock->Spacing[0]));
    levelBlock->Nj =
      1 + int(vcl_ceil((this->InputBounds[3] - this->InputBounds[2]) / levelBlock->Spacing[1]));

    this->Timer->StartTimer();

    // Allocate the terrain representation for this level.
    levelBlock->Terrain.resize(levelBlock->Ni * levelBlock->Nj);

    // get the tokens for this scale
    this->Refine->get_tokens(extractLevel, 0, this->Tokens);

    // DO THE WORK
    //push this all into threaded routine
    this->LevelIndex = extractLevel;
    this->Extract2D(levelBlock, prevLevelBlock, this->OutPoints, this->OutScales, tvw);
    if (prevLevelBlock)
    {
      prevLevelBlock->Terrain.clear();
    }

    this->Timer->StopTimer();
    // MaximumLevelTime / 2 becasue the next level will take about twice as long
    if (this->Timer->GetElapsedTime() > this->MaximumLevelTime / 2)
    {
      this->InitialExtractSplitLevel = extractLevel - 1;
    }

    this->ExtractSave(levelBlock, extractLevel, false, this->OutPoints, this->OutScales);

    // update the progress
    abort = this->UpdateProgress(extractLevel, levelBlock->Bounds);

    if (!abort && extractLevel > this->MinExtractLevel)
    {
      abort = this->TerrainExtractSubLevel(levelBlock, extractLevel - 1);
    }
    delete levelBlock;
  }
  else // process children of prevLevelBlock
  {
    for (int i = 0; i < prevLevelBlock->NumberOfSubBlocks && !abort; i++)
    {
      TerrainLevelBlock* levelBlock = prevLevelBlock->SubBlock[i];
      // get the tokens within
      double bounds[4];
      bounds[0] = this->InputBounds[0] + levelBlock->Offset[0] * levelBlock->Spacing[0] - 3 * scale;
      bounds[1] = this->InputBounds[0] +
        (levelBlock->Offset[0] + levelBlock->Ni - 1) * levelBlock->Spacing[0] + 3 * scale;
      bounds[2] = this->InputBounds[2] + levelBlock->Offset[1] * levelBlock->Spacing[1] - 3 * scale;
      bounds[3] = this->InputBounds[2] +
        (levelBlock->Offset[1] + levelBlock->Nj - 1) * levelBlock->Spacing[1] + 3 * scale;
      this->Refine->get_tokens(extractLevel, bounds, this->Tokens);

      // Allocate the terrain representation for this level.
      levelBlock->Terrain.resize(levelBlock->Ni * levelBlock->Nj);

      // DO THE WORK
      this->LevelIndex = extractLevel;

      this->Extract2D(levelBlock, prevLevelBlock, this->OutPoints, this->OutScales, tvw);
      if (prevLevelBlock->NumberOfSubBlocks == 1)
      {
        prevLevelBlock->Terrain.clear();
      }

      this->ExtractSave(levelBlock, extractLevel, true, this->OutPoints, this->OutScales);

      // update the progress
      abort = this->UpdateProgress(extractLevel, levelBlock->Bounds);

      if (!abort && levelBlock->NumberOfSubBlocks > 0)
      {
        abort = this->TerrainExtractSubLevel(levelBlock, extractLevel - 1);
      }

      // delete the "tree" as we process the leaves
      delete levelBlock;
      prevLevelBlock->SubBlock[i] = 0;
    }
  }
  return abort;
}

void vtkTerrainExtractionInternal::BuildLevelBlockTree(
  TerrainLevelBlock* parentBlock, int currentLevel, bool split)
{
  double const factor = 1;
  double scale = this->LevelScales[currentLevel];
  for (int i = 0; i < (split ? 2 : 1); i++)
  {

    for (int j = 0; j < (split ? 2 : 1); j++)
    {
      TerrainLevelBlock* childBlock = new TerrainLevelBlock;

      if (split)
      {
        childBlock->Bounds[i] = parentBlock->Bounds[i];
        childBlock->Bounds[i ? 0 : 1] = (parentBlock->Bounds[1] + parentBlock->Bounds[0]) / 2.0;
        childBlock->Bounds[j + 2] = parentBlock->Bounds[j + 2];
        childBlock->Bounds[j ? 2 : 3] = (parentBlock->Bounds[3] + parentBlock->Bounds[2]) / 2.0;
        childBlock->SplitLevel = parentBlock->SplitLevel + 1;
      }
      else
      {
        memcpy(childBlock->Bounds, parentBlock->Bounds, sizeof(double) * 4);
        childBlock->SplitLevel = parentBlock->SplitLevel;
      }

      if (currentLevel > this->MinExtractLevel)
      {
        this->BuildLevelBlockTree(childBlock, currentLevel - 1, !split);
      }

      childBlock->Spacing[0] = childBlock->Spacing[1] = scale * factor;

      this->SetupBlockExtents(childBlock);

      parentBlock->SubBlock[j * 2 + i] = childBlock;
      parentBlock->NumberOfSubBlocks++;
    }
  }
}

void vtkTerrainExtractionInternal::SetupBlockExtents(TerrainLevelBlock* childBlock)
{
  // figure out what extents to use; for currentLevel = MinExtractLevel, want
  // only points within bounds of the compute block; otherwise want one TerrainPoint
  // beyond that need by the children of childBlock
  // Origin of the whole level
  childBlock->Origin[0] = this->InputBounds[0];
  childBlock->Origin[1] = this->InputBounds[2];

  int maxIndex;
  if (childBlock->NumberOfSubBlocks == 0)
  {
    // instead of bounds should perhaps keep indices, but for now, Bounds work...
    childBlock->Offset[0] =
      int(vcl_ceil((childBlock->Bounds[0] - this->InputBounds[0]) / childBlock->Spacing[0]));
    childBlock->Offset[1] =
      int(vcl_ceil((childBlock->Bounds[2] - this->InputBounds[2]) / childBlock->Spacing[1]));
    if (childBlock->Bounds[1] == this->InputBounds[1])
    {
      maxIndex =
        int(vcl_ceil((childBlock->Bounds[1] - this->InputBounds[0]) / childBlock->Spacing[0]));
    }
    else
    {
      maxIndex =
        int(vcl_floor((childBlock->Bounds[1] - this->InputBounds[0]) / childBlock->Spacing[0]));
    }
    childBlock->Ni = maxIndex - childBlock->Offset[0] + 1;
    if (childBlock->Bounds[3] == this->InputBounds[3])
    {
      maxIndex =
        int(vcl_ceil((childBlock->Bounds[3] - this->InputBounds[2]) / childBlock->Spacing[1]));
    }
    else
    {
      maxIndex =
        int(vcl_floor((childBlock->Bounds[3] - this->InputBounds[2]) / childBlock->Spacing[1]));
    }
    childBlock->Nj = maxIndex - childBlock->Offset[1] + 1;
    return;
  }

  int extent[4] = { static_cast<int>(childBlock->SubBlock[0]->Offset[0]),
    static_cast<int>(childBlock->SubBlock[0]->Offset[0] + childBlock->SubBlock[0]->Ni - 1),
    static_cast<int>(childBlock->SubBlock[0]->Offset[1]),
    static_cast<int>(childBlock->SubBlock[0]->Offset[1] + childBlock->SubBlock[0]->Nj - 1) };
  if (childBlock->NumberOfSubBlocks == 4)
  {
    extent[1] = childBlock->SubBlock[3]->Offset[0] + childBlock->SubBlock[3]->Ni - 1;
    extent[3] = childBlock->SubBlock[3]->Offset[1] + childBlock->SubBlock[3]->Nj - 1;
  }

  childBlock->Offset[0] =
    int(vcl_floor(childBlock->SubBlock[0]->Spacing[0] * extent[0] / childBlock->Spacing[0]));
  childBlock->Offset[1] =
    int(vcl_floor(childBlock->SubBlock[0]->Spacing[1] * extent[2] / childBlock->Spacing[1]));

  maxIndex =
    int(vcl_ceil(childBlock->SubBlock[0]->Spacing[0] * extent[1] / childBlock->Spacing[0]));
  if (maxIndex >
    int(vcl_ceil(this->InputBounds[1] - this->InputBounds[0]) / childBlock->Spacing[0]))
  {
    maxIndex = int(vcl_ceil(this->InputBounds[1] - this->InputBounds[0]) / childBlock->Spacing[0]);
  }
  childBlock->Ni = maxIndex - childBlock->Offset[0] + 1;

  maxIndex =
    int(vcl_ceil(childBlock->SubBlock[0]->Spacing[1] * extent[3] / childBlock->Spacing[1]));
  if (maxIndex >
    int(vcl_ceil(this->InputBounds[3] - this->InputBounds[2]) / childBlock->Spacing[1]))
  {
    maxIndex = int(vcl_ceil(this->InputBounds[3] - this->InputBounds[2]) / childBlock->Spacing[1]);
  }
  childBlock->Nj = maxIndex - childBlock->Offset[1] + 1;
}

bool vtkTerrainExtractionFilter::TerrainExtract(
  const char* /*baseIntermediateFileName*/, vtkMultiBlockDataSet* /*output*/)
{
  // want X and Y bounds in internal structure
  memcpy(this->Internal->InputBounds, this->InputBounds, sizeof(double) * 4);
  this->Internal->MinExtractLevel = this->MinExtractLevel;
  // this is the value it is initialized to in rtvl_refine
  // the remainder of "Internal" InputBounds set from Block's bounds
  this->Internal->InputBounds[4] = this->InputBounds[4];
  this->Internal->InputBounds[5] = this->InputBounds[5];

  this->SetProgressText("Extracting Terrain");

  this->Internal->OutputSplitCount = new int[this->MaxExtractLevel + 1];
  this->Internal->LevelScales = new double[this->MaxExtractLevel + 1];
  for (int i = 0; i <= this->MaxExtractLevel; i++)
  {
    this->Internal->OutputSplitCount[i] = 0;
    this->Internal->LevelScales[i] = this->Internal->Refine->get_level_scale(i);
  }
  this->Internal->OutputFileNameBase = new vcl_string[this->MaxExtractLevel + 1];

  // 1st thing we do is figure out the level (if any) that we start splitting at
  unsigned int maxMemoryMB = 400;
  this->Internal->InitialExtractSplitLevel = this->DetermineStartingSplitLevel(maxMemoryMB);

  // process 1st couple levels a single block at a time, but then start
  // processing sub-blocks, and it's sub-block, etc, down to leaf.  Can only release
  // "parent" extracted terrain when done with complete block underneath (right now
  // released in ExtractNextLevel)

  // call recursive function which processes a current level
  return this->Internal->TerrainExtractSubLevel(0, this->MaxExtractLevel);
}

int vtkTerrainExtractionFilter::DetermineStartingSplitLevel(unsigned int maxMemoryMB)
{
  double size[2] = { this->InputBounds[1] - this->InputBounds[0],
    this->InputBounds[3] - this->InputBounds[2] };
  // figure out scale of max level based on initial scale and scaleMultiplier
  int startingSplitLevel = this->MaxExtractLevel;
  for (; startingSplitLevel >= this->MinExtractLevel; startingSplitLevel--)
  {
    unsigned int memoryMB = this->Internal->ComputeMemoryRequirement(
      startingSplitLevel, this->MinExtractLevel, false, size);
    if (startingSplitLevel == this->MinExtractLevel)
    {
      memoryMB *= 1.5;
    }
    if (memoryMB > maxMemoryMB)
    {
      if (startingSplitLevel == this->MaxExtractLevel)
      {
        // VERY surprised if we have to start splitting sooner than this!
        vtkErrorMacro("Unexpected memory managment failure!");
      }
      break;
    }
  }

  return startingSplitLevel;
}

unsigned int vtkTerrainExtractionInternal::ComputeMemoryRequirement(
  int level, int minLevel, bool splitThisLevel, double size[2])
{
  // main memory requirement from TerrinPoint + output (points + scale)
  size_t memoryPerPoint = sizeof(TerrainPoint) + sizeof(double) * 4;
  double nextSize[2] = { size[0], size[1] };

  unsigned int persistentMemory = 0;

  double scale = this->LevelScales[level];
  if (!splitThisLevel || level < 2)
  {
    double nX = (vcl_ceil(size[0] / scale) + 1.0) / 1024.0;
    double nY = (vcl_ceil(size[1] / scale) + 1.0) / 1024.0;
    persistentMemory = static_cast<unsigned int>(nX * nY * memoryPerPoint);
    if (!splitThisLevel)
    {
      nextSize[0] /= 2;
      nextSize[1] /= 2;
    }
  }

  if (level > minLevel)
  {
    persistentMemory +=
      this->ComputeMemoryRequirement(level - 1, minLevel, !splitThisLevel, nextSize);
  }

  return persistentMemory;
}

vtkPolyData* vtkTerrainExtractionInternal::ExtractSave(TerrainLevelBlock* levelBlock,
  int extractLevel, bool levelSplit, vtkPoints* extractOutPoints, vtkDoubleArray* outScales)
{
  vtkNew<vtkCellArray> verts;
  verts->Allocate(extractOutPoints->GetNumberOfPoints());

  for (vtkIdType id = 0; id < extractOutPoints->GetNumberOfPoints(); id++)
  {
    verts->InsertNextCell(1, &id);
  }

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(extractOutPoints);
  polyData->SetVerts(verts.GetPointer());
  if (this->RGBScalars)
  {
    polyData->GetPointData()->SetScalars(this->RGBScalars);
    polyData->GetPointData()->AddArray(outScales);
  }
  else
  {
    polyData->GetPointData()->SetScalars(outScales);
  }
  if (this->IntensityArray)
  {
    polyData->GetPointData()->AddArray(this->IntensityArray);
  }

  vcl_string levelFileName = this->OutputPath;
  char buf[16];
  if (levelSplit)
  {
    sprintf(buf, "%02u/", extractLevel);
    levelFileName += buf;
    vtksys::SystemTools::MakeDirectory(levelFileName.c_str());
  }

  levelFileName += this->BaseFileName;
  sprintf(buf, "_%02u", extractLevel);
  levelFileName += buf;
  if (this->OutputSplitCount[extractLevel] == 0)
  {
    this->OutputFileNameBase[extractLevel] = levelFileName;
    if (levelSplit)
    {
      this->OutputSplitCount[extractLevel] = 0x01 << levelBlock->SplitLevel;
    }
    else
    {
      this->OutputSplitCount[extractLevel] = 1;
    }
  }
  if (levelSplit)
  {
    int col = int(
      vcl_floor(((levelBlock->Bounds[1] + levelBlock->Bounds[0]) / 2.0 - levelBlock->Origin[0]) /
        (levelBlock->Bounds[1] - levelBlock->Bounds[0])));
    int row = int(
      vcl_floor(((levelBlock->Bounds[3] + levelBlock->Bounds[2]) / 2.0 - levelBlock->Origin[1]) /
        (levelBlock->Bounds[3] - levelBlock->Bounds[2])));
    int splitCount = 0x01 << levelBlock->SplitLevel;
    int quadIndex = row * splitCount + col;
    sprintf(buf, "_%d", quadIndex);
    levelFileName += buf;
  }

  this->WritePoints(levelFileName, this->Main->GetOutputPtsFormat(), polyData);
  polyData->Delete();
  return 0;
  //  return polyData;
}

struct vtkThreadUserData
{
  TerrainLevelBlock* LevelBlock;
  TerrainLevelBlock* PrevLevelBlock;
  vtkTerrainExtractionInternal* Internal;
  vtkPoints** OutPoints;
  vtkDoubleArray** OutScales;
  vtkUnsignedCharArray** RGBScalars;
  vtkFloatArray** IntensityArray;
  rtvl_weight_smooth<3>* TVW;
};

VTK_THREAD_RETURN_TYPE vtkExtract2DExecute(void* arg)
{

  int threadId = static_cast<vtkMultiThreader::ThreadInfo*>(arg)->ThreadID;
  int threadCount = static_cast<vtkMultiThreader::ThreadInfo*>(arg)->NumberOfThreads;
  vtkThreadUserData* td =
    static_cast<vtkThreadUserData*>(static_cast<vtkMultiThreader::ThreadInfo*>(arg)->UserData);

  // which rows does this thread work on
  unsigned int rowsPerThread = static_cast<int>(
    vcl_ceil(static_cast<double>(td->LevelBlock->Nj) / static_cast<double>(threadCount)));
  if (threadId * rowsPerThread >= td->LevelBlock->Nj)
  {
    // may end up with scenario where we have some threads with no work to do.
    // This might happen becasue it is setup to potentially have most of the
    // threads do the maximum amount of work, versus most do a little bit
    // less but one does more such that it is the weakest link;  Example:
    // 42 rows and 8 threads:  can have 7 threads do 5 rows each, and the 8th
    // process 7 rows... or have 7 rows each process 6 rows (the 8th thread
    // has nothing to do, but max any rows process is 6 instead of 7 as in the
    // 1st scenario)
    return VTK_THREAD_RETURN_VALUE;
  }
  unsigned int startRow = threadId * rowsPerThread;
  unsigned int lastRow = (threadId + 1) * rowsPerThread > td->LevelBlock->Nj
    ? td->LevelBlock->Nj
    : (threadId + 1) * rowsPerThread;

  td->Internal->ExtractNextLevel(td->LevelBlock, td->PrevLevelBlock, startRow, lastRow);

  vtkTerrainExtractionInternal::ThreadSpecificData threadData;

  // setup search structure to tokens, as needed by this thread
  unsigned int n = td->Internal->Tokens.points.get_number_of_points();
  unsigned int /*addPtIndex = 0, */ numberOfPointsForThisThread = 0;
  rgtl_object_array_points<2> points2D;
  // the Y (row) extents of tokens that can affect extraction for this thread
  double minY = -3.0 * td->Internal->LevelScales[td->Internal->LevelIndex] +
    td->LevelBlock->Origin[1] + td->LevelBlock->Spacing[1] * (startRow + td->LevelBlock->Offset[1]);
  double maxY = 3.0 * td->Internal->LevelScales[td->Internal->LevelIndex] +
    td->LevelBlock->Origin[1] + td->LevelBlock->Spacing[1] * (lastRow + td->LevelBlock->Offset[1]);

  points2D.set_number_of_points(n);
  threadData.PointMapping = new int[n];
  double bds[2][2] = { { VTK_FLOAT_MAX, -VTK_FLOAT_MAX }, { VTK_FLOAT_MAX, -VTK_FLOAT_MAX } };
  for (unsigned int i = 0; i < n; i++)
  {
    double p[3];
    td->Internal->Tokens.points.get_point(i, p);
    if (threadCount == 1 || (p[1] >= minY && p[1] <= maxY))
    {
      threadData.PointMapping[numberOfPointsForThisThread] = i;
      points2D.set_point(numberOfPointsForThisThread++, p);
      if (p[0] < bds[0][0])
      {
        bds[0][0] = p[0];
      }
      if (p[0] > bds[0][1])
      {
        bds[0][1] = p[0];
      }
      if (p[1] < bds[1][0])
      {
        bds[1][0] = p[1];
      }
      if (p[1] > bds[1][1])
      {
        bds[1][1] = p[1];
      }
    }
  }

  points2D.set_number_of_points(numberOfPointsForThisThread);

  // Create the 2-D spatial data structure.
  rgtl_octree_cell_bounds<2> bounds2D;
  bounds2D.compute_bounds(bds, 1.01);
  rgtl_octree_objects<2> objects2D(points2D, bounds2D, 8);

  vtkPoints* outPoints = td->OutPoints[threadId];
  vtkDoubleArray* outScales = td->OutScales[threadId];
  outPoints->Allocate((lastRow - startRow) * td->LevelBlock->Ni);
  outScales->Allocate((lastRow - startRow) * td->LevelBlock->Ni);
  threadData.RGBScalars = td->RGBScalars[threadId];
  threadData.IntensityArray = td->IntensityArray[threadId];
  if (threadData.RGBScalars)
  {
    threadData.RGBScalars->Allocate((lastRow - startRow) * td->LevelBlock->Ni * 3);
  }
  if (threadData.IntensityArray)
  {
    threadData.IntensityArray->Allocate((lastRow - startRow) * td->LevelBlock->Ni);
  }

  rtvl_weight_smooth<3> tvw = *(td->TVW);

  vtkIdType previousRowClosestPt = 0;
  for (unsigned int j = startRow; j < lastRow; j++)
  {
    threadData.PreviousClosestPt = previousRowClosestPt;
    for (unsigned int i = 0; i < td->LevelBlock->Ni; i++)
    {
      threadData.SegmentIJ[0] = i;
      threadData.SegmentIJ[1] = j;
      threadData.SegmentXY[0] =
        td->LevelBlock->Origin[0] + td->LevelBlock->Spacing[0] * (i + td->LevelBlock->Offset[0]);
      threadData.SegmentXY[1] =
        td->LevelBlock->Origin[1] + td->LevelBlock->Spacing[1] * (j + td->LevelBlock->Offset[1]);
      if (!td->Internal->ExtractSegmentInit(td->LevelBlock, threadData, objects2D))
      {
        continue;
      }
      td->Internal->ExtractSegmentSearch(
        td->LevelBlock, threadData, outPoints, td->OutScales[threadId], tvw);
      if (i == 0)
      {
        previousRowClosestPt = threadData.PreviousClosestPt;
      }
    }
  }

  delete[] threadData.PointMapping;
  return VTK_THREAD_RETURN_VALUE;
}

void vtkTerrainExtractionInternal::Extract2D(TerrainLevelBlock* levelBlock,
  TerrainLevelBlock* prevLevelBlock, vtkPoints* outPoints, vtkDoubleArray* outScales,
  rtvl_weight_smooth<3>& tvw)
{
  vtkNew<vtkMultiThreader> threader;
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(0);

  vtkThreadUserData userData;
  userData.Internal = this;
  userData.LevelBlock = levelBlock;
  userData.PrevLevelBlock = prevLevelBlock;
  userData.TVW = &tvw;

  // if less than 4 rows per thread, use less threads
  unsigned int numberOfThreads = threader->GetGlobalDefaultNumberOfThreads();
  if (levelBlock->Nj < 4 * numberOfThreads)
  {
    numberOfThreads = int(vcl_ceil(levelBlock->Nj / 4.0));
  }
  threader->SetNumberOfThreads(numberOfThreads);

  // reset existing arrays
  outPoints->Reset();
  outScales->Reset();
  if (this->RGBScalars)
  {
    this->RGBScalars->Reset();
  }
  if (this->IntensityArray)
  {
    this->IntensityArray->Reset();
  }
  userData.OutPoints = new vtkPoints*[threader->GetNumberOfThreads()];
  userData.OutScales = new vtkDoubleArray*[threader->GetNumberOfThreads()];
  userData.RGBScalars = new vtkUnsignedCharArray*[threader->GetNumberOfThreads()];
  userData.IntensityArray = new vtkFloatArray*[threader->GetNumberOfThreads()];
  if (threader->GetNumberOfThreads() == 1)
  {
    userData.OutPoints[0] = outPoints;
    userData.OutScales[0] = outScales;
    userData.RGBScalars[0] = this->RGBScalars;
    userData.IntensityArray[0] = this->IntensityArray;
  }
  else
  {
    for (int i = 0; i < threader->GetNumberOfThreads(); i++)
    {
      userData.OutPoints[i] = vtkPoints::New();
      userData.OutScales[i] = vtkDoubleArray::New();
      if (this->RGBScalars)
      {
        userData.RGBScalars[i] = vtkUnsignedCharArray::New();
        userData.RGBScalars[i]->SetNumberOfComponents(3);
      }
      else
      {
        userData.RGBScalars[i] = 0;
      }
      if (this->IntensityArray)
      {
        userData.IntensityArray[i] = vtkFloatArray::New();
      }
      else
      {
        userData.IntensityArray[i] = 0;
      }
    }
  }

  threader->SetSingleMethod(vtkExtract2DExecute, &userData);
  threader->SingleMethodExecute();

  if (threader->GetNumberOfThreads() > 1)
  {
    // combine points and arrays, then delete
    outPoints->Allocate(levelBlock->Nj * levelBlock->Ni);
    outScales->Allocate(levelBlock->Nj * levelBlock->Ni);
    if (this->RGBScalars)
    {
      this->RGBScalars->Allocate(levelBlock->Nj * levelBlock->Ni * 3);
    }
    if (this->IntensityArray)
    {
      this->IntensityArray->Allocate(levelBlock->Nj * levelBlock->Ni);
    }

    for (int i = 0; i < threader->GetNumberOfThreads(); i++)
    {
      for (vtkIdType j = 0; j < userData.OutPoints[i]->GetNumberOfPoints(); j++)
      {
        outPoints->InsertNextPoint(userData.OutPoints[i]->GetPoint(j));
        outScales->InsertNextValue(userData.OutScales[i]->GetValue(j));
        if (this->RGBScalars)
        {
          this->RGBScalars->InsertNextTuple(userData.RGBScalars[i]->GetTuple(j));
        }
        if (this->IntensityArray)
        {
          this->IntensityArray->InsertNextValue(userData.IntensityArray[i]->GetValue(j));
        }
      }
      userData.OutPoints[i]->Delete();
      userData.OutScales[i]->Delete();
      if (userData.RGBScalars[i])
      {
        userData.RGBScalars[i]->Delete();
      }
      if (userData.IntensityArray[i])
      {
        userData.IntensityArray[i]->Delete();
      }
    }
  }

  delete[] userData.OutPoints;
  delete[] userData.OutScales;
  delete[] userData.RGBScalars;
  delete[] userData.IntensityArray;
}

bool vtkTerrainExtractionInternal::ExtractSegmentInit(
  TerrainLevelBlock* levelBlock, ThreadSpecificData& threadData, rgtl_octree_objects<2>& objects2D)
{
  threadData.SegmentVoters.clear();

  // Select an initial search range.
  TerrainPoint& tp = levelBlock->GetPoint(threadData.SegmentIJ);
  if (tp.known)
  {
    threadData.SegmentRange[0] = tp.z - this->Tokens.scale / 2;
    threadData.SegmentRange[1] = tp.z + this->Tokens.scale / 2;
  }
  else
  {
    threadData.SegmentRange[0] = VTK_DOUBLE_MIN;
    threadData.SegmentRange[1] = VTK_DOUBLE_MAX;
  }

  // Lookup voters that contribute to points on this line segment.
  vcl_vector<int> voter_ids;
  int num_voters = objects2D.query_sphere(threadData.SegmentXY, 3 * this->Tokens.scale, voter_ids);
  for (int i = 0; i < num_voters; i++)
  {
    int id = threadData.PointMapping[voter_ids[i]];
    double p[3];
    this->Tokens.points.get_point(id, p);
    rtvl_tensor<3> const& tensor = this->Tokens.tokens[id];
    double flatness = tensor.saliency(0) / tensor.lambda(0);
    double const flatness_threshold = 0;
    if (flatness >= flatness_threshold &&
      p[2] + 3 * this->Tokens.scale > threadData.SegmentRange[0] &&
      p[2] - 3 * this->Tokens.scale < threadData.SegmentRange[1])
    {
      vtkTerrainExtractionInternal::SegmentVotersType::value_type entry(p[2], id);
      threadData.SegmentVoters.insert(entry);
    }
  }

  // Shrink the range to within reach of the voters.  This is an
  // optimization, and should not be propagated to another level.
  if (!threadData.SegmentVoters.empty())
  {
    // Compute the search range along the line within reach of the
    // voters.
    double range[2] = { threadData.SegmentVoters.begin()->first - 3 * this->Tokens.scale,
      threadData.SegmentVoters.rbegin()->first + 3 * this->Tokens.scale };

    // Shrink the line segment if possible.
    if (range[0] > threadData.SegmentRange[0])
    {
      threadData.SegmentRange[0] = range[0];
    }
    if (range[1] < threadData.SegmentRange[1])
    {
      threadData.SegmentRange[1] = range[1];
    }
  }
  else if (!tp.known)
  {
    // if terrain point isn't known (from previous level) AND there were no
    // SegmentVoters, then we're not going to generate an output point for
    // this SegmentIJ.
    return false;
  }
  return true;
}

bool vtkTerrainExtractionInternal::ExtractSegmentVote(
  Location& loc, ThreadSpecificData& threadData, rtvl_weight_smooth<3>& tvw)
{
  // Find the voters in reach.
  double sigma = this->Tokens.scale;
  vtkTerrainExtractionInternal::SegmentVotersType::iterator first =
    threadData.SegmentVoters.lower_bound(loc.z - 3 * sigma);
  vtkTerrainExtractionInternal::SegmentVotersType::iterator last =
    threadData.SegmentVoters.upper_bound(loc.z + 3 * sigma);
  if (first == last)
  {
    loc.saliency = 0;
    loc.constraint = 0;
    return false;
  }

  // Cast a vote with every voter.
  vnl_vector_fixed<double, 3> votee_location;
  votee_location(0) = threadData.SegmentXY[0];
  votee_location(1) = threadData.SegmentXY[1];
  votee_location(2) = loc.z;
  vnl_matrix_fixed<double, 3, 3> votee_tensor(0.0);
  vnl_matrix_fixed<double, 3, 3> votee_tensor_d[3];
  votee_tensor_d[0].fill(0.0);
  votee_tensor_d[1].fill(0.0);
  votee_tensor_d[2].fill(0.0);
  rtvl_votee_d<3> votee(votee_location, votee_tensor, votee_tensor_d);

  vnl_vector_fixed<double, 3> voter_location;
  for (vtkTerrainExtractionInternal::SegmentVotersType::iterator vi = first; vi != last; ++vi)
  {
    int j = vi->second;
    this->Tokens.points.get_point(j, voter_location.data_block());
    rtvl_voter<3> voter(voter_location, this->Tokens.tokens[j]);
    rtvl_vote(voter, votee, tvw, false);
  }

  rtvl_tensor_d<3> tensor(votee_tensor, votee_tensor_d);
  vnl_vector_fixed<double, 3> dsal;
  tensor.saliency_d(0, dsal);
  vnl_vector_fixed<double, 3>& normal = threadData.LastNormal;
  normal = tensor.basis(0);
  if (normal[2] < 0)
  {
    normal = -normal;
  }

  loc.saliency = tensor.saliency(0);
  loc.constraint = dot_product(normal, dsal);
  return true;
}

void vtkTerrainExtractionInternal::ExtractSegmentSearch(TerrainLevelBlock* levelBlock,
  ThreadSpecificData& threadData, vtkPoints* outPoints, vtkDoubleArray* outScales,
  rtvl_weight_smooth<3>& tvw)
{
  double step = this->Tokens.scale / 2;

  // Shrink the step size if the range is small.
  double size = threadData.SegmentRange[1] - threadData.SegmentRange[0];
  if (4 * step > size)
  {
    step = size / 4;
  }

  int prev_locs_count = 0;
  Location locs[3];
  double z = threadData.SegmentRange[0];
  for (;;)
  {
    locs[prev_locs_count].z = z;
    if (this->ExtractSegmentVote(locs[prev_locs_count], threadData, tvw))
    {
      if (prev_locs_count == 2)
      {
        if (locs[0].saliency < locs[1].saliency && locs[2].saliency < locs[1].saliency)
        {
          if (this->ExtractSegmentRefine(
                levelBlock, locs[0], locs[1], threadData, outPoints, outScales, tvw))
          {
            return;
          }
          if (this->ExtractSegmentRefine(
                levelBlock, locs[1], locs[2], threadData, outPoints, outScales, tvw))
          {
            return;
          }
        }
        locs[0] = locs[1];
        locs[1] = locs[2];
      }
      else
      {
        ++prev_locs_count;
      }
    }
    else
    {
      prev_locs_count = 0;
    }

    if (z >= threadData.SegmentRange[1])
    {
      break;
    }
    z += step;
    if (z >= threadData.SegmentRange[1])
    {
      z = threadData.SegmentRange[1];
    }
  }

  // No terrain was found.  Use information from previous scale.
  if ((threadData.SegmentXY[0] >= levelBlock->Bounds[0] &&
        threadData.SegmentXY[0] < levelBlock->Bounds[1] &&
        threadData.SegmentXY[1] >= levelBlock->Bounds[2] &&
        threadData.SegmentXY[1] < levelBlock->Bounds[3]) ||
    threadData.SegmentXY[0] > this->InputBounds[1] ||
    threadData.SegmentXY[1] > this->InputBounds[3])
  {
    TerrainPoint& tp = levelBlock->GetPoint(threadData.SegmentIJ);
    if (tp.known)
    {
      vnl_vector_fixed<double, 3> p;
      double transformed[3];
      p(0) = threadData.SegmentXY[0];
      p(1) = threadData.SegmentXY[1];
      p(2) = tp.z;
      this->InverseTransform->TransformPoint(p.data_block(), transformed);
      /*tp.id = */ outPoints->InsertNextPoint(transformed);
      outScales->InsertNextTypedTuple(&tp.scale);
      if (this->PointLocator)
      {
        double dist2;
        double searchRadius = sqrt(vtkMath::Distance2BetweenPoints(
          transformed, this->PointLocator->GetDataSet()->GetPoint(threadData.PreviousClosestPt)));
        vtkIdType closestPt =
          this->PointLocator->FindClosestPointWithinRadius(searchRadius, transformed, dist2);
        if (closestPt > -1)
        {
          threadData.PreviousClosestPt = closestPt;
        }
        else
        {
          closestPt = threadData.PreviousClosestPt;
        }
        if (threadData.IntensityArray)
        {
          threadData.IntensityArray->InsertNextValue(
            this->InputIntensityArray->GetValue(closestPt));
        }
        if (threadData.RGBScalars)
        {
          double rgb[3];
          this->InputRGBScalars->GetTuple(closestPt, rgb);
          threadData.RGBScalars->InsertNextTuple(rgb);
        }
      }
    }
  }
}

bool vtkTerrainExtractionInternal::ExtractSegmentRefine(TerrainLevelBlock* levelBlock, Location a,
  Location c, ThreadSpecificData& threadData, vtkPoints* outPoints, vtkDoubleArray* outScales,
  rtvl_weight_smooth<3>& tvw)
{
  if (a.constraint > 0 && c.constraint < 0)
  {
    return this->ExtractSegmentLocalMax(levelBlock, a, c, threadData, outPoints, outScales, tvw);
  }
  else
  {
    return false;
  }
}

bool vtkTerrainExtractionInternal::ExtractSegmentLocalMax(TerrainLevelBlock* levelBlock, Location a,
  Location c, ThreadSpecificData& threadData, vtkPoints* outPoints, vtkDoubleArray* outScales,
  rtvl_weight_smooth<3>& tvw)
{
  vnl_vector_fixed<double, 3> p;
  double transformed[3];
  p(0) = threadData.SegmentXY[0];
  p(1) = threadData.SegmentXY[1];

  // Given a Lipshitz constant (bound on derivative) for the saliency
  // function, we can stop the search when the bracket width is small
  // enough and the current saliency low enough that the saliency
  // maximum could not possibly be high enough.
  //
  // TODO: Actually get the constant.  This value is too conservative.
  double const max_width = this->Tokens.scale / 8;
  double const min_saliency = 10;

  int count = 0;
  double saliency = 0;
  double constraint = 10000;
  double const accuracy = this->Tokens.scale / 100;
  while ((c.z - a.z > accuracy) && vcl_fabs(constraint) > 1e-8)
  {
    // When the saliency is not high enough, we want to shrink the
    // window as quickly as possible to end early.
    double wa = 0.5;
    double wc = 0.5;
    if (count < 10 && saliency > min_saliency)
    {
      // When the saliency is high enough, we use linear interpolation
      // to predict the location of the zero for the first few steps.
      // This reduces the number of iterations to converge.
      wa = -c.constraint;
      wc = a.constraint;
      double wt = wa + wc;
      wa /= wt;
      wc /= wt;

      // Make sure the new point is not too close to either side.
      double const clip = 1.0 / 256;
      if (wa < clip)
      {
        wa = clip;
        wc = 1.0 - clip;
      }
      else if (wc < clip)
      {
        wa = 1.0 - clip;
        wc = clip;
      }
    }
    Location b = { wa * a.z + wc * c.z, 0, 0 };
    p(2) = b.z;
    if (!this->ExtractSegmentVote(b, threadData, tvw))
    {
      fprintf(stderr, "Vote continuity failure!!!!!\n");
      return false;
    }
    saliency = b.saliency;
    constraint = b.constraint;
    if (b.constraint > 0)
    {
      a = b;
    }
    else
    {
      c = b;
    }
    if (++count >= 50)
    {
      if (b.saliency > 200)
      {
        fprintf(stderr, "Failed to converge after %d iterations i=%d j=%d\n"
                        "  bracket = %g %g, width %g\n"
                        "  a.saliency = %g, a.constraint = %g\n"
                        "  c.saliency = %g, c.constraint = %g\n",
          count, threadData.SegmentIJ[0], threadData.SegmentIJ[1], a.z, c.z, c.z - a.z, a.saliency,
          a.constraint, c.saliency, c.constraint);
      }
      break;
    }
    if (b.saliency < min_saliency && (c.z - a.z < max_width))
    {
      break;
    }
  }
  if (saliency > 200)
  {
    // Make sure the normal direction is acceptable.
    TerrainPoint& tp = levelBlock->GetPoint(threadData.SegmentIJ);
    if (tp.known)
    {
      if (dot_product(tp.normal, threadData.LastNormal) < 0.866)
      {
        // TODO: Should abort the whole segment?
        return false;
      }
    }
    if (threadData.LastNormal[2] < 0.1)
    {
      //vtkDebugMacro("  terrain too steep, n = "
      //         << this->LastNormal);
      return false;
    }

    if ((threadData.SegmentXY[0] >= levelBlock->Bounds[0] &&
          threadData.SegmentXY[0] < levelBlock->Bounds[1] &&
          threadData.SegmentXY[1] >= levelBlock->Bounds[2] &&
          threadData.SegmentXY[1] < levelBlock->Bounds[3]) ||
      threadData.SegmentXY[0] > this->InputBounds[1] ||
      threadData.SegmentXY[1] > this->InputBounds[3])
    {
      this->InverseTransform->TransformPoint(p.data_block(), transformed);
      /*tp.id = */ outPoints->InsertNextPoint(transformed);
      double scale = this->Tokens.scale;
      outScales->InsertNextTypedTuple(&scale);
      if (this->PointLocator)
      {
        double dist2;
        double searchRadius = sqrt(vtkMath::Distance2BetweenPoints(
          transformed, this->PointLocator->GetDataSet()->GetPoint(threadData.PreviousClosestPt)));
        vtkIdType closestPt =
          this->PointLocator->FindClosestPointWithinRadius(searchRadius, transformed, dist2);
        if (closestPt > -1)
        {
          threadData.PreviousClosestPt = closestPt;
        }
        else
        {
          closestPt = threadData.PreviousClosestPt;
        }
        if (threadData.IntensityArray)
        {
          threadData.IntensityArray->InsertNextValue(
            this->InputIntensityArray->GetValue(closestPt));
        }
        if (threadData.RGBScalars)
        {
          double rgb[3];
          this->InputRGBScalars->GetTuple(closestPt, rgb);
          threadData.RGBScalars->InsertNextTuple(rgb);
        }
      }
    }
    tp.known = true;
    tp.level = this->LevelIndex;
    tp.scale = this->Tokens.scale;
    tp.z = p(2);
    tp.normal = threadData.LastNormal;
    return true;
  }
  else
  {
    return false;
  }
}

void vtkTerrainExtractionInternal::ExtractNextLevel(TerrainLevelBlock* levelBlock,
  TerrainLevelBlock* prevLevelBlock, unsigned int startRow, unsigned int endRow)
{
  // Short-circuit for the first level.
  if (!prevLevelBlock)
  {
    return;
  }

  // Initialize the next level with bilinear interpolation of this level.
  for (unsigned int j = startRow; j < endRow; j++)
  {
    for (unsigned int i = 0; i < levelBlock->Ni; i++)
    {
      double p[2] = { levelBlock->Origin[0] + levelBlock->Spacing[0] * (i + levelBlock->Offset[0]),
        levelBlock->Origin[1] + levelBlock->Spacing[1] * (j + levelBlock->Offset[1]) };
      double prev_index[2] = { (p[0] - prevLevelBlock->Origin[0]) / prevLevelBlock->Spacing[0],
        (p[1] - prevLevelBlock->Origin[1]) / prevLevelBlock->Spacing[1] };
      int prev_i = int(vcl_floor(prev_index[0]));
      int prev_j = int(vcl_floor(prev_index[1]));
      double dx = prev_index[0] - prev_i;
      double dy = prev_index[1] - prev_j;
      double z = 0;
      double w = 0;
      vnl_vector_fixed<double, 3> n;
      n.fill(0.0);
      int level = -1;
      double scale = 0;
      // "convert" the indices to the local "block" space
      prev_i -= prevLevelBlock->Offset[0];
      prev_j -= prevLevelBlock->Offset[1];
      prevLevelBlock->Contribute(
        prev_i, prev_j, (1 - dx) * (1 - dy), p[0], p[1], z, n, w, level, scale);
      prevLevelBlock->Contribute(
        prev_i + 1, prev_j, (dx) * (1 - dy), p[0], p[1], z, n, w, level, scale);
      prevLevelBlock->Contribute(
        prev_i, prev_j + 1, (1 - dx) * (dy), p[0], p[1], z, n, w, level, scale);
      prevLevelBlock->Contribute(
        prev_i + 1, prev_j + 1, (dx) * (dy), p[0], p[1], z, n, w, level, scale);
      if (w > 0)
      {
        z /= w;
        scale /= w;
        n.normalize();
        TerrainPoint& tp = levelBlock->GetPoint(i, j);
        tp.known = true;
        tp.level = level;
        tp.scale = scale;
        tp.z = z;
        tp.normal = n;
      }
    }
  }
}

int vtkTerrainExtractionFilter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1); // only needed for "setup refine" step
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

void vtkTerrainExtractionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
