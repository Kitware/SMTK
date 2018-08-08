//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DirectoryInfo.h"

#include <iostream>

using namespace smtk::attribute;

void FileInfo::print(const std::string& pre)
{
  std::cerr << pre << "File Name: " << m_filename << "\n" << pre << "  Includes:\n";
  for (auto inc : m_includes)
  {
    std::cerr << pre << "  " << inc << "\n";
  }
  std::cerr << pre << "  Categories:\n";
  for (auto cat : m_catagories)
  {
    std::cerr << pre << "  " << cat;
    if (cat == m_defaultCategory)
    {
      std::cerr << " (Default)";
    }
    std::cerr << std::endl;
  }
}
