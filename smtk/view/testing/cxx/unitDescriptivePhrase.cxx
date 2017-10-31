//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.txx"
#include "smtk/view/ResourcePhrase.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/testing/cxx/helpers.h"

#include "smtk/model/SessionRef.h"

#include "smtk/io/LoadJSON.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <fstream>
#include <iostream>
#include <string>

#include <stdlib.h>

using smtk::shared_ptr;
using namespace smtk::common;
using namespace smtk::view;
using namespace smtk::io;

static int maxIndent = 20;

template <typename T>
void printPhrase(std::ostream& os, int indent, T p)
{
  // Do not descend too far, as infinite recursion is possible,
  // even with the SimpleSubphraseGenerator
  if (indent > maxIndent)
    return;

  os << std::string(indent, ' ') << p->title() << "  (" << p->subtitle() << ")";
  smtk::resource::FloatList rgba = p->relatedColor();
  if (rgba[3] >= 0.)
    os << " rgba(" << rgba[0] << "," << rgba[1] << "," << rgba[2] << "," << rgba[3] << ")";
  os << "\n";
  auto sub = p->subphrases();
  indent += 2;
  for (auto it = sub.begin(); it != sub.end(); ++it)
  {
    printPhrase(os, indent, *it);
  }
}

int unitDescriptivePhrase(int argc, char* argv[])
{
  auto rsrcMgr = smtk::resource::Manager::create();
  auto rsrcs = smtk::resource::testing::loadTestResources(rsrcMgr, argc, argv);
  smtk::view::ResourcePhraseArray loaded;
  for (auto rsrc : rsrcs)
  {
    loaded.push_back(smtk::view::ResourcePhrase::create()->setup(rsrc));
  }
  auto topLevel = smtk::view::PhraseList::create()->setup(loaded);
  auto generator = smtk::view::SubphraseGenerator::create();
  topLevel->setDelegate(generator);

  printPhrase(std::cout, 0, topLevel);

  return 0;
}

smtkComponentInitMacro(smtk_polygon_session);
