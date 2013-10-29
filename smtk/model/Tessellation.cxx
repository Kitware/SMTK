#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

Tessellation::Tessellation()
{
}

int Tessellation::addCoords(double* a)
{
  std::vector<double>::size_type ipt = this->coords.size();
  for (int i = 0; i < 3; ++i)
    {
    this->coords.push_back(a[i]);
    }
  return static_cast<int>(ipt / 3);
}

Tessellation& Tessellation::addCoords(double x, double y, double z)
{
  this->coords.push_back(x);
  this->coords.push_back(y);
  this->coords.push_back(z);
  return *this;
}

Tessellation& Tessellation::addPoint(double* a)
{
  int ai = this->addCoords(a);
  std::vector<int> pconn;
  pconn.push_back(512); // Extension of three.js file format for "Vertex" glyph
  pconn.push_back(ai);
  this->conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::addLine(double* a, double* b)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  this->conn.push_back(513); // Extension of three.js file format for an Edge cell.
  this->conn.push_back(ai);
  this->conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(double* a, double* b, double* c)
{
  int ai = this->addCoords(a);
  int bi = this->addCoords(b);
  int ci = this->addCoords(c);
  this->conn.push_back(0); // A triangle in three.js format.
  this->conn.push_back(ai);
  this->conn.push_back(bi);
  this->conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::addPoint(int ai)
{
  this->conn.push_back(512);
  this->conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::addLine(int ai, int bi)
{
  this->conn.push_back(513);
  this->conn.push_back(ai);
  this->conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::addTriangle(int ai, int bi, int ci)
{
  this->conn.push_back(0);
  this->conn.push_back(ai);
  this->conn.push_back(bi);
  this->conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::reset()
{
  this->conn.clear();
  this->coords.clear();
  return *this;
}

  } // model namespace
} // smtk namespace
