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

#include "vtkDataObject.h"
#include "vtkSmartPointer.h"

#include <algorithm>
#include <string>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

struct SMTKIOVTK_EXPORT ImportFormat
{
  ImportFormat(std::string&& name, std::vector<std::string>&& extensions)
    : Name(name)
    , Extensions(extensions)
  {
  }

  std::string Name;
  std::vector<std::string> Extensions;
};

namespace detail
{

/// ImportAsVTKData is a functor for importing files as VTK data objects. It
/// uses the smtk::common::Generator pattern, which checks if a generator is
/// valid using the virtual method valid(). Our validity check is a comparison
/// against a file's extension; the list of acceptable extensions is also needed
/// by other consuming code. We therefore decorate the Generator by requiring
/// that generator types return a set of acceptable extensions, rather than
/// requiring an implementation of the method valid().
class SMTKIOVTK_EXPORT ImportAsVTKDataBase
{
public:
  virtual std::vector<ImportFormat> fileFormats() const = 0;
};
} // namespace detail
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk

class vtkDataObject;

#ifndef smtkIOVTK_EXPORTS
extern
#endif
  template class SMTKIOVTK_EXPORT smtk::common::Generator<
    std::pair<std::string, std::string>,
    vtkSmartPointer<vtkDataObject>,
    smtk::extension::vtk::io::detail::ImportAsVTKDataBase>;

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

/// Each generator type for ImportAsVTKData must populate a set of strings
/// describing the file extensions it can (attempt to) read.
template<class Self>
class ImportAsVTKDataType
  : public smtk::common::GeneratorType<
      std::pair<std::string, std::string>,
      vtkSmartPointer<vtkDataObject>,
      Self,
      smtk::extension::vtk::io::detail::ImportAsVTKDataBase>
{
public:
  ImportAsVTKDataType()
    : smtk::common::GeneratorType<
        std::pair<std::string, std::string>,
        vtkSmartPointer<vtkDataObject>,
        Self,
        smtk::extension::vtk::io::detail::ImportAsVTKDataBase>({})
  {
  }
  ImportAsVTKDataType(ImportFormat&& format)
    : m_format(format)
  {
  }

  std::vector<ImportFormat> fileFormats() const override { return { m_format }; }

  bool valid(const std::pair<std::string, std::string>& fileInfo) const override
  {
    return std::find(
             std::begin(m_format.Extensions), std::end(m_format.Extensions), fileInfo.first) !=
      std::end(m_format.Extensions);
  }

  vtkSmartPointer<vtkDataObject> operator()(const std::pair<std::string, std::string>&) override =
    0;

protected:
  ImportFormat m_format;
};

/// A functor that accepts as input (a) a pair of strings describing the file
/// type and file url or (b) a string describing the file url and returns a
/// vtkSmartPointer to the data described by the file. This class is extensible
/// via the registration of additional reader types (see
/// smtk::common::Generator).
class SMTKIOVTK_EXPORT ImportAsVTKData
  : public smtk::common::Generator<
      std::pair<std::string, std::string>,
      vtkSmartPointer<vtkDataObject>,
      detail::ImportAsVTKDataBase>
{
public:
  using smtk::common::Generator<
    std::pair<std::string, std::string>,
    vtkSmartPointer<vtkDataObject>,
    ImportAsVTKDataBase>::valid;
  using smtk::common::Generator<
    std::pair<std::string, std::string>,
    vtkSmartPointer<vtkDataObject>,
    ImportAsVTKDataBase>::operator();

  ImportAsVTKData();
  ~ImportAsVTKData() override;

  std::vector<ImportFormat> fileFormats() const override;

  bool valid(const std::string& file) const;

  vtkSmartPointer<vtkDataObject> operator()(const std::string& file);
};
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
