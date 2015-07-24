#ifndef MOAB_ELEM_UTIL_HPP
#define MOAB_ELEM_UTIL_HPP

#include "moab/CartVect.hpp"
#include <vector>
#include "moab/Matrix3.hpp"

// to access data structures for spectral elements

extern "C" 
{
#include "moab/FindPtFuncs.h"
}

namespace moab {
namespace ElemUtil {

  bool nat_coords_trilinear_hex(const CartVect* hex_corners,
                                const CartVect& x,
                                CartVect& xi,
                                double tol);
  bool point_in_trilinear_hex(const CartVect *hex_corners,
                              const CartVect& xyz,
                              double etol);

  bool point_in_trilinear_hex(const CartVect *hex_corners,
                              const CartVect& xyz,
                              const CartVect& box_min,
                              const CartVect& box_max,
                              double etol);

    //wrapper to hex_findpt
  void nat_coords_trilinear_hex2(const CartVect* hex_corners,
                                 const CartVect& x,
                                 CartVect& xi,
                                 double til);



  void hex_findpt(double *xm[3],
                  int n,
                  CartVect xyz,
                  CartVect& rst,
                  double& dist);

  void hex_eval(double *field,
		int n,
		CartVect rst,
		double &value);

  bool integrate_trilinear_hex(const CartVect* hex_corners,
                               double *corner_fields,
                               double& field_val,
                               int num_pts);

} // namespace ElemUtil

  namespace Element {
    /**\brief Class representing a map (diffeomorphism) F parameterizing a 3D element by its canonical preimage.*/
    /*
         Shape functions on the element can obtained by a pushforward (pullback by the inverse map)
         of the shape functions on the canonical element. This is done by extending this class.

         We further assume that the parameterizing map is defined by the location of n vertices,
         which can be set and retrived on a Map instance.  The number of vertices is fixed at
         compile time.
    */
    class Map {
    public:
      /**\brief Construct a Map defined by the given std::vector of vertices. */
      Map(const std::vector<CartVect>& v) {this->vertex.resize(v.size()); this->set_vertices(v);};
      /**\brief Construct a Map defined by n vertices. */
      Map(const unsigned int n) {this->vertex = std::vector<CartVect>(n);};
      virtual ~Map();
      /**\brief Evaluate the map on \xi (calculate $\vec x = F($\vec \xi)$ )*/
      virtual CartVect evaluate( const CartVect& xi ) const = 0;
      /**\brief Evaluate the inverse map (calculate $\vec \xi = F^-1($\vec x)$ to given tolerance)*/
      virtual CartVect ievaluate( const CartVect& x, double tol, const CartVect& x0 = CartVect(0.0)) const ;
      /**\brief decide if within the natural param space, with a tolerance*/
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const = 0;
      /* FIX: should evaluate and ievaluate return both the value and the Jacobian (first jet)? */
      /**\brief Evaluate the map's Jacobi matrix. */
      virtual Matrix3 jacobian( const CartVect& xi ) const = 0;
      /* FIX: should this be evaluated in real coordinates and be obtained as part of a Newton solve? */
      /**\brief Evaluate the inverse of the Jacobi matrix. */
      virtual Matrix3 ijacobian( const CartVect& xi ) const {return this->jacobian(xi).inverse();};
      /* det_jacobian and det_ijacobian should be overriden for efficiency. */
      /**\brief Evaluate the determinate of the Jacobi matrix. */
      virtual double  det_jacobian(const CartVect& xi) const {return this->jacobian(xi).determinant();};
      /* FIX: should this be evaluated in real coordinates and be obtained as part of a Newton solve? */
      /**\brief Evaluate the determinate of the inverse Jacobi matrix. */
      virtual double  det_ijacobian(const CartVect& xi) const {return this->jacobian(xi).inverse().determinant();};

      /**\brief Evaluate a scalar field at a point given field values at the vertices. */
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const = 0;
      /**\brief Integrate a scalar field over the element given field values at the vertices. */
      virtual double   integrate_scalar_field(const double *field_vertex_values) const = 0;

      /**\brief Size of the vertices vector. */
      unsigned int size() {return this->vertex.size();}
      /**\brief Retrieve vertices. */
      const std::vector<CartVect>& get_vertices();
      /**\brief Set vertices.      */
      virtual void set_vertices(const std::vector<CartVect>& v);

      // will look at the box formed by vertex coordinates, and before doing any NR, bail out if necessary
      virtual bool inside_box(const CartVect & xi, double & tol) const;

