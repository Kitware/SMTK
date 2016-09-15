//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqGenerateContoursDialog - .
// .SECTION Description
// .SECTION Caveats

#ifndef __smtk_pq_GenerateContoursDialog_h
#define __smtk_pq_GenerateContoursDialog_h

#include "smtk/extension/paraview/widgets/Exports.h"
#include "smtk/PublicPointerDefs.h"
#include <QDialog>
#include <QDoubleValidator>

class pqDataRepresentation;
class pqRenderView;
class pqPipelineSource;
class QDoubleValidator;
class QIntValidator;
class QProgressDialog;

namespace Ui
{
  class qtGenerateContoursDialog;
};

class SMTKPQWIDGETSEXT_EXPORT pqGenerateContoursDialog : public QDialog
{
  Q_OBJECT
public:
  pqGenerateContoursDialog(pqPipelineSource*,
    const bool& mapScalars2Colors,
    QWidget *parent = NULL, Qt::WindowFlags flags= 0);
  virtual ~pqGenerateContoursDialog();

  int exec();
  void close();

signals:
  void contoursAccepted(pqPipelineSource*);

protected slots:
  void generateContours();
  void onAccecptContours();
  void onCancel();
  void updateProgress(const QString&, int progress);
  void updateContourButtonStatus();

  void onOpacityChanged(int opacity);

protected:
  void setupProgressBar(QWidget* progressWidget);
  void disableWhileProcessing();

  Ui::qtGenerateContoursDialog *InternalWidget;

  QDialog *MainDialog;
  pqRenderView *RenderView;
  pqPipelineSource *ImageSource;
  pqPipelineSource *ImageMesh;
  pqDataRepresentation* ImageRepresentation;

  pqDataRepresentation *ContourRepresentation;
  pqPipelineSource *ContourSource;
  pqPipelineSource *CleanPolyLines;
  QString ProgressMessage;
  bool ProgressMessagesMustMatch;
  double ContourValue;
  double MinimumLineLength;
  bool UseRelativeLineLength;
  QDoubleValidator *ContourValidator;
  QProgressDialog *Progress;
  smtk::weak_ptr<smtk::model::Operator> m_edgeOp;
};

//need a sublcass validator, since QDoubleValidator is really shitty
class SMTKPQWIDGETSEXT_EXPORT InternalDoubleValidator : public QDoubleValidator
{
  Q_OBJECT
public:
    InternalDoubleValidator(QObject * parent);
    virtual void fixup(QString &input) const;
};

#endif
