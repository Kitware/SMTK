#include "smtk/model/Tessellation.h"

namespace smtk {
  namespace model {

Tessellation::Tessellation()
{
}

int Tessellation::AddCoords(double* a)
{
  std::vector<double>::size_type ipt = this->Coords.size();
  for (int i = 0; i < 3; ++i)
    {
    this->Coords.push_back(a[i]);
    }
  return static_cast<int>(ipt / 3);
}

Tessellation& Tessellation::AddCoords(double x, double y, double z)
{
  this->Coords.push_back(x);
  this->Coords.push_back(y);
  this->Coords.push_back(z);
  return *this;
}

Tessellation& Tessellation::AddPoint(double* a)
{
  int ai = this->AddCoords(a);
  std::vector<int> pconn;
  pconn.push_back(512); // Extension of three.js file format for "Vertex" glyph
  pconn.push_back(ai);
  this->Conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::AddLine(double* a, double* b)
{
  int ai = this->AddCoords(a);
  int bi = this->AddCoords(b);
  this->Conn.push_back(513); // Extension of three.js file format for an Edge cell.
  this->Conn.push_back(ai);
  this->Conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::AddTriangle(double* a, double* b, double* c)
{
  int ai = this->AddCoords(a);
  int bi = this->AddCoords(b);
  int ci = this->AddCoords(c);
  this->Conn.push_back(0); // A triangle in three.js format.
  this->Conn.push_back(ai);
  this->Conn.push_back(bi);
  this->Conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::AddPoint(int ai)
{
  this->Conn.push_back(512);
  this->Conn.push_back(ai);
  return *this;
}

Tessellation& Tessellation::AddLine(int ai, int bi)
{
  this->Conn.push_back(513);
  this->Conn.push_back(ai);
  this->Conn.push_back(bi);
  return *this;
}

Tessellation& Tessellation::AddTriangle(int ai, int bi, int ci)
{
  this->Conn.push_back(0);
  this->Conn.push_back(ai);
  this->Conn.push_back(bi);
  this->Conn.push_back(ci);
  return *this;
}

Tessellation& Tessellation::Reset()
{
  this->Conn.clear();
  this->Coords.clear();
  return *this;
}

  } // model namespace
} // smtk namespace
