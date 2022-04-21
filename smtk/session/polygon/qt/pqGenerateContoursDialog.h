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

#ifndef smtk_pq_GenerateContoursDialog_h
#define smtk_pq_GenerateContoursDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/session/polygon/qt/Exports.h"
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

class SMTKPOLYGONQTEXT_EXPORT pqGenerateContoursDialog : public QDialog
{
  Q_OBJECT
public:
  pqGenerateContoursDialog(
    pqPipelineSource*,
    const bool& mapScalars2Colors,
    QWidget* parent = nullptr,
    Qt::WindowFlags flags = {});
  ~pqGenerateContoursDialog() override;

  int exec() override;
  void close();

Q_SIGNALS:
  void contoursAccepted(pqPipelineSource*);

protected Q_SLOTS:
  void generateContours();
  void onAccecptContours();
  void onCancel();
  void updateProgress(const QString&, int progress);
  void updateContourButtonStatus();

  void onOpacityChanged(int opacity);
  void onMapScalars(int mapScalars2Colors);

  void resetCamera();

protected:
  void setupProgressBar(QWidget* progressWidget);
  void disableWhileProcessing();

  Ui::qtGenerateContoursDialog* InternalWidget;

  QDialog* MainDialog;
  pqRenderView* RenderView;
  pqPipelineSource* ImageSource;
  pqPipelineSource* ImageMesh;
  pqDataRepresentation* ImageRepresentation;

  pqDataRepresentation* ContourRepresentation;
  pqPipelineSource* ContourSource;
  pqPipelineSource* CleanPolyLines;
  QString ProgressMessage;
  bool ProgressMessagesMustMatch;
  double ContourValue;
  double MinimumLineLength;
  bool UseRelativeLineLength;
  QDoubleValidator* ContourValidator;
  QProgressDialog* Progress;
};

//need a sublcass validator, since QDoubleValidator is really shitty
class InternalDoubleValidator : public QDoubleValidator
{
  Q_OBJECT
public:
  InternalDoubleValidator(QObject* parent);
  void fixup(QString& input) const override;
};

#endif
