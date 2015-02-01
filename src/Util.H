/* rendera/Util.H */

/* rendera's home for wayward functions */

#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <string>

namespace Util
{
  namespace
  {
    template <typename UnaryFunc>
    struct _comparator
    {
      UnaryFunc unaryfunc_;

      explicit _comparator(UnaryFunc unaryfunc):unaryfunc_(unaryfunc)
      {
      }

      template< typename T >
      bool operator()(T const &lhs, T const &rhs)
      {
        return (unaryfunc_(lhs) < unaryfunc_(rhs));
      }
    };
  }

  // template for palette sorting etc
  template <typename Iter, typename UnaryFunc>
  void sortBy(Iter first, Iter last, UnaryFunc unaryfunc)
  {
    std::sort(first, last, _comparator <UnaryFunc>(unaryfunc));
  }

  // sign, used by fastStretch in Bitmap.cxx
  template <typename T>
  inline T sign( T const &v )
  {
    static T const zero(0);
    return (zero > v) ? T(-1) : T(1);
  }


  // absolute value
  template <typename T>
  inline T abs(T const n)
  {
    return (0 > n) ? -n : n;
  }

  std::string readlink( std::string const &);

  std::string parent_path( std::string const &);

  std::string executable_path(void);

  std::string bindir_path(void);

  std::string usrdir_path(void);
}

#endif

