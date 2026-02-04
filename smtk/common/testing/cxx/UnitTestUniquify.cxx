//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/StringUtil.h"

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

int UnitTestUniquify(int, char** const)
{
  // Test that smtk::common::StringUtil::uniquify() is able to produce
  // unique names by appending an underscore and an integer number unique
  // to the keys of the passed dictionary.

  bool ok = true;
  std::vector<std::string> fails{ "foo",          "foo_a", "foo_0a",
                                  "foo_lksfj_a0", "_",     "_25699456738475773459358234577",
                                  "foo_" };
  std::vector<std::string> passes{ "foo_0", "foo_1", "foo_42", "bar_86", "_37" };

  int ii;
  for (const auto& fail : fails)
  {
    ii = 0;
    std::string key = fail;
    if (smtk::common::StringUtil::endsWithUnderscoreNumber(key, ii))
    {
      std::cerr << "ERROR: \"" << fail << "\" should not have passed. (" << key << ", " << ii
                << ")\n";
      ok = false;
    }
    else
    {
      if (key != fail || ii != 0)
      {
        std::cerr << "ERROR: \"" << fail << "\" modified inputs (" << key << ", " << ii << ")\n";
      }
      else
      {
        std::cout << "Expected failure \"" << fail << "\" (" << key << ", " << ii << ")\n";
      }
    }
  }

  for (const auto& pass : passes)
  {
    ii = 0;
    std::string key = pass;
    if (!smtk::common::StringUtil::endsWithUnderscoreNumber(key, ii))
    {
      std::cerr << "ERROR: \"" << pass << "\" should have passed. (" << key << ", " << ii << ")\n";
      ok = false;
    }
    else
    {
      std::cout << "\"" << key << "\" with " << ii << "\n";
    }
  }

  std::map<std::string, std::string> dict{ { "foo_0", "foo_2" },   { "foo_1", "foo_2" },
                                           { "foo_37", "foo_38" }, { "bar_86", "bar_88" },
                                           { "bar_87", "bar_87" }, { "_0", "_1" } };
  for (const auto& pass : passes)
  {
    std::string uu = smtk::common::StringUtil::uniquify(pass, dict);
    std::cout << "\"" << pass << "\" → \"" << uu << "\"\n";
    if (dict.find(pass) != dict.end() && dict[pass] != uu)
    {
      std::cerr << "ERROR: \"" << pass << "\" was \"" << uu << "\", expected \"" << dict[pass]
                << "\".\n";
      ok = false;
    }
  }

  return ok ? 0 : 1;
}
