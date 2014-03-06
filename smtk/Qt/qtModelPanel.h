/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME qtModelPanel - the ui panel for smtk model.
// .SECTION Description
// .SECTION Caveats

#ifndef _qtModelPanel_h
#define _qtModelPanel_h

#include <QtGui/QWidget.h>
#include "smtk/QtSMTKExports.h"
#include "smtk/Qt/qtModelView.h"
#include "smtk/util/UUID.h"

namespace smtk {
  namespace model {

class QTSMTK_EXPORT qtModelPanel : public QWidget
{
  Q_OBJECT

public:
  qtModelPanel(QWidget* p = NULL);
  ~qtModelPanel();

  smtk::model::qtModelView* getModelView();

public slots:
  void onAddDomainset();
  void onAddBC();
  void onRemove();

signals:

protected:

private:
  class qInternal;
  qInternal* Internal;

};

  } // namespace model
} // namespace smtk

#endif // !_qtModelPanel_h
