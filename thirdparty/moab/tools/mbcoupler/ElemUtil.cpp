#include <iostream>
#include <limits>
#include <assert.h>

#include "ElemUtil.hpp"
#include "moab/BoundBox.hpp"

namespace moab {
namespace ElemUtil {

  bool debug = false;

/**\brief Class representing a 3-D mapping function (e.g. shape function for volume element) */
class VolMap {
  public:
      /**\brief Return $\vec \xi$ corresponding to logical center of element */
    virtual CartVect center_xi() const = 0;
      /**\brief Evaluate mapping function (calculate $\vec x = F($\vec \xi)$ )*/
    virtual CartVect evaluate( const CartVect& xi ) const = 0;
      /**\brief Evaluate Jacobian of mapping function */
    virtual Matrix3 jacobian( const CartVect& xi ) const = 0;
      /**\brief Evaluate inverse of mapping function (calculate $\vec \xi = F^-1($\vec x)$ )*/
    bool solve_inverse( const CartVect& x, CartVect& xi, double tol ) const ;
};


bool VolMap::solve_inverse( const CartVect& x, CartVect& xi, double tol ) const
{
  const double error_tol_sqr = tol*tol;
  double det;
  xi = center_xi();
  CartVect delta = evaluate(xi) - x;
  Matrix3 J;
  while (delta % delta > error_tol_sqr) {
    J = jacobian(xi);
    det = J.determinant();
    if (det < std::numeric_limits<double>::epsilon())
      return false;
    xi -= J.inverse(1.0/det) * delta;
    delta = evaluate( xi ) - x;
  }
  return true;
}

/**\brief Shape function for trilinear hexahedron */
class LinearHexMap : public VolMap {
  public:
    LinearHexMap( const CartVect* corner_coords ) : corners(corner_coords) {}
    virtual CartVect center_xi() const;
    virtual CartVect evaluate( const CartVect& xi ) const;
    virtual double evaluate_scalar_field( const CartVect& xi, const double *f_vals ) const;
    virtual Matrix3 jacobian( const CartVect& xi ) const;
  private:
    const CartVect* corners;
    static const double corner_xi[8][3];
};

const double LinearHexMap::corner_xi[8][3] = { { -1, -1, -1 },
                                               {  1, -1, -1 },
                                               {  1,  1, -1 },
                                               { -1,  1, -1 },
                                               { -1, -1,  1 },
                                               {  1, -1,  1 },
                                               {  1,  1,  1 },
                                               { -1,  1,  1 } };
CartVect LinearHexMap::center_xi() const
  { return CartVect(0.0); }

CartVect LinearHexMap::evaluate( const CartVect& xi ) const
{
  CartVect x(0.0);
  for (unsigned i = 0; i < 8; ++i) {
    const double N_i = (1 + xi[0]*corner_xi[i][0])
                     * (1 + xi[1]*corner_xi[i][1])
                     * (1 + xi[2]*corner_xi[i][2]);
    x += N_i * corners[i];
  }
  x *= 0.125;
  return x;
}

double LinearHexMap::evaluate_scalar_field( const CartVect& xi, const double *f_vals ) const
{
  double f(0.0);
  for (unsigned i = 0; i < 8; ++i) {
    const double N_i = (1 + xi[0]*corner_xi[i][0])
                     * (1 + xi[1]*corner_xi[i][1])
                     * (1 + xi[2]*corner_xi[i][2]);
    f += N_i * f_vals[i];
  }
  f *= 0.125;
  return f;
}

Matrix3 LinearHexMap::jacobian( const CartVect& xi ) const
{
  Matrix3 J(0.0);
  for (unsigned i = 0; i < 8; ++i) {
    const double   xi_p = 1 + xi[0]*corner_xi[i][0];
    const double  eta_p = 1 + xi[1]*corner_xi[i][1];
    const double zeta_p = 1 + xi[2]*corner_xi[i][2];
    const double dNi_dxi   = corner_xi[i][0] * eta_p * zeta_p;
    const double dNi_deta  = corner_xi[i][1] *  xi_p * zeta_p;
    const double dNi_dzeta = corner_xi[i][2] *  xi_p *  eta_p;
    J(0,0) += dNi_dxi   * corners[i][0];
    J(1,0) += dNi_dxi   * corners[i][1];
    J(2,0) += dNi_dxi   * corners[i][2];
    J(0,1) += dNi_deta  * corners[i][0];
    J(1,1) += dNi_deta  * corners[i][1];
    J(2,1) += dNi_deta  * corners[i][2];
    J(0,2) += dNi_dzeta * corners[i][0];
    J(1,2) += dNi_dzeta * corners[i][1];
    J(2,2) += dNi_dzeta * corners[i][2];
  }
  return J *= 0.125;
}


bool nat_coords_trilinear_hex( const CartVect* corner_coords,
                               const CartVect& x,
                               CartVect& xi,
                               double tol )
{
  return LinearHexMap( corner_coords ).solve_inverse( x, xi, tol );
}
//
// nat_coords_trilinear_hex2
//  Duplicate functionality of nat_coords_trilinear_hex using hex_findpt
//
void nat_coords_trilinear_hex2(const CartVect hex[8],
                               const CartVect& xyz,
                               CartVect &ncoords,
                               double etol)

{
  const int ndim = 3;
  const int nverts = 8;
  const int vertMap[nverts] = {0,1,3,2, 4,5,7,6}; //Map from nat to lex ordering

  const int n = 2; //linear
  real coords[ndim*nverts]; //buffer

  real *xm[ndim];
  for(int i=0; i<ndim; i++)
    xm[i] = coords + i*nverts;

  //stuff hex into coords
  for(int i=0; i<nverts; i++){
    real vcoord[ndim];
    hex[i].get(vcoord);

    for(int d=0; d<ndim; d++)
      coords[d*nverts + vertMap[i]] = vcoord[d];

  }

  double dist = 0.0;
  ElemUtil::hex_findpt(xm, n, xyz, ncoords, dist);
  if (3*EPS < dist) {
      // outside element, set extremal values to something outside range
    for (int j = 0; j < 3; j++) {
      if (ncoords[j] < (-1.0-etol) || ncoords[j] > (1.0+etol))
        ncoords[j] *= 10;
    }
  }

}
bool point_in_trilinear_hex(const CartVect *hex,
                            const CartVect& xyz,
                            double etol)
{
  CartVect xi;
  return nat_coords_trilinear_hex( hex, xyz, xi, etol )
      && (fabs(xi[0]-1.0) < etol)
      && (fabs(xi[1]-1.0) < etol)
      && (fabs(xi[2]-1.0) < etol) ;
}


bool point_in_trilinear_hex(const CartVect *hex,
                            const CartVect& xyz,
                            const CartVect& box_min,
                            const CartVect& box_max,
                            double etol)
{
    // all values scaled by 2 (eliminates 3 flops)
  const CartVect mid = box_max + box_min;
  const CartVect dim = box_max - box_min;
  const CartVect pt = 2*xyz - mid;
  return (fabs(pt[0] - dim[0]) < etol) &&
         (fabs(pt[1] - dim[1]) < etol) &&
         (fabs(pt[2] - dim[2]) < etol) &&
         point_in_trilinear_hex( hex, xyz, etol );
}


// Wrapper to James Lottes' findpt routines
// hex_findpt
// Find the parametric coordinates of an xyz point inside
// a 3d hex spectral element with n nodes per dimension
// xm: coordinates fields, value of x,y,z for each of then n*n*n gauss-lobatto nodes. Nodes are in lexicographical order (x is fastest-changing)
// n: number of nodes per dimension -- n=2 for a linear element
// xyz: input, point to find
// rst: output: parametric coords of xyz inside the element. If xyz is outside the element, rst will be the coords of the closest point
// dist: output: distance between xyz and the point with parametric coords rst
/*extern "C"{
#include "types.h"
#include "poly.h"
#include "tensor.h"
#include "findpt.h"
#include "extrafindpt.h"
#include "errmem.h"
}*/

void hex_findpt(real *xm[3],
                int n,
                CartVect xyz,
                CartVect &rst,
                double &dist)
{

  //compute stuff that only depends on the order -- could be cached
  real *z[3];
  lagrange_data ld[3];
  opt_data_3 data;

  //triplicates
  for(int d=0; d<3; d++){
    z[d] = tmalloc(real, n);
    lobatto_nodes(z[d], n);
    lagrange_setup(&ld[d], z[d], n);
  }

  opt_alloc_3(&data, ld);

  //find nearest point
  real x_star[3];
  xyz.get(x_star);

  real r[3] = {0, 0, 0 }; // initial guess for parametric coords
  unsigned c = opt_no_constraints_3;
  dist = opt_findpt_3(&data, (const real **)xm, x_star, r, &c);
  //c tells us if we landed inside the element or exactly on a face, edge, or node

  //copy parametric coords back
  rst = r;

  //Clean-up (move to destructor if we decide to cache)
  opt_free_3(&data);
  for(int d=0; d<3; ++d)
    lagrange_free(&ld[d]);
  for(int d=0; d<3; ++d)
    free(z[d]);
}




// hex_eval
// Evaluate a field in a 3d hex spectral element with n nodes per dimension, at some given parametric coordinates
// field: field values for each of then n*n*n gauss-lobatto nodes. Nodes are in lexicographical order (x is fastest-changing)
// n: number of nodes per dimension -- n=2 for a linear element
// rst: input: parametric coords of the point where we want to evaluate the field
// value: output: value of field at rst

void hex_eval(real *field,
	      int n,
	      CartVect rstCartVec,
	      double &value)
{
  int d;
  real rst[3];
  rstCartVec.get(rst);

  //can cache stuff below
  lagrange_data ld[3];
  real *z[3];
  for(d=0;d<3;++d){
    z[d] = tmalloc(real, n);
    lobatto_nodes(z[d], n);
    lagrange_setup(&ld[d], z[d], n);
  }

  //cut and paste -- see findpt.c
  const unsigned
    nf = n*n,
    ne = n,
    nw = 2*n*n + 3*n;
  real *od_work = tmalloc(real, 6*nf + 9*ne + nw);

  //piece that we shouldn't want to cache
  for(d=0; d<3; d++){
    lagrange_0(&ld[d], rst[d]);
  }

  value = tensor_i3(ld[0].J,ld[0].n,
		    ld[1].J,ld[1].n,
		    ld[2].J,ld[2].n,
		    field,
		    od_work);

  //all this could be cached
  for(d=0; d<3; d++){
    free(z[d]);
    lagrange_free(&ld[d]);
  }
  free(od_work);
}


// Gaussian quadrature points for a trilinear hex element.
// Five 2d arrays are defined.
// One for the single gaussian point solution, 2 point solution,
// 3 point solution, 4 point solution and 5 point solution.
// There are 2 columns, one for Weights and the other for Locations
//                                Weight         Location

const double gauss_1[1][2] = { {  2.0,           0.0          } };

const double gauss_2[2][2] = { {  1.0,          -0.5773502691 },
                               {  1.0         ,  0.5773502691 } };

const double gauss_3[3][2] = { {  0.5555555555, -0.7745966692 },
                               {  0.8888888888,  0.0          },
                               {  0.5555555555,  0.7745966692 } };

const double gauss_4[4][2] = { {  0.3478548451, -0.8611363116 },
                               {  0.6521451549, -0.3399810436 },
                               {  0.6521451549,  0.3399810436 },
                               {  0.3478548451,  0.8611363116 } };

const double gauss_5[5][2] = { {  0.2369268851,  -0.9061798459 },
                               {  0.4786286705,  -0.5384693101 },
                               {  0.5688888889,   0.0          },
                               {  0.4786286705,   0.5384693101 },
                               {  0.2369268851,   0.9061798459 } };

// Function to integrate the field defined by field_fn function
// over the volume of the trilinear hex defined by the hex_corners

bool integrate_trilinear_hex(const CartVect* hex_corners,
                             double *corner_fields,
                             double& field_val,
                             int num_pts)
{
  // Create the LinearHexMap object using the hex_corners array of CartVects
  LinearHexMap hex(hex_corners);

  // Use the correct table of points and locations based on the num_pts parameter
  const double (*g_pts)[2] = 0;
  switch (num_pts) {
  case 1:
    g_pts = gauss_1;
    break;

  case 2:
    g_pts = gauss_2;
    break;

  case 3:
    g_pts = gauss_3;
    break;

  case 4:
    g_pts = gauss_4;
    break;

  case 5:
    g_pts = gauss_5;
    break;

  default:  // value out of range
    return false;
  }

  // Test code - print Gaussian Quadrature data
  if (debug) {
    for (int r=0; r<num_pts; r++)
      for (int c=0; c<2; c++)
        std::cout << "g_pts[" << r << "][" << c << "]=" << g_pts[r][c] << std::endl;
  }
  // End Test code

  double soln = 0.0;

  for (int i=0; i<num_pts; i++) {     // Loop for xi direction
    double w_i  = g_pts[i][0];
    double xi_i = g_pts[i][1];
    for (int j=0; j<num_pts; j++) {   // Loop for eta direction
      double w_j   = g_pts[j][0];
      double eta_j = g_pts[j][1];
      for (int k=0; k<num_pts; k++) { // Loop for zeta direction
        double w_k    = g_pts[k][0];
        double zeta_k = g_pts[k][1];

        // Calculate the "real" space point given the "normal" point
        CartVect normal_pt(xi_i, eta_j, zeta_k);

        // Calculate the value of F(x(xi,eta,zeta),y(xi,eta,zeta),z(xi,eta,zeta)
        double field = hex.evaluate_scalar_field(normal_pt, corner_fields);

        // Calculate the Jacobian for this "normal" point and its determinant
        Matrix3 J = hex.jacobian(normal_pt);
        double det = J.determinant();

        // Calculate integral and update the solution
        soln = soln + (w_i*w_j*w_k*field*det);
      }
    }
  }

  // Set the output parameter
  field_val = soln;

  return true;
}

} // namespace ElemUtil


namespace Element {

