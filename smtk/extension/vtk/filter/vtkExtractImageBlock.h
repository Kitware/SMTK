//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkExtractImageBlock - extracts a single image block from a multiblock dataset.
// .SECTION Description
// vtkExtractImageBlock is simialr to vtkExtractBlock, except that only an image
// block was returned.

// .SECTION See Also
// vtkExtractBlock

#ifndef smtk_vtk_ExtractImageBlock_h
#define smtk_vtk_ExtractImageBlock_h

#include "smtk/extension/vtk/filter/vtkSMTKFilterExtModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKSMTKFILTEREXT_EXPORT vtkExtractImageBlock : public vtkImageAlgorithm
{
public:
  static vtkExtractImageBlock* New();
  vtkTypeMacro(vtkExtractImageBlock, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkExtractImageBlock(const vtkExtractImageBlock&) = delete;
  vtkExtractImageBlock& operator=(const vtkExtractImageBlock&) = delete;

  // Description:
  // Select the block index to be extracted. Default is 0.
  vtkSetMacro(BlockIndex, int);
  vtkGetMacro(BlockIndex, int);

  virtual void SetExtent(int extent[6]);
  virtual void SetExtent(int x1, int x2, int y1, int y2, int z1, int z2);
  vtkGetVector6Macro(Extent, int);

protected:
  vtkExtractImageBlock();
  ~vtkExtractImageBlock() override = default;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  /// Implementation of the algorithm.
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  int BlockIndex;
  int Extent[6];
};

#endif
