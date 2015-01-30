//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionIOJSON_h
#define __smtk_model_SessionIOJSON_h

#include "smtk/model/SessionIO.h"

struct cJSON;

namespace smtk {
  namespace model {

/**\brief A base class for delegating session I/O to/from JSON.
  *
  * Subclasses should implement both
  * importJSON and exportJSON methods.
  */
class SMTKCORE_EXPORT SessionIOJSON : public SessionIO
{
public:
  smtkTypeMacro(SessionIOJSON);

  virtual int importJSON(ManagerPtr modelMgr, cJSON* sessionRec);
  virtual int exportJSON(ManagerPtr modelMgr, cJSON* sessionRec);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_SessionIOJSON_h