    Map::~Map() 
    {}
    
    inline const std::vector<CartVect>& Map::get_vertices() {
        return this->vertex;
      }
        //
      void Map::set_vertices(const std::vector<CartVect>& v) {
        if(v.size() != this->vertex.size()) {
          throw ArgError();
        }
        this->vertex = v;
      }

  bool Map::inside_box(const CartVect & xi, double & tol) const
  {
    // bail out early, before doing an expensive NR iteration
    // compute box
    BoundBox box(this->vertex);
    return box.contains_point(xi.array(), tol);

  }

  //
  CartVect Map::ievaluate(const CartVect& x, double tol, const CartVect& x0) const {
    // TODO: should differentiate between epsilons used for
    // Newton Raphson iteration, and epsilons used for curved boundary geometry errors
    // right now, fix the tolerance used for NR
    tol = 1.0e-10;
    const double error_tol_sqr = tol*tol;
    double det;
    CartVect xi = x0;
    CartVect delta = evaluate(xi) - x;
    Matrix3 J;

    int iters=0;
    while (delta % delta > error_tol_sqr) {
      if(++iters>10)
        throw Map::EvaluationError(x, vertex);

      J = jacobian(xi);
      det = J.determinant();
      if (det < std::numeric_limits<double>::epsilon())
        throw Map::EvaluationError(x, vertex);
      xi -= J.inverse(1.0/det) * delta;
      delta = evaluate( xi ) - x;
    }
    return xi;
  }// Map::ievaluate()

// filescope for static member data that is cached
  const double LinearEdge::corner[2][3] = {  { -1, 0, 0 },
                                         {  1, 0, 0 } };

