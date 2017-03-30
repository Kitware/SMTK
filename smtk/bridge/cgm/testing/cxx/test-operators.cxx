//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/io/SaveJSON.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/Session.h"
#include "smtk/model/SessionRef.h"
#include "smtk/model/Volume.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/AutoInit.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "clpp/parser.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <algorithm>
#include <fstream>

using namespace smtk::io;
using namespace smtk::model;
using namespace smtk::common;

smtkComponentInitMacro(smtk_cgm_session);
smtkComponentInitMacro(smtk_cgm_boolean_union_operator);
smtkComponentInitMacro(smtk_cgm_create_sphere_operator);
smtkComponentInitMacro(smtk_cgm_create_prism_operator);

int usage(int errCode = 0, const std::string& msg = std::string())
{
  // I. Basic usage info.
  std::cout
    << "Usage:\n"
    << "  test-operators [options]\n"
    << "where options may include\n"
    << "  -sph-radius=<radius>       Specifies the radius of a sphere.\n"
    << "  -sph-center=<cx,cy,cz>     Specifies the center of a sphere.\n"
    << "  -sph-hollow=<radius>       Specifies the radius of a portion of the sphere to hollow "
       "out.\n"
    << "  -pri-height=<height>       Specifies the height of a prismatic regular polygon.\n"
    << "  -pri-number=<num sides>    Specifies the number of sides of a prismatic regular "
       "polygon.\n"
    << "  -pri-major=<major radius>  Specifies the major radius of a prismatic regular polygon.\n"
    << "  -pri-minor=<minor radius>  Specifies the minor radius of a prismatic regular polygon.\n"
    << "\n";

  // II. Print user-specified message and return exit code.
  if (!msg.empty())
    std::cout << msg << "\n";

  return errCode;
}

// A struct to hold options passed to the model server.
struct ProgOpts
{
  ProgOpts()
    : m_sphRadius(1.0)
    , m_sphCenter(3, 0.)
    , m_sphHollow(0.0)
    , m_priHeight(0.2)
    , m_priNumber(3)
    , m_priMajor(1.0)
    , m_priMinor(1.0)
    , m_relErr("0.001")
    , m_angErr("2.0")
    , m_printHelp(false)
  {
  }

  void setSphereRadius(double x) { this->m_sphRadius = x; }
  void setSphereCenter(const std::vector<double>& x) { this->m_sphCenter = x; }
  void setSphereCenterX(double x) { this->m_sphCenter[0] = x; }
  void setSphereCenterY(double y) { this->m_sphCenter[1] = y; }
  void setSphereCenterZ(double z) { this->m_sphCenter[2] = z; }
  void setSphereHollow(double x) { this->m_sphHollow = x; }
  void setPrismHeight(double x) { this->m_priHeight = x; }
  void setPrismNumber(int n) { this->m_priNumber = n; }
  void setPrismMajor(double x) { this->m_priMajor = x; }
  void setPrismMinor(double x) { this->m_priMinor = x; }
  void setRelativeChordError(const std::string& x) { this->m_relErr = x; }
  void setAngleError(const std::string& x) { this->m_angErr = x; }
  void setPrintHelp() { this->m_printHelp = true; }

  double sphereRadius() { return this->m_sphRadius; }
  std::vector<double> sphereCenter() { return this->m_sphCenter; }
  double sphereHollow() { return this->m_sphHollow; }
  double prismHeight() { return this->m_priHeight; }
  int prismNumber() { return this->m_priNumber; }
  double prismMajor() { return this->m_priMajor; }
  double prismMinor() { return this->m_priMinor; }
  std::string relativeChordError() { return this->m_relErr; }
  std::string angleError() { return this->m_angErr; }
  bool printHelp() { return this->m_printHelp; }

  double m_sphRadius;
  std::vector<double> m_sphCenter;
  double m_sphHollow;
  double m_priHeight;
  int m_priNumber;
  double m_priMajor;
  double m_priMinor;
  std::string m_relErr;
  std::string m_angErr;
  bool m_printHelp;
};

