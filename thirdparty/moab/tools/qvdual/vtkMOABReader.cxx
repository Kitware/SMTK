/*
 * Copyright 2004 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMOABReader.h"

#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMOABUtils.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkProperty.h"
#include "vtkExtractEdges.h"
#include "vtkTubeFilter.h"
#include "vtkRenderer.h"
#include "vtkExtractGeometry.h"
#include "vtkPlane.h"

#include "assert.h"

vtkCxxRevisionMacro(vtkMOABReader, "$Revision$");
vtkStandardNewMacro(vtkMOABReader);

vtkMOABReader::vtkMOABReader()
{
  this->FileName = NULL;
}

vtkMOABReader::~vtkMOABReader()
{
}

void vtkMOABReader::Execute()
{
    //this->DebugOn();

    // assert that MOAB has been initialized
  MBErrorCode result;
  assert(NULL != vtkMOABUtils::mbImpl);
  
  result = vtkMOABUtils::mbImpl->load_mesh(this->GetFileName());
  if (MB_SUCCESS != result)
    {
    vtkErrorMacro( << "Failed to open file " << this->GetFileName() );
    return;
    }
  vtkDebugMacro(<<"Read MOAB file...");
}