  LinearEdge::LinearEdge() : Map(0) {

  }// LinearEdge::LinearEdge()

  /* For each point, its weight and location are stored as an array.
     Hence, the inner dimension is 2, the outer dimension is gauss_count.
     We use a one-point Gaussian quadrature, since it integrates linear functions exactly.
  */
  const double LinearEdge::gauss[1][2] = { {  2.0,           0.0          } };

  CartVect LinearEdge::evaluate( const CartVect& xi ) const {
    CartVect x(0.0);
    for (unsigned i = 0; i < LinearEdge::corner_count; ++i) {
      const double N_i = (1.0 + xi[0]*corner[i][0]);
      x += N_i * this->vertex[i];
    }
    x /= LinearEdge::corner_count;
    return x;
  }// LinearEdge::evaluate

  Matrix3 LinearEdge::jacobian( const CartVect& xi ) const {
    Matrix3 J(0.0);
    for (unsigned i = 0; i < LinearEdge::corner_count; ++i) {
      const double   xi_p = 1.0 + xi[0]*corner[i][0];
      const double dNi_dxi   = corner[i][0] * xi_p ;
      J(0,0) += dNi_dxi   * vertex[i][0];
    }
    J(1,1) = 1.0; /* to make sure the Jacobian determinant is non-zero */
    J(2,2) = 1.0; /* to make sure the Jacobian determinant is non-zero */
    J /= LinearEdge::corner_count;
    return J;
  }// LinearEdge::jacobian()

