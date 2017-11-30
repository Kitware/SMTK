//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/View.h"
#include "smtk/view/jsonView.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/attribute/Collection.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace nlohmann;

int unitJSONView(int argc, char* argv[])
{
  bool ok = true;
  smtk::io::Logger::instance().setFlushToStdout(true);
  smtk::io::AttributeReader rdr;

  std::string readFilePath(SMTK_DATA_DIR);
  std::string jsonFile("/attribute/attribute_collection/viewproto3.json");
  std::string xmlFile("/attribute/attribute_collection/viewproto3.xml");

  auto rsrc = smtk::attribute::Collection::create();
  rdr.read(rsrc, argc > 2 ? argv[2] : (readFilePath + xmlFile).c_str(), false,
    smtk::io::Logger::instance());

  std::cout << (readFilePath + jsonFile) << "\n";
  std::ifstream file(argc > 1 ? argv[1] : (readFilePath + jsonFile).c_str());
  std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  json j = json::parse(data);
  int numJSONViews = 0;
  for (auto view : j["views"])
  {
    smtk::view::ViewPtr test = view;
    json jtmp = test;
    auto xit = rsrc->views().find(test->title());
    if (xit == rsrc->views().end())
    {
      std::cerr << jtmp.dump(2) << "\n";
      smtkErrorMacro(smtk::io::Logger::instance(), "Error: View "
          << test->title() << " (" << test->type() << ") unmatched.");
      ok = false;
    }
    else
    {
      if (*xit->second == *test)
      {
        smtkInfoMacro(smtk::io::Logger::instance(), "Views of " << test->title() << " matched.");
      }
      else
      {
        json urk = xit->second;
        std::cerr << "different from baseline:\n" << urk.dump(2) << "\n";
        smtkErrorMacro(smtk::io::Logger::instance(), "Views did not match");
        ok = false;
      }
    }
    ++numJSONViews;
  }

  smtkInfoMacro(smtk::io::Logger::instance(), "Tested " << numJSONViews << " JSON views against "
                                                        << rsrc->views().size() << " XML views.");
  test(numJSONViews == static_cast<int>(rsrc->views().size()),
    "XML and JSON had different numbers of views.");

  return ok ? 0 : 1;
}
