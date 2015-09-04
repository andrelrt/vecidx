#include <iostream>
#include <vector>
#include <boost/timer/timer.hpp>

#include "../vecidx/vector_index.h"


template< typename Index_Size_T >
void bench( const std::string& name, size_t size, size_t loop )
{
    boost::timer::cpu_timer timer;
    std::vector<size_t> org( size );

    srand( 1 );
    std::generate( org.begin(), org.end(), &rand );
    vecidx::vector_index< Index_Size_T, size_t > index( org );

    timer.start();
    index.build_index();
    timer.stop();
    std::cout << name << "_index build: " << timer.format();

    timer.start();
    for( size_t j = 0; j < loop; ++j )
    {
        for( auto i : org )
        {
            index.find( i );
        }
    }
    timer.stop();
    std::cout << name << "_index find all: " << timer.format();
}

int main(int /*argc*/, char* /*argv*/[])
{
    {
        std::cout << "\nsize: 0x100\n\n";
        bench< uint8_t>(  "uint8", 0x100, 100'000 );
        bench<uint16_t>( "uint16", 0x100, 100'000 );
        bench<uint32_t>( "uint32", 0x100, 100'000 );
        bench<uint64_t>( "uint64", 0x100, 100'000 );
    }

    {
        std::cout << "\nsize: 0x1'0000\n\n";
        bench<uint16_t>( "uint16", 0x1'0000, 100 );
        bench<uint32_t>( "uint32", 0x1'0000, 100 );
        bench<uint64_t>( "uint64", 0x1'0000, 100 );
    }

    {
        std::cout << "\nsize: 0x0fff'ffff\n\n";
        bench<uint32_t>( "uint32", 0x0fff'ffff, 1 );
        bench<uint64_t>( "uint64", 0x0fff'ffff, 1 );
    }

    return 0;
}
