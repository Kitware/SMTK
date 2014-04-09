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
// .NAME qtModelView - a tree view of smtk model.
// .SECTION Description
// .SECTION Caveats

#ifndef __smtk_attribute_qtNewAttributeWidget_h
#define __smtk_attribute_qtNewAttributeWidget_h

#include <QDialog>

#include "smtk/QtSMTKExports.h"
#include "smtk/PublicPointerDefs.h"

namespace smtk
{
  namespace attribute
  {
  class QTSMTK_EXPORT qtNewAttributeWidget : public QDialog
  {
    Q_OBJECT
    typedef QDialog Superclass;

  public:
    qtNewAttributeWidget(QWidget* parent = 0);
    virtual ~qtNewAttributeWidget();

    QString attributeName() const;
    virtual void setBaseWidget(QWidget* baseWidget);
    virtual int showWidget(const QString& name);

  private:
    qtNewAttributeWidget(const qtNewAttributeWidget&); // Not implemented.
    void operator=(const qtNewAttributeWidget&); // Not implemented.

    class PIMPL;
    PIMPL *Private;

    }; // class
  }; // namespace attribute
}; // namespace smtk

#endif
