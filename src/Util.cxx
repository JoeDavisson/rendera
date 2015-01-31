/* rendera/Util.cxx */

#if HAVE_CONFIG_H
#  include "config.h"
#else
#  error "missing config.h"
#endif

#include <algorithm>
#include <climits>
#include <unistd.h>

#include "Util.H"

namespace
{
  std::string
  _readlink( std::string const&path )
  {
    char buf[ PATH_MAX ] = { 0 };
    ssize_t count = readlink( path.c_str(), buf, PATH_MAX );
    return std::string( buf, std::max( count, ssize_t( 0 ) ) );
  }
}

std::string
Util::readlink( std::string const&path )
{
  return _readlink( path );
}

std::string
Util::parent_path( std::string const&path )
{
  return path.substr( 0, path.find_last_of( "/\\" ) );
}

std::string
Util::executable_path( void )
{
  return Util::readlink( "/proc/self/exe" );
}

std::string
Util::bindir_path( void )
{
  return Util::parent_path( Util::executable_path() );
}

std::string
Util::usrdir_path( void )
{
  return Util::parent_path( Util::bindir_path() );
}

