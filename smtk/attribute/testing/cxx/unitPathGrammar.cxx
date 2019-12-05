//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/PathGrammar.h"
#include "smtk/common/testing/cxx/helpers.h"

int unitPathGrammar(int, char** const)
{
  using namespace smtk::attribute;
  // Some quick sanity grammar tests
  std::string a, b, c, path = "/a/b.s/jk_j3/foo-bar";
  bool ok;
  pegtl::string_input<> in(path, "sampleTest");

  pegtl::parse<pathGrammar::grammar, pathGrammar::action>(in, a, b, ok);
  test(ok, "Failed to parse: /a/b.s/jk_j3/foo-bar!");
  std::cerr << "Path = " << path << " Leading Item = " << a << " Residue = " << b << std::endl;
  test(a == "a", "Failed: Leading Item should be a!");
  test(b == "/b.s/jk_j3/foo-bar", "Failed: Residue should be /b.s/jk_j3/foo-bar !");
  path = b;
  a.clear();
  pegtl::string_input<> in1(path, "sampleTest");
  pegtl::parse<pathGrammar::grammar, pathGrammar::action>(in1, a, b, ok);
  test(ok, "Failed to parse: /b.s/jk_j3/foo-bar!");
  std::cerr << "Path = " << path << " Leading Item = " << a << " Residue = " << b << std::endl;
  test(a == "b.s", "Failed: Leading Item should be b.s!");
  test(b == "/jk_j3/foo-bar", "Failed: Residue should be /jk_j3/foo-bar !");
  path = b;
  a.clear();
  pegtl::string_input<> in2(path, "sampleTest");
  pegtl::parse<pathGrammar::grammar, pathGrammar::action>(in2, a, b, ok);
  test(ok, "Failed to parse: /jk_j3/foo-bar!");
  std::cerr << "Path = " << path << " Leading Item = " << a << " Residue = " << b << std::endl;
  test(a == "jk_j3", "Failed: Leading Item should be jk_j3!");
  test(b == "/foo-bar", "Failed: Residue should be /foo-bar !");
  path = b;
  a.clear();
  pegtl::string_input<> in3(path, "sampleTest");
  pegtl::parse<pathGrammar::grammar, pathGrammar::action>(in3, a, b, ok);
  test(ok, "Failed to parse: /foo-bar!");
  std::cerr << "Path = " << path << " Leading Item = " << a << " Residue = " << b << std::endl;
  test(a == "foo-bar", "Failed: Leading Item should be foo-bar!");
  test(b.empty(), "Failed: Residue should be empty !");

  return 0;
}
