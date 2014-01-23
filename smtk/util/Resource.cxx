/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also


#include "smtk/util/Resource.h"

using namespace smtk::util;


//----------------------------------------------------------------------------
Resource::Resource()
{
}

//----------------------------------------------------------------------------
Resource::~Resource()
{
}

//----------------------------------------------------------------------------
std::string Resource::type2String(Resource::Type t)
{
  switch (t)
    {
    case ATTRIBUTE:
      return "attribute";
    case MODEL:
      return "model";
    case MESH:
      return "mesh";
    default:
      return "";
    }
  return "Error!";
}

//----------------------------------------------------------------------------
Resource::Type Resource::string2Type(const std::string &s)
{
  if (s == "attribute")
    {
    return ATTRIBUTE;
    }
  if (s == "model")
    {
    return MODEL;
    }
  if (s == "mesh")
    {
    return MESH;
    }
  return NUMBER_OF_TYPES;
}
