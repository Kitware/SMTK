/* Copyright 2007-2009 Brad King, Chuck Stewart
   Distributed under the Boost Software License, Version 1.0.
   (See accompanying file rtvl_license_1_0.txt or copy at
   http://www.boost.org/LICENSE_1_0.txt) */
#ifndef rtvl_weight_original_hxx
#define rtvl_weight_original_hxx

#include "rtvl_weight.hxx"

template <unsigned int N>
class rtvl_terms;

template <unsigned int N>
class rtvl_weight_original : public rtvl_weight<N>
{
public:
  typedef rtvl_weight<N> derived;
  rtvl_weight_original(double gs = 1, double cc = 1);
  void set_scale(double gs) override;

protected:
  double compute_flat(rtvl_terms<N> const& terms) override;
  void compute_flat_d(rtvl_terms<N> const& terms, vnl_vector_fixed<double, N>& dwflat) override;
  double compute_curved(rtvl_terms<N> const& terms) override;
  void compute_curved_d(rtvl_terms<N> const& terms, vnl_vector_fixed<double, N>& dwcurve) override;
  void set_scale_impl(double gs);

private:
  double cconst;
  double geodesic_scale2;
  double c;
  double theta;
  double arclen;
  double kappa;
};

#endif
