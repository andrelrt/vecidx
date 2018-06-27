#include <immintrin.h>
#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <algorithm>
#include <numeric>
#include <list>
#include <map>
#include <boost/timer/timer.hpp>

#include "../vecidx/vector_index.h"
#include "../vecidx/search_index.h"
#include "../vecidx/tree_index.h"
#include "../vecidx/smart_step.h"

template< class Cont_T >
struct container_only
{
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    container_only( const container_type& ref ) : ref_( ref ){}

    void build_index(){}

    const_iterator find( const value_type& key )
    {
        auto first = std::lower_bound( ref_.begin(), ref_.end(), key );
        return (first!=ref_.end() && !(key<*first)) ? first : ref_.end();
    }
private:
    const container_type& ref_;
};

template< template < typename... > class Index_T, typename Index_Size_T >
size_t bench( const std::string& name, size_t size, size_t loop )
{
    boost::timer::cpu_timer timer;
    std::vector<uint32_t> org( size );

    //srand( 1 );
    //std::generate( org.begin(), org.end(), &rand );
    std::iota( org.begin(), org.end(), 0 );
    Index_T< Index_Size_T, uint32_t > index( org );

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
                std::cout << "end- " << std::hex << i << std::endl;
                break;
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
    //std::cout << "Raw wall: " << timer.elapsed().wall << std::endl;

    return timer.elapsed().wall;
}

template< class Cont_T, template < typename... > class Index_T >
size_t bench_any( const std::string& name, size_t size, size_t loop )
{
	using container_type = Cont_T;
    using index_type = Index_T< container_type >;

    boost::timer::cpu_timer timer;
    container_type org;

    size_t cnt = 0;
    std::generate_n( std::back_inserter(org), size, [&cnt](){ return cnt++; } );
    index_type index( org );

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
                std::cout << "end- " << std::hex << i << std::endl;
                break;
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
    //std::cout << "Raw wall: " << timer.elapsed().wall << std::endl;

    return timer.elapsed().wall;
}

int main(int /*argc*/, char* /*argv*/[])
{
    //bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 15, 1 );

//    {
//        std::cout << "\nsize: 0xff\n\n";
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only,  uint8", 0xff, 100000 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint16", 0xff, 100000 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint32", 0xff, 100000 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint64", 0xff, 100000 );
//
//        std::cout << "\n";
//        bench<vecidx::vector_index,  uint8_t>( "vecidx::vector_index,  uint8", 0xff, 100000 );
//        bench<vecidx::vector_index, uint16_t>( "vecidx::vector_index, uint16", 0xff, 100000 );
//        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0xff, 100000 );
//        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0xff, 100000 );
//
//        std::cout << "\n";
//        bench<vecidx::search_index, uint8_t>( "vecidx::search_index,  uint8", 0xff, 100000 );
//        bench<vecidx::search_index, uint16_t>( "vecidx::search_index, uint16", 0xff, 100000 );
//        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0xff, 100000 );
//        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0xff, 100000 );
//
////        std::cout << "\n";
////        bench<vecidx::tree_index,  uint8_t>( "vecidx::tree_index,  uint8", 0xff, 100000 );
////        bench<vecidx::tree_index, uint16_t>( "vecidx::tree_index, uint16", 0xff, 100000 );
////        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0xff, 100000 );
////        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0xff, 100000 );
//
//        std::cout << "\n";
//        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0xff, 100000 );
//        bench<vecidx::smart_step2, uint32_t>( "vecidx::smart_step2, uint32", 0xff, 100000 );
//    }
//
//    {
//        std::cout << "\nsize: 0xffff\n\n";
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint16", 0xffff, 100 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint32", 0xffff, 100 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint64", 0xffff, 100 );
//
//        std::cout << "\n";
//        bench<vecidx::vector_index, uint16_t>( "vecidx::vector_index, uint16", 0xffff, 100 );
//        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0xffff, 100 );
//        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0xffff, 100 );
//
//        std::cout << "\n";
//        bench<vecidx::search_index, uint16_t>( "vecidx::search_index, uint16", 0xffff, 100 );
//        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0xffff, 100 );
//        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0xffff, 100 );
//
////        std::cout << "\n";
////        bench<vecidx::tree_index, uint16_t>( "vecidx::tree_index, uint16", 0xffff, 100 );
////        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0xffff, 100 );
////        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0xffff, 100 );
//
//        std::cout << "\n";
//        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0xffff, 100 );
//        bench<vecidx::smart_step2, uint32_t>( "vecidx::smart_step2, uint32", 0xffff, 100 );
//    }
//
//    {
//        std::cout << "\nsize: 0x03ff'ffff\n\n";
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint32", 0x03ffffff, 1 );
//        bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint64", 0x03ffffff, 1 );
//
//        std::cout << "\n";
//        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0x03ffffff, 1 );
//        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0x03ffffff, 1 );
//
//        std::cout << "\n";
//        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0x03ffffff, 1 );
//        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0x03ffffff, 1 );
//
////        std::cout << "\n";
////        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0x03ffffff, 1 );
////        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0x03ffffff, 1 );
//
//        std::cout << "\n";
//        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0x03ffffff, 1 );
//        bench<vecidx::smart_step2, uint32_t>( "vecidx::smart_step2, uint32", 0x03ffffff, 1 );
//    }

    std::cout << "\nsize: 0x00ff'ffff\n\n";
    while( 1 )
    {
        size_t base = bench_any< std::vector< uint32_t >, container_only >( "vector_only, uint32", 0x00ffffff, 10 );
//        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0x00ffffff, 10 );
//        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0x00ffffff, 10 );
//        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0x00ffffff, 10 );
        size_t smart2 = bench<vecidx::smart_step2, uint32_t>( "vecidx::smart_step2,  uint32", 0x00ffffff, 10 );
        size_t smart1 = bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step,  uint32", 0x00ffffff, 10 );
        size_t smart3 = bench_any< std::vector< uint32_t >,
                                   vecidx::any_smart_step >( "vecidx::any_smart_step, uint32", 0x00ffffff, 10 );

        size_t lbase  = bench_any< std::list< uint32_t >, container_only >( "list_only, uint32", 0x0001ffff, 1 );
        size_t lsmart = bench_any< std::list< uint32_t >,
                                   vecidx::any_smart_step >( "vecidx::any_smart_step, uint32", 0x0001ffff, 1 );

        std::cout << std::endl << "Smart Step Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) smart1)/((float) base) - 1.0f) << "%"
                  << std::endl << "Smart2 Step Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) smart2)/((float) base) - 1.0f) << "%"
                  << std::endl << "Smart3 Step Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) smart3)/((float) base) - 1.0f) << "%"
                  << std::endl << "Smart2/Smart1 Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) smart2)/((float) smart1) - 1.0f) << "%"
                  << std::endl << "Smart3/Smart1 Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) smart3)/((float) smart1) - 1.0f) << "%"
                  << std::endl << "List Step Diff: " << std::fixed << std::setprecision(2)
                  << 100.0f * (((float) lsmart)/((float) lbase) - 1.0f) << "%"
                  << std::endl << std::endl;
    }
    return 0;
}
