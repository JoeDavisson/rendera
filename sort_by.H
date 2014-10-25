/* rendera/sort_by.H */


#ifndef ___RENDERA___SORT_BY_H___
#define ___RENDERA___SORT_BY_H___


namespace rendera
{
  namespace
  {
    template< typename UnaryFunc >
    struct _comparator
    {
      UnaryFunc unaryfunc_;

      explicit _comparator( UnaryFunc unaryfunc ):unaryfunc_( unaryfunc ){}

      template< typename T >
      bool
      operator()( T const&lhs, T const&rhs )
      {
        return ( unaryfunc_( lhs ) < unaryfunc_( rhs ) ) ;
      }
    };
  }

  template< typename Iter, typename UnaryFunc >
  void
  sort_by( Iter first, Iter last, UnaryFunc unaryfunc )
  {
    std::sort( first, last, _comparator< UnaryFunc >( unaryfunc ) );
  }
}


#endif//ndef ___RENDERA___SORT_BY_H___
