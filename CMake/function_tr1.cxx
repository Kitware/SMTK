//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <tr1/functional>

struct favorite_int {
  int operator() () { return 42; }
};

struct conditional_int {
  int operator() (float x) { return 2 * static_cast<int>(x); }
};

int main(int, char**)
{
  std::tr1::function<int()> f;
  f = favorite_int();
  std::tr1::function<int()> g;
  g = std::tr1::bind<float>(conditional_int(), 21.0);
  return (f() == 42 && g() == 42 ) ? 0 : 1;
}