      /* Exception thrown when an evaluation fails (e.g., ievaluate fails to converge). */
      class EvaluationError {
      public:
        EvaluationError(const CartVect & x, const std::vector<CartVect> & verts): p(x), vertices(verts){
#ifndef NDEBUG
          std::cout << "p:" << p << "\n vertices.size() " <<vertices.size() << "\n";
          for (size_t i=0; i<vertices.size(); i++)
            std::cout << vertices[i] << "\n";
#endif
        };
      private:
        CartVect p;
        std::vector<CartVect> vertices;
      };// class EvaluationError

      /* Exception thrown when a bad argument is encountered. */
      class ArgError {
      public:
        ArgError(){};
      };// class ArgError
    protected:
      std::vector<CartVect> vertex;
    };// class Map

    /**\brief Shape function space for trilinear hexahedron, obtained by a pushforward of the canonical linear (affine) functions. */
    class LinearHex : public Map {
    public:
      LinearHex(const std::vector<CartVect>& vertices) : Map(vertices){};
      LinearHex();
      virtual ~LinearHex();
      
      virtual CartVect evaluate( const CartVect& xi ) const;
      //virtual CartVect ievaluate(const CartVect& x, double tol) const ;
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const;

      virtual Matrix3  jacobian(const CartVect& xi) const;
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      virtual double   integrate_scalar_field(const double *field_vertex_values) const;

    protected:
      /* Preimages of the vertices -- "canonical vertices" -- are known as "corners". */
      static const double corner[8][3];
      static const double gauss[2][2];
      static const unsigned int corner_count = 8;
      static const unsigned int gauss_count  = 2;

    };// class LinearHex

    /**\brief Shape function space for trilinear hexahedron, obtained by a pushforward of the canonical linear (affine) functions. */
    class QuadraticHex : public Map {
    public:
      QuadraticHex(const std::vector<CartVect>& vertices) : Map(vertices){};
      QuadraticHex();
      virtual ~QuadraticHex();
      virtual CartVect evaluate( const CartVect& xi ) const;
      //virtual CartVect ievaluate(const CartVect& x, double tol) const ;
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const;

      virtual Matrix3  jacobian(const CartVect& xi) const;
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      virtual double   integrate_scalar_field(const double *field_vertex_values) const;

    protected:
      /* Preimages of the vertices -- "canonical vertices" -- are known as "corners".
       * there are 27 vertices for a tri-quadratic xes*/
      static const int corner[27][3];
      static const double gauss[8][2];// TODO fix me
      static const unsigned int corner_count = 27;
      static const unsigned int gauss_count  = 2; // TODO fix me

    };// class QuadraticHex
    /**\brief Shape function space for a linear tetrahedron, obtained by a pushforward of the canonical affine shape functions. */
    class LinearTet : public Map {
    public:
      LinearTet(const std::vector<CartVect>& vertices) : Map(vertices){ LinearTet::set_vertices(vertex);};
      LinearTet();
      virtual ~LinearTet();
      /* Override the evaluation routines to take advantage of the properties of P1. */
      virtual CartVect evaluate(const CartVect& xi) const {return this->vertex[0] + this->T*xi;};
      using Map::ievaluate;
      virtual CartVect ievaluate(const CartVect& x) const {return this->T_inverse*(x-this->vertex[0]);};
      virtual Matrix3  jacobian(const CartVect& )  const {return this->T;};
      virtual Matrix3  ijacobian(const CartVect& ) const {return this->T_inverse;};
      virtual double   det_jacobian(const CartVect& )  const {return this->det_T;};
      virtual double   det_ijacobian(const CartVect& ) const {return this->det_T_inverse;};
      //
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      virtual double   integrate_scalar_field(const double *field_vertex_values) const;
      //
      /* Override set_vertices so we can precompute the matrices effecting the mapping to and from the canonical simplex. */
      virtual void     set_vertices(const std::vector<CartVect>& v);
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const;
    protected:
      static const double corner[4][3];
      Matrix3 T, T_inverse;
      double  det_T, det_T_inverse;
    };// class LinearTet

    class SpectralHex : public Map {
    public:
      SpectralHex(const std::vector<CartVect>& vertices) : Map(vertices){};
      SpectralHex(int order, double * x, double * y, double *z) ;
      SpectralHex(int order);
      SpectralHex();
      virtual ~SpectralHex();
      void set_gl_points( double * x, double * y, double *z) ;
      virtual CartVect evaluate( const CartVect& xi ) const;
      using Map::ievaluate;
      virtual CartVect ievaluate(const CartVect& x) const;
      virtual Matrix3  jacobian(const CartVect& xi) const;
      double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      double   integrate_scalar_field(const double *field_vertex_values) const;
      bool inside_nat_space(const CartVect & xi, double & tol) const;