  double LinearEdge::evaluate_scalar_field(const CartVect& xi, const double *field_vertex_value) const {
    double f(0.0);
    for (unsigned i = 0; i < LinearEdge::corner_count; ++i) {
      const double N_i = (1 + xi[0]*corner[i][0])
                          * (1.0 + xi[1]*corner[i][1]);
      f += N_i * field_vertex_value[i];
    }
    f /= LinearEdge::corner_count;
    return f;
  }// LinearEdge::evaluate_scalar_field()

  double LinearEdge::integrate_scalar_field(const double *field_vertex_values) const {
    double I(0.0);
    for(unsigned int j1 = 0; j1 < this->gauss_count; ++j1) {
      double x1 = this->gauss[j1][1];
      double w1 = this->gauss[j1][0];
      CartVect x(x1,0.0,0.0);
      I += this->evaluate_scalar_field(x,field_vertex_values)*w1*this->det_jacobian(x);
    }
    return I;
  }// LinearEdge::integrate_scalar_field()

  bool LinearEdge::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) ;
  }


  const double LinearHex::corner[8][3] = { { -1, -1, -1 },
                                           {  1, -1, -1 },
                                           {  1,  1, -1 },
                                           { -1,  1, -1 },
                                           { -1, -1,  1 },
                                           {  1, -1,  1 },
                                           {  1,  1,  1 },
                                           { -1,  1,  1 } };

  LinearHex::LinearHex() : Map(0) {

  }// LinearHex::LinearHex()

    LinearHex::~LinearHex() 
    {}
  /* For each point, its weight and location are stored as an array.
     Hence, the inner dimension is 2, the outer dimension is gauss_count.
     We use a one-point Gaussian quadrature, since it integrates linear functions exactly.
  */
  //const double LinearHex::gauss[1][2] = { {  2.0,           0.0          } };
  const double LinearHex::gauss[2][2] = { {  1.0,          -0.5773502691 },
                                          {  1.0         ,  0.5773502691 } };
  //const double LinearHex::gauss[4][2] = { {  0.3478548451, -0.8611363116 },
  //                             {  0.6521451549, -0.3399810436 },
  //                             {  0.6521451549,  0.3399810436 },
  //                             {  0.3478548451,  0.8611363116 } };


  CartVect LinearHex::evaluate( const CartVect& xi ) const {
    CartVect x(0.0);
    for (unsigned i = 0; i < 8; ++i) {
      const double N_i =
        (1 + xi[0]*corner[i][0])
      * (1 + xi[1]*corner[i][1])
      * (1 + xi[2]*corner[i][2]);
      x += N_i * this->vertex[i];
    }
    x *= 0.125;
    return x;
  }// LinearHex::evaluate

  Matrix3 LinearHex::jacobian( const CartVect& xi ) const {
    Matrix3 J(0.0);
    for (unsigned i = 0; i < 8; ++i) {
      const double   xi_p = 1 + xi[0]*corner[i][0];
      const double  eta_p = 1 + xi[1]*corner[i][1];
      const double zeta_p = 1 + xi[2]*corner[i][2];
      const double dNi_dxi   = corner[i][0] * eta_p * zeta_p;
      const double dNi_deta  = corner[i][1] *  xi_p * zeta_p;
      const double dNi_dzeta = corner[i][2] *  xi_p *  eta_p;
      J(0,0) += dNi_dxi   * vertex[i][0];
      J(1,0) += dNi_dxi   * vertex[i][1];
      J(2,0) += dNi_dxi   * vertex[i][2];
      J(0,1) += dNi_deta  * vertex[i][0];
      J(1,1) += dNi_deta  * vertex[i][1];
      J(2,1) += dNi_deta  * vertex[i][2];
      J(0,2) += dNi_dzeta * vertex[i][0];
      J(1,2) += dNi_dzeta * vertex[i][1];
      J(2,2) += dNi_dzeta * vertex[i][2];
    }
    return J *= 0.125;
  }// LinearHex::jacobian()

  double LinearHex::evaluate_scalar_field(const CartVect& xi, const double *field_vertex_value) const {
    double f(0.0);
    for (unsigned i = 0; i < 8; ++i) {
      const double N_i = (1 + xi[0]*corner[i][0])
        * (1 + xi[1]*corner[i][1])
        * (1 + xi[2]*corner[i][2]);
      f += N_i * field_vertex_value[i];
    }
    f *= 0.125;
    return f;
  }// LinearHex::evaluate_scalar_field()

  double LinearHex::integrate_scalar_field(const double *field_vertex_values) const {
    double I(0.0);
    for(unsigned int j1 = 0; j1 < this->gauss_count; ++j1) {
      double x1 = this->gauss[j1][1];
      double w1 = this->gauss[j1][0];
      for(unsigned int j2 = 0; j2 < this->gauss_count; ++j2) {
        double x2 = this->gauss[j2][1];
        double w2 = this->gauss[j2][0];
        for(unsigned int j3 = 0; j3 < this->gauss_count; ++j3) {
          double x3 = this->gauss[j3][1];
          double w3 = this->gauss[j3][0];
          CartVect x(x1,x2,x3);
          I += this->evaluate_scalar_field(x,field_vertex_values)*w1*w2*w3*this->det_jacobian(x);
        }
      }
    }
    return I;
  }// LinearHex::integrate_scalar_field()

  bool LinearHex::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) &&
           ( xi[1]>=-1.-tol) && (xi[1]<=1.+tol) &&
           ( xi[2]>=-1.-tol) && (xi[2]<=1.+tol);
  }

  // those are not just the corners, but for simplicity, keep this name
  //
  const int QuadraticHex::corner[27][3] = {
      { -1, -1, -1 },
      {  1, -1, -1 },
      {  1,  1, -1 },  // corner nodes: 0-7
      { -1,  1, -1 },  // mid-edge nodes: 8-19
      { -1, -1,  1 },  // center-face nodes 20-25  center node  26
      {  1, -1,  1 },  //
      {  1,  1,  1 },
      { -1,  1,  1 }, //                    4   ----- 19   -----  7
      {  0, -1, -1 }, //                .   |                 .   |
      {  1,  0, -1 }, //            16         25         18      |
      {  0,  1, -1 }, //         .          |          .          |
      { -1,  0, -1 }, //      5   ----- 17   -----  6             |
      { -1, -1,  0 }, //      |            12       | 23         15
      {  1, -1,  0 }, //      |                     |             |
      {  1,  1,  0 }, //      |     20      |  26   |     22      |
      { -1,  1,  0 }, //      |                     |             |
      {  0, -1,  1 }, //     13         21  |      14             |
      {  1,  0,  1 }, //      |             0   ----- 11   -----  3
      {  0,  1,  1 }, //      |         .           |         .
      { -1,  0,  1 }, //      |      8         24   |     10
      {  0, -1,  0 }, //      |  .                  |  .
      {  1,  0,  0 }, //      1   -----  9   -----  2
      {  0,  1,  0 }, //
      { -1,  0,  0 },
      {  0,  0, -1 },
      {  0,  0,  1 },
      {  0,  0,  0 }
  };
  //QuadraticHex::QuadraticHex(const std::vector<CartVect>& vertices) : Map(vertices){};
  QuadraticHex::QuadraticHex():Map(0) {
  }

    QuadraticHex::~QuadraticHex() 
    {}
  double SH(const int i, const double xi)
  {
    switch (i)
    {
    case -1: return (xi*xi-xi)/2;
    case 0: return 1-xi*xi;
    case 1: return (xi*xi+xi)/2;
    default: return 0.;
    }
  }
  double DSH(const int i, const double xi)
  {
    switch (i)
    {
    case -1: return xi-0.5;
    case 0: return -2*xi;
    case 1: return xi+0.5;
    default: return 0.;
    }
  }

  CartVect QuadraticHex::evaluate( const CartVect& xi ) const
  {

    CartVect x(0.0);
    for (int i=0; i<27; i++)
    {
      const double sh= SH(corner[i][0], xi[0])
                      *SH(corner[i][1], xi[1])
                      *SH(corner[i][2], xi[2]);
      x+=sh* vertex[i];
    }

    return x;
  }
  //virtual CartVect ievaluate(const CartVect& x, double tol) const ;
  bool QuadraticHex::inside_nat_space(const CartVect & xi, double & tol) const
  {// just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) &&
           ( xi[1]>=-1.-tol) && (xi[1]<=1.+tol) &&
           ( xi[2]>=-1.-tol) && (xi[2]<=1.+tol);
  }

  Matrix3  QuadraticHex::jacobian(const CartVect& xi) const
  {
    Matrix3 J(0.0);
    for (int i=0; i<27; i++)
    {
      const double sh[3]={ SH(corner[i][0], xi[0]),
                           SH(corner[i][1], xi[1]),
                           SH(corner[i][2], xi[2]) };
      const double dsh[3]={ DSH(corner[i][0], xi[0]),
                            DSH(corner[i][1], xi[1]),
                            DSH(corner[i][2], xi[2]) };


      for (int j=0; j<3; j++)
      {
        J(j,0)+=dsh[0]*sh[1]*sh[2]*vertex[i][j]; // dxj/dr first column
        J(j,1)+=sh[0]*dsh[1]*sh[2]*vertex[i][j]; // dxj/ds
        J(j,2)+=sh[0]*sh[1]*dsh[2]*vertex[i][j]; // dxj/dt
      }
    }


    return J;
  }
  double   QuadraticHex::evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const
  {
    double x=0.0;
    for (int i=0; i<27; i++)
    {
      const double sh= SH(corner[i][0], xi[0])
                *SH(corner[i][1], xi[1])
                *SH(corner[i][2], xi[2]);
      x+=sh* field_vertex_values[i];
    }

    return x;
  }
  double   QuadraticHex::integrate_scalar_field(const double* /*field_vertex_values*/) const
  {
    return 0.;// TODO: gaussian integration , probably 2x2x2
  }


  const double LinearTet::corner[4][3] = { {0,0,0},
                                           {1,0,0},
                                           {0,1,0},
                                           {0,0,1}};

  LinearTet::LinearTet() : Map(0) {

  }// LinearTet::LinearTet()


    LinearTet::~LinearTet() 
    {}

  void LinearTet::set_vertices(const std::vector<CartVect>& v) {
    this->Map::set_vertices(v);
    this->T = Matrix3(v[1][0]-v[0][0],v[2][0]-v[0][0],v[3][0]-v[0][0],
                      v[1][1]-v[0][1],v[2][1]-v[0][1],v[3][1]-v[0][1],
                      v[1][2]-v[0][2],v[2][2]-v[0][2],v[3][2]-v[0][2]);
    this->T_inverse = this->T.inverse();
    this->det_T = this->T.determinant();
    this->det_T_inverse = (0.0 == this->det_T ? HUGE : 1.0/this->det_T);
  }// LinearTet::set_vertices()


  double LinearTet::evaluate_scalar_field(const CartVect& xi, const double *field_vertex_value) const {
    double f0 = field_vertex_value[0];
    double f = f0;
    for (unsigned i = 1; i < 4; ++i) {
      f += (field_vertex_value[i]-f0)*xi[i-1];
    }
    return f;
  }// LinearTet::evaluate_scalar_field()

  double LinearTet::integrate_scalar_field(const double *field_vertex_values) const {
    double I(0.0);
    for(unsigned int i = 0; i < 4; ++i) {
      I += field_vertex_values[i];
    }
    I *= this->det_T/24.0;
    return I;
  }// LinearTet::integrate_scalar_field()
  bool LinearTet::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // linear tet space is a tetra with vertices (0,0,0), (1,0,0), (0,1,0), (0, 0, 1)
    // first check if outside bigger box, then below the plane x+y+z=1
    return ( xi[0]>=-tol)  &&
        ( xi[1]>=-tol)  &&
        ( xi[2]>=-tol)  &&
        ( xi[0]+xi[1]+xi[2] < 1.0+tol);
  }
  // SpectralHex

  // filescope for static member data that is cached
  int SpectralHex::_n;
  real *SpectralHex::_z[3];
  lagrange_data SpectralHex::_ld[3];
  opt_data_3 SpectralHex::_data;
  real * SpectralHex::_odwork;

  bool SpectralHex::_init = false;

  SpectralHex::SpectralHex() : Map(0)
  {
  }
  // the preferred constructor takes pointers to GL blocked positions
  SpectralHex::SpectralHex(int order, double * x, double *y, double *z) : Map(0)
  {
    Init(order);
    _xyz[0]=x; _xyz[1]=y; _xyz[2]=z;
  }
  SpectralHex::SpectralHex(int order) : Map(0)
  {
    Init(order);
    _xyz[0]=_xyz[1]=_xyz[2]=NULL;
  }
  SpectralHex::~SpectralHex()
  {
    if (_init)
      freedata();
    _init=false;
  }
  void SpectralHex::Init(int order)
  {
    if (_init && _n==order)
      return;
    if (_init && _n!=order)
    {
      // TODO: free data cached
      freedata();
    }
    // compute stuff that depends only on order
    _init = true;
    _n = order;
    //triplicates! n is the same in all directions !!!
    for(int d=0; d<3; d++){
      _z[d] = tmalloc(real, _n);
      lobatto_nodes(_z[d], _n);
      lagrange_setup(&_ld[d], _z[d], _n);
    }
    opt_alloc_3(&_data, _ld);

    unsigned int nf = _n*_n, ne = _n, nw = 2*_n*_n + 3*_n;
    _odwork = tmalloc(real, 6*nf + 9*ne + nw);
  }
  void SpectralHex::freedata()
  {
    for(int d=0; d<3; d++){
      free(_z[d]);
      lagrange_free(&_ld[d]);
    }
    opt_free_3(&_data);
    free(_odwork);
  }

  void SpectralHex::set_gl_points( double * x, double * y, double *z)
  {
    _xyz[0] = x;
    _xyz[1] = y;
    _xyz[2] = z;
  }
  CartVect SpectralHex::evaluate( const CartVect& xi ) const
  {
    //piece that we shouldn't want to cache
    int d=0;
    for(d=0; d<3; d++){
      lagrange_0(&_ld[d], xi[d]);
    }
    CartVect result;
    for (d=0; d<3; d++)
    {
      result[d] = tensor_i3(_ld[0].J,_ld[0].n,
            _ld[1].J,_ld[1].n,
            _ld[2].J,_ld[2].n,
            _xyz[d],   // this is the "field"
            _odwork);
    }
    return result;
  }
  // replicate the functionality of hex_findpt
  CartVect SpectralHex::ievaluate(CartVect const & xyz) const
  {
    //find nearest point
    real x_star[3];
    xyz.get(x_star);

    real r[3] = {0, 0, 0 }; // initial guess for parametric coords
    unsigned c = opt_no_constraints_3;
    real dist = opt_findpt_3(&_data, (const real **)_xyz, x_star, r, &c);
    // if it did not converge, get out with throw...
    if (dist > 0.9e+30)
    {
      std::vector<CartVect> dummy;
      throw Map::EvaluationError(xyz, dummy);
    }
    //c tells us if we landed inside the element or exactly on a face, edge, or node
    // also, dist shows the distance to the computed point.
    //copy parametric coords back
    return CartVect(r);
  }
  Matrix3  SpectralHex::jacobian(const CartVect& xi) const
  {
    real x_i[3];
    xi.get(x_i);
    // set the positions of GL nodes, before evaluations
    _data.elx[0]=_xyz[0];
    _data.elx[1]=_xyz[1];
    _data.elx[2]=_xyz[2];
    opt_vol_set_intp_3(&_data,x_i);
    Matrix3 J(0.);
    // it is organized differently
    J(0,0) = _data.jac[0]; // dx/dr
    J(0,1) = _data.jac[1]; // dx/ds
    J(0,2) = _data.jac[2]; // dx/dt
    J(1,0) = _data.jac[3]; // dy/dr
    J(1,1) = _data.jac[4]; // dy/ds
    J(1,2) = _data.jac[5]; // dy/dt
    J(2,0) = _data.jac[6]; // dz/dr
    J(2,1) = _data.jac[7]; // dz/ds
    J(2,2) = _data.jac[8]; // dz/dt
    return J;
  }
  double   SpectralHex::evaluate_scalar_field(const CartVect& xi, const double *field) const
  {
    //piece that we shouldn't want to cache
    int d;
    for(d=0; d<3; d++){
      lagrange_0(&_ld[d], xi[d]);
    }

    double value = tensor_i3(_ld[0].J,_ld[0].n,
          _ld[1].J,_ld[1].n,
          _ld[2].J,_ld[2].n,
          field,
          _odwork);
    return value;
  }
  double   SpectralHex::integrate_scalar_field(const double *field_vertex_values) const
  {
    // set the position of GL points
    // set the positions of GL nodes, before evaluations
    _data.elx[0]=_xyz[0];
    _data.elx[1]=_xyz[1];
    _data.elx[2]=_xyz[2];
    double xi[3];
    //triple loop; the most inner loop is in r direction, then s, then t
    double integral = 0.;
    //double volume = 0;
    int index=0; // used fr the inner loop
    for (int k=0; k<_n; k++ )
    {
      xi[2]=_ld[2].z[k];
      //double wk= _ld[2].w[k];
      for (int j=0; j<_n; j++)
      {
        xi[1]=_ld[1].z[j];
        //double wj= _ld[1].w[j];
        for (int i=0; i<_n; i++)
        {
          xi[0]=_ld[0].z[i];
          //double wi= _ld[0].w[i];
          opt_vol_set_intp_3(&_data,xi);
          double wk= _ld[2].J[k];
          double wj= _ld[1].J[j];
          double wi= _ld[0].J[i];
          Matrix3 J(0.);
          // it is organized differently
          J(0,0) = _data.jac[0]; // dx/dr
          J(0,1) = _data.jac[1]; // dx/ds
          J(0,2) = _data.jac[2]; // dx/dt
          J(1,0) = _data.jac[3]; // dy/dr
          J(1,1) = _data.jac[4]; // dy/ds
          J(1,2) = _data.jac[5]; // dy/dt
          J(2,0) = _data.jac[6]; // dz/dr
          J(2,1) = _data.jac[7]; // dz/ds
          J(2,2) = _data.jac[8]; // dz/dt
          double bm = wk*wj*wi* J.determinant();
          integral+= bm*field_vertex_values[index++];
          //volume +=bm;
        }
      }
    }
    //std::cout << "volume: " << volume << "\n";
    return integral;
  }
  // this is the same as a linear hex, although we should not need it
  bool SpectralHex::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) &&
           ( xi[1]>=-1.-tol) && (xi[1]<=1.+tol) &&
           ( xi[2]>=-1.-tol) && (xi[2]<=1.+tol);
  }

  // SpectralHex

  // filescope for static member data that is cached
  const double LinearQuad::corner[4][3] = {  { -1, -1, 0 },
                                             {  1, -1, 0 },
                                             {  1,  1, 0 },
                                             { -1,  1, 0 } };

  LinearQuad::LinearQuad() : Map(0) {

  }// LinearQuad::LinearQuad()

    LinearQuad::~LinearQuad() 
    {}
    
  /* For each point, its weight and location are stored as an array.
     Hence, the inner dimension is 2, the outer dimension is gauss_count.
     We use a one-point Gaussian quadrature, since it integrates linear functions exactly.
  */
  const double LinearQuad::gauss[1][2] = { {  2.0,           0.0          } };

  CartVect LinearQuad::evaluate( const CartVect& xi ) const {
    CartVect x(0.0);
    for (unsigned i = 0; i < LinearQuad::corner_count; ++i) {
      const double N_i =
        (1 + xi[0]*corner[i][0])
      * (1 + xi[1]*corner[i][1]);
      x += N_i * this->vertex[i];
    }
    x /= LinearQuad::corner_count;
    return x;
  }// LinearQuad::evaluate

  Matrix3 LinearQuad::jacobian( const CartVect& xi ) const {
    Matrix3 J(0.0);
    for (unsigned i = 0; i < LinearQuad::corner_count; ++i) {
      const double   xi_p = 1 + xi[0]*corner[i][0];
      const double  eta_p = 1 + xi[1]*corner[i][1];
      const double dNi_dxi   = corner[i][0] * eta_p ;
      const double dNi_deta  = corner[i][1] *  xi_p ;
      J(0,0) += dNi_dxi   * vertex[i][0];
      J(1,0) += dNi_dxi   * vertex[i][1];
      J(0,1) += dNi_deta  * vertex[i][0];
      J(1,1) += dNi_deta  * vertex[i][1];
    }
    J(2,2) = 1.0; /* to make sure the Jacobian determinant is non-zero */
    J /= LinearQuad::corner_count;
    return J;
  }// LinearQuad::jacobian()

  double LinearQuad::evaluate_scalar_field(const CartVect& xi, const double *field_vertex_value) const {
    double f(0.0);
    for (unsigned i = 0; i < LinearQuad::corner_count; ++i) {
      const double N_i = (1 + xi[0]*corner[i][0])
        * (1 + xi[1]*corner[i][1]);
      f += N_i * field_vertex_value[i];
    }
    f /= LinearQuad::corner_count;
    return f;
  }// LinearQuad::evaluate_scalar_field()

  double LinearQuad::integrate_scalar_field(const double *field_vertex_values) const {
    double I(0.0);
    for(unsigned int j1 = 0; j1 < this->gauss_count; ++j1) {
      double x1 = this->gauss[j1][1];
      double w1 = this->gauss[j1][0];
      for(unsigned int j2 = 0; j2 < this->gauss_count; ++j2) {
        double x2 = this->gauss[j2][1];
        double w2 = this->gauss[j2][0];
        CartVect x(x1,x2,0.0);
        I += this->evaluate_scalar_field(x,field_vertex_values)*w1*w2*this->det_jacobian(x);
      }
    }
    return I;
  }// LinearQuad::integrate_scalar_field()

  bool LinearQuad::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) &&
           ( xi[1]>=-1.-tol) && (xi[1]<=1.+tol) ;
  }


  // filescope for static member data that is cached
  int SpectralQuad::_n;
  real *SpectralQuad::_z[2];
  lagrange_data SpectralQuad::_ld[2];
  opt_data_2 SpectralQuad::_data;
  real * SpectralQuad::_odwork;
  real * SpectralQuad::_glpoints;
  bool SpectralQuad::_init = false;

  SpectralQuad::SpectralQuad() : Map(0)
  {
  }
  // the preferred constructor takes pointers to GL blocked positions
  SpectralQuad::SpectralQuad(int order, double * x, double *y, double *z) : Map(0)
  {
    Init(order);
    _xyz[0]=x; _xyz[1]=y; _xyz[2]=z;
  }
  SpectralQuad::SpectralQuad(int order) : Map(4)
  {
    Init(order);
    _xyz[0]=_xyz[1]=_xyz[2]=NULL;
  }
  SpectralQuad::~SpectralQuad()
  {
    if (_init)
      freedata();
    _init=false;
  }
  void SpectralQuad::Init(int order)
  {
    if (_init && _n==order)
      return;
    if (_init && _n!=order)
    {
      // TODO: free data cached
      freedata();
    }
    // compute stuff that depends only on order
    _init = true;
    _n = order;
    //duplicates! n is the same in all directions !!!
    for(int d=0; d<2; d++){
      _z[d] = tmalloc(real, _n);
      lobatto_nodes(_z[d], _n);
      lagrange_setup(&_ld[d], _z[d], _n);
    }
    opt_alloc_2(&_data, _ld);

    unsigned int nf = _n*_n, ne = _n, nw = 2*_n*_n + 3*_n;
    _odwork = tmalloc(real, 6*nf + 9*ne + nw);
    _glpoints = tmalloc (real, 3*nf);
  }

  void SpectralQuad::freedata()
  {
    for(int d=0; d<2; d++){
      free(_z[d]);
      lagrange_free(&_ld[d]);
    }
    opt_free_2(&_data);
    free(_odwork);
    free(_glpoints);
  }

  void SpectralQuad::set_gl_points( double * x, double * y, double *z)
  {
    _xyz[0] = x;
    _xyz[1] = y;
    _xyz[2] = z;
  }
  CartVect SpectralQuad::evaluate( const CartVect& xi ) const
  {
    //piece that we shouldn't want to cache
    int d=0;
    for(d=0; d<2; d++){
      lagrange_0(&_ld[d], xi[d]);
    }
    CartVect result;
    for (d=0; d<3; d++)
    {
      result[d] = tensor_i2(_ld[0].J,_ld[0].n,
          _ld[1].J,_ld[1].n,
          _xyz[d],
          _odwork);
    }
    return result;
  }
  // replicate the functionality of hex_findpt
  CartVect SpectralQuad::ievaluate(CartVect const & xyz) const
  {
    //find nearest point
    real x_star[3];
    xyz.get(x_star);

    real r[2] = {0, 0 }; // initial guess for parametric coords
    unsigned c = opt_no_constraints_3;
    real dist = opt_findpt_2(&_data, (const real **)_xyz, x_star, r, &c);
    // if it did not converge, get out with throw...
    if (dist > 0.9e+30)
    {
      std::vector<CartVect> dummy;
      throw Map::EvaluationError(xyz, dummy);
    }

    //c tells us if we landed inside the element or exactly on a face, edge, or node
    // also, dist shows the distance to the computed point.
    //copy parametric coords back
    return CartVect(r[0], r[1], 0.);
  }


  Matrix3  SpectralQuad::jacobian(const CartVect& /*xi*/) const
  {
    // not implemented
    Matrix3 J(0.);
    return J;
  }


  double   SpectralQuad::evaluate_scalar_field(const CartVect& xi, const double *field) const
  {
    //piece that we shouldn't want to cache
    int d;
    for(d=0; d<2; d++){
      lagrange_0(&_ld[d], xi[d]);
    }

    double value = tensor_i2(_ld[0].J,_ld[0].n,
          _ld[1].J,_ld[1].n,
          field,
          _odwork);
    return value;
  }
  double  SpectralQuad:: integrate_scalar_field(const double */*field_vertex_values*/) const
  {
    return 0.;// not implemented
  }
  // this is the same as a linear hex, although we should not need it
  bool SpectralQuad::inside_nat_space(const CartVect & xi, double & tol) const
  {
    // just look at the box+tol here
    return ( xi[0]>=-1.-tol) && (xi[0]<=1.+tol) &&
           ( xi[1]>=-1.-tol) && (xi[1]<=1.+tol) ;
  }
  // something we don't do for spectral hex; we do it here because
  //       we do not store the position of gl points in a tag yet
  void SpectralQuad::compute_gl_positions()
  {
    // will need to use shape functions on a simple linear quad to compute gl points
    // so we know the position of gl points in parametric space, we will just compute those
    // from the 3d vertex position (corner nodes of the quad), using simple mapping
    assert (this->vertex.size()==4);
    static double corner_xi[4][2]={ { -1., -1.},
                              {  1., -1.},
                              {  1.,  1.},
                              { -1.,  1.} };
    // we will use the cached lobatto nodes in parametric space _z[d] (the same in both directions)
    int indexGL=0;
    int n2= _n*_n;
    for (int i=0; i<_n; i++)
    {
      double eta=_z[0][i];
      for (int j=0; j<_n; j++)
      {
        double csi = _z[1][j]; // we could really use the same _z[0] array of lobatto nodes
        CartVect pos(0.0);
        for (int k = 0; k < 4; k++) {
          const double N_k = (1 + csi*corner_xi[k][0])
                           * (1 + eta*corner_xi[k][1]);
          pos += N_k * vertex[k];
        }
        pos *= 0.25;// these are x, y, z of gl points; reorder them
        _glpoints[indexGL] = pos[0]; // x
        _glpoints[indexGL+n2] = pos[1]; // y
        _glpoints[indexGL+2*n2] = pos[2]; // z
        indexGL++;
      }
    }
    // now, we can set the _xyz pointers to internal memory allocated to these!
    _xyz[0] =  &(_glpoints[0]);
    _xyz[1] =  &(_glpoints[n2]);
    _xyz[2] =  &(_glpoints[2*n2]);
  }
  void SpectralQuad::get_gl_points( double *& x, double *& y, double *& z, int & psize)
  {
    x=  (double *)_xyz[0] ;
    y = (double *)_xyz[1] ;
    z = (double *)_xyz[2] ;
    psize = _n*_n;
  }
}// namespace Element

} // namespace moab
