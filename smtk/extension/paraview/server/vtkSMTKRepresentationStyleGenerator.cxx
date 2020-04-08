//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/server/vtkSMTKRepresentationStyleGenerator.h"

// must be declared in a .cxx file, and not .h, so external users like RGG
// will link properly - i.e. it must be compiled somewhere.
template class smtk::common::Generator<smtk::resource::ResourcePtr,
  vtkSMTKResourceRepresentation::StyleFromSelectionFunction>;
