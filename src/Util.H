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

#endif//ndef ___RENDERA___UTIL_H___
