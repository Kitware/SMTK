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
// .NAME GridInfo.h - abstract class to get grid information.
// .SECTION Description
// Abstract class to get grid information. CMB will implement a version
// of this to get the grid information into SMTK from CMB withouth
// introducing a dependency on CMB.
// .SECTION See Also

#ifndef __smtk_model_GridInfo_h
#define __smtk_model_GridInfo_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <map>
#include <vector>

namespace smtk
{
  namespace model
  {
    class SMTKCORE_EXPORT GridInfo
    {
    public:
      virtual void getGroupCellIds(int groupId, std::vector<int>& cellIds) = 0;
      virtual double getGroupArea(int groupId) = 0;
      virtual void getAgentsInGroupDomain(
        int groupId, int numberOfAgents,
        std::vector<std::pair<int, std::pair<double, double> > >& locations) = 0;

      GridInfo() {}
      virtual ~GridInfo() {}
    };
  }
}

#endif /* __smtk_model_GridInfo_h */
