//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_io_RedirectOutput_h
#define smtk_extension_vtk_io_RedirectOutput_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/vtk/io/IOVTKExports.h"

namespace smtk
{
namespace io
{
class Logger;
}
} // namespace smtk

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

//Redirect the output from VTK I/O to an smtk::io::Logger.
SMTKIOVTK_EXPORT void RedirectVTKOutputTo(smtk::io::Logger& log);

//Reset the output from VTK I/O back to its default behavior.
SMTKIOVTK_EXPORT void ResetVTKOutput();
} // namespace io
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
