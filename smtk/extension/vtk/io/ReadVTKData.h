//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_vtk_io_ReadVTKData_h
#define __smtk_extension_vtk_io_ReadVTKData_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/Generator.h"

#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "vtkSmartPointer.h"

#include <string>

class vtkDataObject;

#ifndef smtkIOVTK_EXPORTS
extern
#endif
  template class SMTKIOVTK_EXPORT
    smtk::common::Generator<std::pair<std::string, std::string>, vtkSmartPointer<vtkDataObject> >;

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

/// A functor that accepts as input (a) a pair of strings describing the file
/// type and file url or (b) a string describing the file url and returns a
/// vtkSmartPointer to the data described by the file. This class is extensible
/// via the registration of additional reader types.
class SMTKIOVTK_EXPORT ReadVTKData
  : public smtk::common::Generator<std::pair<std::string, std::string>,
      vtkSmartPointer<vtkDataObject> >
{
public:
  using smtk::common::Generator<std::pair<std::string, std::string>,
    vtkSmartPointer<vtkDataObject> >::valid;
  using smtk::common::Generator<std::pair<std::string, std::string>,
    vtkSmartPointer<vtkDataObject> >::operator();

  virtual ~ReadVTKData();

  bool valid(const std::string& file) const;

  vtkSmartPointer<vtkDataObject> operator()(const std::string& file);
};
}
}
}
}

#endif
