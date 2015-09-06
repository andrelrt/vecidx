#include <iostream>
#include <vector>
#include <boost/timer/timer.hpp>

#include "../vecidx/vector_index.h"
#include "../vecidx/search_index.h"


template< template < typename... > class Index_T, typename Index_Size_T >
void bench( const std::string& name, size_t size, size_t loop )
{
    boost::timer::cpu_timer timer;
    std::vector<size_t> org( size );

    srand( 1 );
    std::generate( org.begin(), org.end(), &rand );
    Index_T< Index_Size_T, size_t > index( org );

    timer.start();
    index.build_index();
    timer.stop();
    std::cout << name << "_index build...: " << timer.format();

    timer.start();
    for( size_t j = 0; j < loop; ++j )
    {
        for( auto i : org )
        {
            auto ret = index.find( i );

            if( ret == org.end() )
            {
                std::cout << "end-";
            }
            else
            {
                if( *ret != i )
                {
                    std::cout << *ret << "," << i << "-";
                }
            }
        }
    }
    timer.stop();
    std::cout << name << "_index find all: " << timer.format();
}

int main(int /*argc*/, char* /*argv*/[])
{
    //bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 15, 1 );

    {
        std::cout << "\nsize: 0xff\n\n";
        bench<vecidx::vector_index,  uint8_t>( "vecidx::vector_index,  uint8", 0xff, 100'000 );
        bench<vecidx::vector_index, uint16_t>( "vecidx::vector_index, uint16", 0xff, 100'000 );
        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0xff, 100'000 );
        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0xff, 100'000 );

        std::cout << "\n";
        bench<vecidx::search_index,  uint8_t>( "vecidx::search_index,  uint8", 0xff, 100'000 );
        bench<vecidx::search_index, uint16_t>( "vecidx::search_index, uint16", 0xff, 100'000 );
        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0xff, 100'000 );
        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0xff, 100'000 );
    }

    {
        std::cout << "\nsize: 0xffff\n\n";
        bench<vecidx::vector_index, uint16_t>( "vecidx::vector_index, uint16", 0xffff, 100 );
        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0xffff, 100 );
        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0xffff, 100 );

        std::cout << "\n";
        bench<vecidx::search_index, uint16_t>( "vecidx::search_index, uint16", 0xffff, 100 );
        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0xffff, 100 );
        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0xffff, 100 );
    }

    {
        std::cout << "\nsize: 0x0fff'ffff\n\n";
        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0x0fff'ffff, 1 );
        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0x0fff'ffff, 1 );

        std::cout << "\n";
        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0x0fff'ffff, 1 );
        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0x0fff'ffff, 1 );
    }
    return 0;
}
