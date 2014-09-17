#include "octree.h"

#ifdef WIN32
#include <tchar.h>
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int /* argc */, char** /* argv */)
#endif
{
    /* Create 4096x4096x4096 octree containing doubles. */
    Octree::Octree<double> o(4096);

    /* Put pi in (1,2,3). */
    o(1,2,3) = 3.1416;

    /* Erase that node. */
    o.erase(1,2,3);

    return 0 ;
}

