//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Resource.h"
#include "smtk/markup/detail/DumpColors.h"
#include "smtk/markup/operators/Read.h"

#include "smtk/graph/evaluators/Dump.h"

#include "smtk/common/Color.h"

#include <iostream>
#include <string>

int usage(int code, const std::string& message = std::string())
{
  std::cout << R"(
Usage:
  markup_dump [--ascii] [--bgcolor rgba] filename.smtk
where
  --ascii         – indicates that the ASCII dump format (rather than
                    graphviz) should be used.
  --bgcolor rgba  – a hexadecimal RGB or RGBA string specifying the
                    background color for the graph (e.g., "#ff0000ff").
  filename.smtk   – is the path to an SMTK markup resource file.

)" << message
            << "\n";
  return code;
}

int main(int argc, char* argv[])
{
  bool ascii = false;
  std::string filename;
  std::array<double, 4> bgColor{ 1., 1., 1., 1. };
  for (int ii = 1; ii < argc; ++ii)
  {
    std::string arg(argv[ii]);
    if (arg == "--help" || arg == "-h")
    {
      return usage(0);
    }
    else if (arg == "--ascii")
    {
      ascii = true;
    }
    else if (arg == "--bgcolor")
    {
      if (ii < argc - 1)
      {
        ++ii;
        smtk::common::Color::stringToFloatRGBA(bgColor.data(), argv[ii]);
      }
      else
      {
        return usage(1, "You must provide a background color as the next value.");
      }
    }
    else
    {
      filename = arg;
    }
  }
  if (filename.empty())
  {
    return usage(1, "You must provide a filename to read.");
  }

  auto resource = std::dynamic_pointer_cast<smtk::markup::Resource>(smtk::markup::read(filename));
  if (!resource)
  {
    return usage(2, "Unable to read file.");
  }

  smtk::graph::evaluators::Dump dump(ascii ? "text/plain" : "text/vnd.graphviz");
  smtk::graph::evaluators::Dump::setBackground(bgColor);
  for (const auto& arcColor : smtk::markup::DumpArcColors())
  {
    dump.setArcColor(arcColor.first, arcColor.second);
  }
  resource->evaluateArcs<smtk::graph::evaluators::Dump>(std::cout, dump);
  // resource->dump(std::string(), ascii ? "text/plain" : "text/vnd.graphviz");
  return 0;
}
