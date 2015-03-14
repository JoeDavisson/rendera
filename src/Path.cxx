/*
Copyright (c) 2015 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

// skip this entirely on windows
#ifndef __WIN32

#if HAVE_CONFIG_H
#  include "config.h"
#else
#  error "missing config.h"
#endif

#include <climits>
#include <string>
#include <unistd.h>

#include "Path.H"

namespace
{
  std::string _readlink(std::string const &path)
  {
    char buf[PATH_MAX] = { 0 };
    ssize_t count = readlink(path.c_str(), buf, PATH_MAX);
    return std::string(buf, std::max(count, ssize_t(0)));
  }

  std::string _executable( void )
  {
    return _readlink("/proc/self/exe");
  }
}

std::string Path::basename( std::string const&path )
{
  return path.substr( path.find_last_of("/\\") + 1 );
}

std::string Path::parent(std::string const &path)
{
  return path.substr(0, path.find_last_of("/\\"));
}

std::string Path::executable(void)
{
  return _executable();
}

std::string Path::bindir(void)
{
  return Path::parent(Path::executable());
}

std::string Path::usrdir(void)
{
  return Path::parent(Path::bindir());
}
#endif

