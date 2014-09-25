/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelPluginStarter.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
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

#ifndef __pqCMBModelPluginStarter_h
#define __pqCMBModelPluginStarter_h

#include <QObject>
#include "cmbSystemConfig.h"

class pqCMBModelPluginStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;
public:
  pqCMBModelPluginStarter(QObject* p=0);
  ~pqCMBModelPluginStarter();

  // Callback for shutdown.
  void onShutdown();

  // Callback for startup.
  void onStartup();

private:
  pqCMBModelPluginStarter(const pqCMBModelPluginStarter&); // Not implemented.
  void operator=(const pqCMBModelPluginStarter&); // Not implemented.
};

#endif
