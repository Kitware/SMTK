/*=========================================================================

  Program:   ERDC Hydro
  Module:    qtExportDialog.h

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
// .NAME qtExportDialog - Options for exporting  simulation file.
// .SECTION Description
// .SECTION Caveats


#ifndef __slctk_attribute_qtExportDialog_h
#define __slctk_attribute_qtExportDialog_h


#include <QObject>
#include "AttributeExports.h"
#include <QMap>
#include <QPointer>
#include <QStringList>
#include "attribute/PublicPointerDefs.h";

class QDialog;
class pqServer;
class vtkXMLDataParser;
class vtkSBAnalysisContainer;
class erdcCMBModel;

namespace Ui
{
  class  qtqtExportDialog;
}; // class


namespace slctk
{
  namespace attribute
  {
    class SLCTKATTRIBUTE_EXPORT qtExportDialog : public QObject
    {
      Q_OBJECT

    public:
      qtExportDialog(vtkSBAnalysisContainer *container);
      virtual ~qtExportDialog();

      QString getFileName() const;
      QString getFormatFileName() const;
      void setRootDataContainer(slctk::AttributeItemPtr root);
      void setCmbModel(erdcCMBModel *model);

      void setActiveServer(pqServer* server);
      int exec();

    protected slots:
      void accept();
      void cancel();
      void displayFileBrowser();
      void displayFormatFileBrowser();
      void formatFileChanged();
      //void outputFileChanged();
      void validate();

    protected:
      void setAcceptable(bool state);

    private:

      int Status;
      Ui::qtqtExportDialog *Dialog;
      QDialog *MainDialog;
      QPointer<pqServer> ActiveServer;
      vtkXMLDataParser *Parser;
      vtkSBDataContainer *RootDataContainer;
      static QString LastFormatFileParsed;
      static QString LastExtension;
      int ParserStatus;
      erdcCMBModel *CMBModel;
    }; // class
  }; // namespace attribute
}; // namespace slctk



#endif /* __slctk_attribute_qtExportDialog_h */
