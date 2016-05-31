//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <tr1/memory>

int main(int argc, char** argv)
{
  std::tr1::shared_ptr<float> f = std::tr1::make_shared(42.0f);
  return 0;
}
