//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkExtractImageBlock.h"

#include "vtkBoundingBox.h"
#include "vtkCompositeDataIterator.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredData.h"

vtkStandardNewMacro(vtkExtractImageBlock);

vtkExtractImageBlock::vtkExtractImageBlock()
{
  this->BlockIndex = -1;
  this->Extent[0] = this->Extent[2] = this->Extent[4] = 0;
  this->Extent[1] = this->Extent[3] = this->Extent[5] = -1;
}

int vtkExtractImageBlock::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

void vtkExtractImageBlock::SetExtent(int* extent)
{
  int description = vtkStructuredData::SetExtent(extent, this->Extent);
  if (description < 0) //improperly specified
  {
    vtkErrorMacro(<< "Bad Extent, retaining previous values");
    return;
  }

  if (description == VTK_UNCHANGED)
  {
    return;
  }

  this->Modified();
}

void vtkExtractImageBlock::SetExtent(int x1, int x2, int y1, int y2, int z1, int z2)
{
  int ext[6];
  ext[0] = x1;
  ext[1] = x2;
  ext[2] = y1;
  ext[3] = y2;
  ext[4] = z1;
  ext[5] = z2;
  this->SetExtent(ext);
}

// Fill in the WholeExtent and spacing information from the image block
int vtkExtractImageBlock::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  double bounds[6];
  for (int i = 0; i < 6; i++)
    bounds[i] = (double)this->Extent[i];

  vtkBoundingBox bbox(bounds);
  if (bbox.IsValid())
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->Extent, 6);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), this->Extent, 6);
  }

  return 1;
}

int vtkExtractImageBlock::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMultiBlockDataSet* input = vtkMultiBlockDataSet::GetData(inputVector[0], 0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkImageData* output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input)
  {
    vtkErrorMacro("Input not specified!");
    return 0;
  }

  if (this->BlockIndex < 0 ||
    static_cast<unsigned int>(this->BlockIndex) >= input->GetNumberOfBlocks())
  {
    vtkErrorMacro("Must specify a valid block index to extract!");
    return 0;
  }

  vtkCompositeDataIterator* iter = input->NewIterator();

  int numberOfLeaves = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    numberOfLeaves++;
  }

  if (numberOfLeaves <= this->BlockIndex)
  {
    iter->Delete();
    vtkErrorMacro("Specified Index is too large!");
    return 0;
  }

  // Copy selected block over to the output.
  int index = 0;
  vtkImageData* block = NULL;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), index++)
  {
    if (index == this->BlockIndex)
    {
      block = vtkImageData::SafeDownCast(iter->GetCurrentDataObject());
      break;
    }
  }
  iter->Delete();

  if (block)
  {
    output->ShallowCopy(block);
    return 1;
  }
  else
  {
    vtkErrorMacro("The specified block is either null or not valid image data!");
    return 0;
  }
}

void vtkExtractImageBlock::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Block Index: " << this->BlockIndex << "\n";
}
