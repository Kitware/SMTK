/*=========================================================================

  Program:   ERDC Hydro
  Module:    qtTableWidget.h

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
// .NAME qtTableWidget - a Attribute expression Tree widget.
// .SECTION Description
//  A convenience QTableWidget with extra features:
//  1.  Automatic size hints based on contents
//  2.  A check box added in a header if items have check boxes
//  3.  Navigation through columns of top level items on Tab.
//  4.  Signal emitted when user navigates beyond end of the table giving an 
//      opportunity to the lister to grow the table.
//  5.  Customized Drag-n-Drop
// .SECTION Caveats

#ifndef _qtTableWidget_h
#define _qtTableWidget_h

#include "AttributeExports.h"
#include <QTableWidget>

class QKeyEvent;

class SLCTKATTRIBUTE_EXPORT qtTableWidget : public QTableWidget
{
  Q_OBJECT
  
public:
  
  qtTableWidget(QWidget* p = NULL);
  ~qtTableWidget();

  QModelIndexList getSelectedIndexes() const
    {
    return this->selectedIndexes();
    }
public slots:

signals:
  void keyPressed(QKeyEvent*);
  
protected slots:
  virtual void keyPressEvent(QKeyEvent*);

};

#endif // !_qtTableWidget_h

