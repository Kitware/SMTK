//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef imageFeatureExtractorWidget_h
#define imageFeatureExtractorWidget_h

#include "smtk/bridge/polygon/qt/Exports.h"
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>

#include <QDialog>

// Forward Qt class declarations
class Ui_imageFeatureExtractor;

class SMTKPOLYGONQTEXT_EXPORT imageFeatureExtractorWidget : public QDialog
{
  Q_OBJECT
public:
  friend class vtkGrabLeftMousePressCallback;
  friend class vtkGrabMouseMoveCallback;
  friend class vtkGrabLeftMouseReleasedCallback;
  // Constructor/Destructor
  imageFeatureExtractorWidget();
  ~imageFeatureExtractorWidget() override;

  void setImage(std::string inputImage);
  vtkSmartPointer<vtkPolyData> getPolydata();

//public slots:

//  virtual void slotExit();

//signals:
//  void send(vtkSmartPointer<vtkPolyData>);

protected slots:
  void saveMask();
  void saveLines();
  void loadLines();
  void run();
  void clear();
  void pointSize(int i);
  void numberOfIterations(int j);
  void showPossibleLabel(bool b);
  void setTransparency(int t);
  void setDrawMode(int m);
  void setAlgorithm(int a);
  void setFGFilterSize(QString const& f);
  void setBGFilterSize(QString const& b);
  //void accept();

private:

  class Internal;
  Internal *internal;
  // Designer form
  Ui_imageFeatureExtractor *ui;
};

#endif
