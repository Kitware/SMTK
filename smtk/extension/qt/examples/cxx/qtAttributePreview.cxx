//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME qtAttributePreview - Standalone attribute editor
// .SECTION Description
// .SECTION See Also

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/AutoInit.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"
#include "smtk/view/Configuration.h"

#ifdef VTK_SESSION
#include "smtk/session/vtk/Resource.h"
#include "smtk/session/vtk/operators/Import.h"
#include "smtk/session/vtk/operators/Read.h"
#endif

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFrame>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/lexical_cast.hpp>
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

int main(int argc, char* argv[])
{
  // Instantiate Qt application
  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("Load attribute template and display editor panel");
  parser.addHelpOption();
  parser.addPositionalArgument("attribute_filename", "Attribute file (.sbt)");
  parser.addOptions({ { { "o", "output-file" }, "Output attribute file (.sbi)", "main" },
#ifdef VTK_SESSION
    { { "m", "model-file" }, "Model file (using vtk session)", "main" },
#endif
    { { "v", "view-name" }, "Specific View to display", "main" } });

  parser.process(app);
  if (!parser.parse(QCoreApplication::arguments()))
  {
    qDebug() << parser.errorText();
    return 1;
  }

  const QStringList positionalArguments = parser.positionalArguments();
  if (positionalArguments.isEmpty())
  {
    qCritical() << "\nERROR: Argument 'input_filename' missing.";
    parser.showHelp();
    return 1;
  }
  if (positionalArguments.size() > 1)
  {
    qCritical("\nERROR: More than one positional argument specified.");
    parser.showHelp();
    return 1;
  }

  // Instantiate and load attribute resource
  smtk::attribute::ResourcePtr attResource = smtk::attribute::Resource::create();
  std::string inputPath = positionalArguments.first().toStdString();
  qInfo() << "Loading simulation file:" << inputPath.c_str();
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;
  bool err = reader.read(attResource, inputPath, true, inputLogger);
  if (err)
  {
    qCritical() << "Error loading simulation file -- exiting"
                << "\n";
    qCritical() << QString::fromStdString(inputLogger.convertToString());
    return 1;
  }

#ifdef VTK_SESSION
  // Load model if specified
  smtk::session::vtk::Resource::Ptr modelResource;
  if (parser.isSet("model-file"))
  {
    QString modelFile = parser.value("model-file");
    qInfo() << "Loading model file" << modelFile;

    smtk::operation::Operation::Ptr loadOp;

    // If model file is resource (.smtk), use read operator; otherwise use load operator
    QFileInfo fi(modelFile);
    QString ext = fi.suffix();
    if (ext == "smtk")
    {
      qDebug() << "Using Read operator";
      loadOp = smtk::session::vtk::Read::create();
    }
    else
    {
      loadOp = smtk::session::vtk::Import::create();
    }

    if (loadOp == nullptr)
    {
      qCritical("No vtk model-input operator");
      return 1;
    }

    loadOp->parameters()->findFile("filename")->setValue(modelFile.toStdString());

    // Execute the operation
    smtk::operation::Operation::Result loadOpResult = loadOp->operate();

    // Retrieve the resulting resource
    smtk::attribute::ResourceItemPtr resourceItem =
      std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
        loadOpResult->findResource("resource"));

    modelResource = std::dynamic_pointer_cast<smtk::session::vtk::Resource>(resourceItem->value());

    if (!modelResource)
    {
      qCritical() << "ERROR: Model file failed to import.\n";
      return 1;
    }

    // If model resource loaded, then associate it to the attribute resource
    attResource->associate(modelResource);
  }
#endif

  // Find view if specified
  smtk::view::ConfigurationPtr root;
  if (parser.isSet("view-name"))
  {
    QString viewName = parser.value("view-name");
    root = attResource->findView(viewName.toStdString());
    if (!root)
    {
      qCritical() << "ERROR: View \"" << viewName << "\" not found";
      return 1;
    }
  }
  else
  {
    // Use top-level view by default
    root = attResource->findTopLevelView();
  }

  // If resource contains no views, create InstancedView by default
  if (!root)
  {
    root = smtk::view::Configuration::New("Group", "RootView");
    root->details().setAttribute("TopLevel", "true");
    attResource->addView(root);
    smtk::view::Configuration::Component& temp = root->details().addChild("Views");
    (void)temp;
    int viewsIndex = root->details().findChild("Views");

    //  Add instances of all non-abstract attribute definitions
    smtk::view::Configuration::Component& viewsComp = root->details().child(viewsIndex);
    std::vector<smtk::attribute::DefinitionPtr> defs;
    std::vector<smtk::attribute::DefinitionPtr> baseDefinitions;
    attResource->findBaseDefinitions(baseDefinitions);
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator baseIter;

    for (baseIter = baseDefinitions.begin(); baseIter != baseDefinitions.end(); baseIter++)
    {
      std::vector<smtk::attribute::DefinitionPtr> derivedDefs;
      attResource->findAllDerivedDefinitions(*baseIter, true, derivedDefs);
      defs.insert(defs.end(), derivedDefs.begin(), derivedDefs.end());
    }

    // Instantiate attribute for each concrete definition
    std::vector<smtk::attribute::DefinitionPtr>::const_iterator defIter;
    for (defIter = defs.begin(); defIter != defs.end(); defIter++)
    {
      smtk::view::ConfigurationPtr instanced =
        smtk::view::Configuration::New("Instanced", (*defIter)->type());

      smtk::view::Configuration::Component& comp =
        instanced->details().addChild("InstancedAttributes").addChild("Att");
      comp.setAttribute("Type", (*defIter)->type());
      comp.setAttribute("Name", (*defIter)->type());
      attResource->addView(instanced);
      smtk::attribute::AttributePtr instance = attResource->createAttribute((*defIter)->type());
      comp.setContents(instance->name());
      viewsComp.addChild("View").setAttribute("Title", (*defIter)->type());
    }
  }

  // Instantiate smtk's qtUIManager
  smtk::extension::qtUIManager* uiManager = new smtk::extension::qtUIManager(attResource);

  // Instantiate empty widget as containter for qtUIManager
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout();
  widget->setLayout(layout);

  bool useInternalFileBrowser = true;
  if (parser.isSet("view-name"))
  {
    // Create simple panel for specified views (aesthetics)
    QFrame* frame = new QFrame();
    frame->setFrameShadow(QFrame::Raised);
    frame->setFrameShape(QFrame::Panel);

    QVBoxLayout* frameLayout = new QVBoxLayout();
    frame->setLayout(frameLayout);
    uiManager->setSMTKView(root, frame, useInternalFileBrowser);
    widget->layout()->addWidget(frame);
  }
  else
  {
    // Use standard logic for top-level view
    uiManager->setSMTKView(root, widget, useInternalFileBrowser);
  }

  widget->show();
  int retcode = app.exec();
  QCoreApplication::processEvents();

  if (parser.isSet("output-file"))
  {
    QString outputFile = parser.value("output-file");
    qInfo() << "Writing output (attribute) file:" << outputFile;
    smtk::io::AttributeWriter writer;
    smtk::io::Logger outputLogger;
    bool outputErr = writer.write(attResource, outputFile.toStdString(), outputLogger);
    if (outputErr)
    {
      qCritical() << "ERROR writing simulation file";
      return 1;
    }
  }

  // Done
  return retcode;
}
