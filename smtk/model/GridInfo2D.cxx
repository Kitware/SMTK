/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME GridInfo2D.h - class to get information for 2D grids
// .SECTION Description
// Class to get information for 2D grids. CMB will implement a version
// of this to get the grid information into SMTK from CMB without
// introducing a dependency on CMB.
// Note that this should be an abstract class but due to limitations
// with Python wrapping we need it to be a concrete implementation.
// .SECTION See Also


#include "smtk/model/GridInfo2D.h"

namespace smtk::model
{

//----------------------------------------------------------------------------
GridInfo2D::GridInfo2D()
{
}

//----------------------------------------------------------------------------
GridInfo2D::~GridInfo2D()
{
}

//----------------------------------------------------------------------------
void GridInfo2D::groupCellIds(int, std::vector<int>& cellIds)
{
  cellIds.reset();
  std::cerr << "GridInfo2D::groupCellIds() should be implemented by a derived class\n";
}

//----------------------------------------------------------------------------
double GridInfo2D::groupArea(int)
{
  std::cerr << "GridInfo2D::groupArea() should be implemented by a derived class\n";
  return -1;
}

//----------------------------------------------------------------------------
std::vector<int> GridInfo2D::cellPointIds(int)
{
  std::cerr << "GridInfo2D::cellPointIds() should be implemented by a derived class\n";
  std::vector<int> notUsed;
  return notUsed
}

//----------------------------------------------------------------------------
std::vector<double> GridInfo2D::pointLocation(int)
{
  std::cerr << "GridInfo2D::pointLocation() should be implemented by a derived class\n";
  std::vector<double> notUsed;
  return notUsed;
}
}
