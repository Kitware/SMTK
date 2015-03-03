//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBPt123Reader - "reader" for the pt123 formats
// .SECTION Description - Functions common to many cmb readers
//


#ifndef __smtkdiscrete_vtkCMBReaderHelperFunctions_h
#define __smtkdiscrete_vtkCMBReaderHelperFunctions_h

#include <fstream>
#include <sstream>
#include <string>


namespace smtk {
  namespace bridge {
    namespace discrete {

namespace ReaderHelperFunctions
{
  bool readNextLine(std::ifstream& file, std::stringstream& line);
  bool readNextLine(std::ifstream& file, std::stringstream& line, std::string& card);

  inline const char* GetModelFaceTagName() {return "modelfaceids";}
  inline const char* GetShellTagName() {return "Region";}
  inline const char* GetMaterialTagName() {return "cell materials";}
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif
