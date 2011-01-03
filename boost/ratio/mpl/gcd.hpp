//  gcd.hpp
//
//  (C) Copyright 2011 Vicente J. Botet Escriba
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//

#ifndef BOOST_RATIO_MPL_GCD_HPP
#define BOOST_RATIO_MPL_GCD_HPP

#include <boost/ratio.hpp>
#include <boost/ratio/mpl/numeric_cast.hpp>
#include <boost/ratio/detail/mpl/gcd.hpp>

namespace boost { 
namespace mpl {

template<>
struct gcd_impl< rational_c_tag,rational_c_tag >
{
    template< typename R1, typename R2 > struct apply
        : ratio_gcd<R1, R2>
    {
    };
};    
}
}

#endif  // BOOST_RATIO_MPL_GCD_HPP