int main(int argc, char* argv[])
{
  ProgOpts opts;
  clpp::command_line_parameters_parser args;
  try
  {
    args.add_parameter("-sph-radius", &opts, &ProgOpts::setSphereRadius);
    args.add_parameter("-sph-cx", &opts, &ProgOpts::setSphereCenterX);
    args.add_parameter("-sph-cy", &opts, &ProgOpts::setSphereCenterY);
    args.add_parameter("-sph-cz", &opts, &ProgOpts::setSphereCenterZ);
    args.add_parameter("-sph-hollow", &opts, &ProgOpts::setSphereHollow);
    args.add_parameter("-pri-height", &opts, &ProgOpts::setPrismHeight);
    args.add_parameter("-pri-number", &opts, &ProgOpts::setPrismNumber);
    args.add_parameter("-pri-major", &opts, &ProgOpts::setPrismMajor);
    args.add_parameter("-pri-minor", &opts, &ProgOpts::setPrismMinor);
    args.add_parameter("-err-chord", &opts, &ProgOpts::setRelativeChordError);
    args.add_parameter("-err-angle", &opts, &ProgOpts::setAngleError);
    args.add_parameter("-help", &opts, &ProgOpts::setPrintHelp);
    args.parse(argc, argv);
  }
  catch (std::exception& e)
  {
    return usage(1, e.what());
  }
  if (opts.printHelp())
  {
    return usage(0);
  }

  Manager::Ptr mgr = Manager::create();
  Session::Ptr brg = mgr->createSessionOfType("cgm");
  mgr->registerSession(brg);
  StringList err(1);
  err[0] = opts.relativeChordError();
  brg->setup("tessellation maximum relative chord error", err);
  err[0] = opts.angleError();
  brg->setup("tessellation maximum angle error", err);
  Operator::Ptr op;
  OperatorResult result;

  op = brg->op("create sphere");
  op->findDouble("radius")->setValue(opts.sphereRadius());
  op->findDouble("center")->setValue(0, opts.sphereCenter()[0]);
  op->findDouble("center")->setValue(1, opts.sphereCenter()[1]);
  op->findDouble("center")->setValue(2, opts.sphereCenter()[2]);
  op->findDouble("inner radius")->setValue(opts.sphereHollow());
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
  {
    std::cerr << "Sphere Fail\n";
    return 1;
  }
  Model sphere = result->findModelEntity("created")->value();

  op = brg->op("create prism");
  op->findDouble("height")->setValue(opts.prismHeight());
  op->findInt("number of sides")->setValue(opts.prismNumber());
  op->findDouble("major radius")->setValue(opts.prismMajor());
  op->findDouble("minor radius")->setValue(opts.prismMinor());
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
  {
    std::cerr << "Prism Fail\n";
    return 1;
  }
  Model prism = result->findModelEntity("created")->value();

  Models operands;
  operands.push_back(sphere);
  operands.push_back(prism);
  SessionRef bs(mgr, brg->sessionId());
  StringList validOps = bs.operatorsForAssociation(operands);
  test(!validOps.empty(), "Expected at least 1 operator (union) that can act on model entities.");
  test(std::find(validOps.begin(), validOps.end(), "union") != validOps.end(),
    "Expected the union operator to be valid.");

  op = brg->op("union");
  op->ensureSpecification();
  test(op->associateEntity(sphere), "Could not associate sphere to union operator");
  test(op->associateEntity(prism), "Could not associate prism to union operator");
  result = op->operate();
  if (result->findInt("outcome")->value() != OPERATION_SUCCEEDED)
  {
    std::cerr << "Union Fail\n";
    return 1;
  }

  smtk::attribute::ModelEntityItem::Ptr bodies = result->findModelEntity("modified");
  std::cout << "Created " << bodies->value().flagSummary() << "\n";
  std::cout << "   with " << bodies->value().as<Model>().cells().size() << " cells\n";

  return 0;
}