      // to compute the values that need to be cached for each element of order n
      void Init(int order);
      void freedata();
    protected:
      /* values that depend only on the order of the element , cached */
      /*  the order in 3 directions */
      static int _n;
      static real *_z[3];
      static lagrange_data _ld[3];
      static opt_data_3 _data;
      static real * _odwork;// work area

      // flag for initialization of data
      static bool _init;

      real * _xyz[3];

    };// class SpectralHex

    /**\brief Shape function space for bilinear quadrilateral, obtained from the canonical linear (affine) functions. */
    class LinearQuad : public Map {
    public:
      LinearQuad(const std::vector<CartVect>& vertices) : Map(vertices){};
      LinearQuad();
      virtual ~LinearQuad();
      virtual CartVect evaluate( const CartVect& xi ) const;
      //virtual CartVect ievaluate(const CartVect& x, double tol) const ;
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const;

      virtual Matrix3  jacobian(const CartVect& xi) const;
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      virtual double   integrate_scalar_field(const double *field_vertex_values) const;

    protected:
      /* Preimages of the vertices -- "canonical vertices" -- are known as "corners". */
      static const double corner[4][3];
      static const double gauss[1][2];
      static const unsigned int corner_count = 4;
      static const unsigned int gauss_count  = 1;

    };// class LinearQuad

    /**\brief Shape function space for bilinear quadrilateral, obtained from the canonical linear (affine) functions. */
    class LinearEdge : public Map {
    public:
      LinearEdge(const std::vector<CartVect>& vertices) : Map(vertices){};
      LinearEdge();
      virtual CartVect evaluate( const CartVect& xi ) const;
      //virtual CartVect ievaluate(const CartVect& x, double tol) const ;
      virtual bool inside_nat_space(const CartVect & xi, double & tol) const;

      virtual Matrix3  jacobian(const CartVect& xi) const;
      virtual double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
      virtual double   integrate_scalar_field(const double *field_vertex_values) const;

    protected:
      /* Preimages of the vertices -- "canonical vertices" -- are known as "corners". */
      static const double corner[2][3];
      static const double gauss[1][2];
      static const unsigned int corner_count = 2;
      static const unsigned int gauss_count  = 1;

    };// class LinearEdge

    class SpectralQuad : public Map {
      public:
        SpectralQuad(const std::vector<CartVect>& vertices) : Map(vertices){};
        SpectralQuad(int order, double * x, double * y, double *z) ;
        SpectralQuad(int order);
        SpectralQuad();
        virtual ~SpectralQuad();
        void set_gl_points( double * x, double * y, double *z) ;
        virtual CartVect evaluate( const CartVect& xi ) const;// a 2d, so 3rd component is 0, always
        using Map::ievaluate;
        virtual CartVect ievaluate(const CartVect& x) const; //a 2d, so 3rd component is 0, always
        virtual Matrix3  jacobian(const CartVect& xi) const;
        double   evaluate_scalar_field(const CartVect& xi, const double *field_vertex_values) const;
        double   integrate_scalar_field(const double *field_vertex_values) const;
        bool inside_nat_space(const CartVect & xi, double & tol) const;

        // to compute the values that need to be cached for each element of order n
        void Init(int order);
        void freedata();
        // this will take node, vertex positions and compute the gl points
        void compute_gl_positions();
        void get_gl_points(  double *& x, double *& y, double *& z, int & size) ;
      protected:
        /* values that depend only on the order of the element , cached */
        /*  the order in all 3 directions ; it is also np in HOMME lingo*/
        static int _n;
        static real *_z[2];
        static lagrange_data _ld[2];
        static opt_data_2 _data; // we should use only 2nd component
        static real * _odwork;// work area

        // flag for initialization of data
        static bool _init;
        static real * _glpoints; // it is a space we can use to store gl positions for elements
        // on the fly; we do not have a tag yet for them, as in Nek5000 application
        // also, these positions might need to be moved on the sphere, for HOMME grids
        // do we project them or how do we move them on the sphere?

        real * _xyz[3]; // these are gl points; position?


      };// class SpectralQuad


  }// namespace Element

} // namespace moab

#endif /*MOAB_ELEM_UTIL_HPP*/
