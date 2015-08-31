#include <iostream>
#include <boost/timer/timer.hpp>

#include "../vecidx/vector_index.h"


template< typename Index_Size_T >
void bench( const std::vector<size_t>& org, const std::string& name )
{
    boost::timer::cpu_timer timer;
    std::vector<size_t> copy(org);
    vecidx::vector_index< Index_Size_T, size_t > index( copy );

    timer.start();
    index.build_index();
    timer.stop();
    std::cout << name << "_index build: " << timer.format();

    timer.start();
    for( size_t j = 0; j < 1000; ++j )
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
    std::vector<size_t> org(255);

    srand(1);
    std::generate( org.begin(), org.end(), &rand );

    bench< uint8_t>(org,  "uint8");
    bench<uint16_t>(org, "uint16");
    bench<uint32_t>(org, "uint32");
    bench<uint64_t>(org, "uint64");

    return 0;
}
