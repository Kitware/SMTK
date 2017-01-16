//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_vtk_vtkPolyFileErrorReporter_h
#define __smtk_vtk_vtkPolyFileErrorReporter_h

#define VTK_POLYFILE_EOF "EndOfFile"
#define VTK_POLYFILE_OUT_OF_DATA "NothingGoodToRead"
#define VTK_POLYFILE_BAD_TOKEN "BadToken"
#define VTK_POLYFILE_MISSING_DIM "NodeDimensionMissing Assuming dimension=3"
#define VTK_POLYFILE_BAD_DIM "NodeDimensionBad"
#define VTK_POLYFILE_MISSING_SEGMENT_BDY "NoSegmentBoundaryMarkerProvided"
#define VTK_POLYFILE_TOO_MANY_SEGMENT_ERRS "TooManySegmentBoundaryMarkerWarnings"
#define VTK_POLYFILE_MISSING_FACET_BDY "NoFacetBoundaryMarkerProvided"
#define VTK_POLYFILE_TOO_MANY_FACET_ERRS "TooManyFacetBoundaryMarkerWarnings"
#define VTK_POLYFILE_BAD_REGION_COUNT "BadNumberOfRegions"
#define VTK_POLYFILE_BAD_EXTERNAL_NODEFILE "BadSeparateNodeFile"
#define VTK_POLYFILE_BAD_NODEFILE_POINTS "BadNodeFileNumPts"
#define VTK_POLYFILE_CANNOT_OPEN "UnableToOpenFile"

#include <iostream>
#include <sstream>
#include <string>

class vtkPolyFileErrorReporter
{
public:
  vtkPolyFileErrorReporter(const std::string& fname)
    {
    this->FileName = fname;
    this->Warnings = 0;
    this->EndOfFile = 0;
    this->ProvideContext = 1;
    }

  void PrintContext(std::istream& stream, int posn)
    {
    std::streampos where = stream.tellg();
    if (where < 0)
      {
      return;
      }
    std::cout << "\n";
    std::streamoff i(0);
    for (int numLines = 0; numLines < 3; ++numLines)
      {
      stream.clear();
      for (; stream.good(); ++i)
        {
        stream.seekg(where - i);
        if (stream.get() == '\n')
          {
          break;
          }
        }
      ++i; // skip past newline
      }
    for (int numLines = 0; numLines < 4; ++numLines)
      {
      std::string context;
      std::streampos lineStart = stream.tellg();
      std::getline(stream, context);
      std::cout << static_cast<int>(lineStart) << ":\"" << context << "\"\n";
      if (lineStart <= posn && (static_cast<size_t>(lineStart) + context.size()) >= static_cast<size_t>(posn))
        {
        std::cout << static_cast<int>(lineStart) << ":-";
        for (int k = posn - static_cast<int>(lineStart); k > 0; --k)
          {
          std::cout << "-";
          }
        std::cout << "^\n";
        }
      }
    std::cout << "\n";
    stream.seekg(where);
    }

  bool Report(std::istream& stream, const std::string& err)
    {
    int posn = static_cast<int>(stream.tellg());
    return this->Report(stream, posn, posn, err);
    }

  bool Report(std::istream& stream, int pos0, int pos1, const std::string& err)
    {
    if (err != VTK_POLYFILE_EOF)
      {
      std::cerr << this->FileName << ": \"" << err << "\" at bytes " << pos0 << " -- " << pos1 << ". Ignoring.\n";
      ++this->Warnings;
      if (this->ProvideContext)
        {
        this->PrintContext(stream, pos0);
        }
      }
    else
      { // EOF
      ++this->EndOfFile;
      }
    return false; // false == keep going, true == stop
    }

  std::string FileName;
  int Warnings;
  int EndOfFile;
  int ProvideContext;
};

#endif // __vtkPolyFileErrorReporter_h
