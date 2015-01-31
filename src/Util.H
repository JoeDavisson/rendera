/* rendera/Util.H */

/* rendera's home for wayward functions */

#ifndef ___RENDERA___UTIL_H___
#define ___RENDERA___UTIL_H___

#include <string>

namespace Util
{
  std::string readlink( std::string const& );

  std::string parent_path( std::string const& );

  std::string executable_path( void );

  std::string bindir_path( void );

  std::string usrdir_path( void );
}

namespace ren
{
  template< typename T >
  inline
  T abs( T const n ){ return ( 0 > n ) ? -n : n ; }
}

#endif//ndef ___RENDERA___UTIL_H___
