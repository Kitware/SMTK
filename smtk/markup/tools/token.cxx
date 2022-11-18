//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/Token.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

int usage(int code, const std::string& message = std::string())
{
  std::cout << R"(
String tokenizer
----------------

This is a utility to compute the integer token for a given string
(or strings). The token is a hash of the string's value that is
likely to be unique for any given set of unique input strings.
The fnv1a hash algorithm provided by smtk::string::Token is used
for the computation.

Usage:
  token string [string]+
where
  string          – is a string you want tokenized.

)" << message
            << "\n";
  return code;
}

int main(int argc, char* argv[])
{
  std::set<smtk::string::Token> tokens;
  for (int ii = 1; ii < argc; ++ii)
  {
    std::string arg(argv[ii]);
    if (arg == "--help" || arg == "-h")
    {
      return usage(0);
    }
    tokens.insert(tokens.end(), arg);
  }
  if (tokens.empty())
  {
    return usage(1, "You must provide at least one string to tokenize.");
  }

  int pad = 0;
  for (const auto& token : tokens)
  {
    auto tlen = static_cast<int>(token.data().size());
    pad = pad > tlen ? pad : tlen;
  }

  for (const auto& token : tokens)
  {
    std::string padding(pad + 4 - token.data().size(), ' ');
    std::cout << token.data() << padding << " " << token.id() << "\n";
  }
  return 0;
}
