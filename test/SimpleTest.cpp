#include <immintrin.h>
#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/timer/timer.hpp>

#include "../vecidx/vector_index.h"
#include "../vecidx/search_index.h"
#include "../vecidx/tree_index.h"
#include "../vecidx/smart_step.h"

template< typename T1, typename T2 >
struct vector_only
{
    vector_only( const std::vector< T2 >& ref ) : ref_( ref ){}

    void build_index(){}

    typename std::vector< T2 >::const_iterator find( const T2& t2 )
    {
        auto first = std::lower_bound( ref_.begin(), ref_.end(), t2 );
        return (first!=ref_.end() && !(t2<*first)) ? first : ref_.end();
    }
private:
    const std::vector< T2 >& ref_;
};

//template< typename T1, typename T2 >
//struct lsmart_step
//{
//    lsmart_step( const std::vector< T2 >& ref ) : ref_( ref ){}
//
//    void build_index()
//    {
//        step_ = ref_.size() / 9;
//       
//        typename std::vector< T2 >::const_iterator beg;
//        typename std::vector< T2 >::const_iterator end = ref_.begin();
//        
//        beg = end; std::advance( end, step_ ); ranges_[ 0 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 1 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 2 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 3 ] = std::make_pair( beg, end +1 );
//        
//        beg = end; std::advance( end, step_ ); ranges_[ 4 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 5 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 6 ] = std::make_pair( beg, end +1 );
//        beg = end; std::advance( end, step_ ); ranges_[ 7 ] = std::make_pair( beg, end +1 );
//
//        ranges_[ 8 ] = std::make_pair( end, ref_.end() );
//        
//        cmp_ = _mm256_set_epi32( *ranges_[8].first, *ranges_[7].first, *ranges_[6].first, *ranges_[5].first,
//                                 *ranges_[4].first, *ranges_[3].first, *ranges_[2].first, *ranges_[1].first  ); 
//        idx_ = _mm256_set1_epi32( 1 );
//    }
//
//    typename std::vector< T2 >::const_iterator find( const T2& t2 )
//    {
//        uint32_t mask = _mm256_movemask_epi8( _mm256_cmpgt_epi32( _mm256_set1_epi32( t2 ), cmp_ ) );
//        size_t i = (mask == 0) ? 0 : (_bit_scan_reverse( mask ) + 1) >> 2;
//
//        //std::cout << t2 << " - " << begin << std::endl
//        //          << pm[ 0 ] << " - "
//        //          << pm[ 1 ] << " - "
//        //          << pm[ 2 ] << " - "
//        //          << pm[ 3 ] << " - "
//        //          << std::endl;
//
//        auto first = std::lower_bound( ranges_[i].first, ranges_[i].second, t2 );
//        return (first!=ranges_[i].second && !(t2<*first)) ? first : ref_.end();
//    }
//private:
//    size_t step_;
//    __m256i cmp_;
//    __m256i idx_;
//    const std::vector< T2 >& ref_;
//    std::array< std::pair< typename std::vector< T2 >::const_iterator, 
//                           typename std::vector< T2 >::const_iterator >, 9 > ranges_;
//};


template< template < typename... > class Index_T, typename Index_Size_T >
size_t bench( const std::string& name, size_t size, size_t loop )
{
    boost::timer::cpu_timer timer;
    std::vector<uint32_t> org( size );

    //srand( 1 );
    //std::generate( org.begin(), org.end(), &rand );
    std::iota( org.begin(), org.end(), 0 );
    Index_T< Index_Size_T, uint32_t > index( org );

    //timer.start();
    index.build_index();
    //timer.stop();
    //std::cout << name << "_index build...: " << timer.format();

    timer.start();
    for( size_t j = 0; j < loop; ++j )
    {
        for( auto i : org )
        {
            auto ret = index.find( i );

            if( ret == org.end() )
            {
                std::cout << "end- " << i << std::endl;
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

    return timer.elapsed().user;
}

int main(int /*argc*/, char* /*argv*/[])
{
    //bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 15, 1 );

    {
        std::cout << "\nsize: 0xff\n\n";
        bench<vecidx::vector_index,  uint8_t>( "vecidx::vector_index,  uint8", 0xff, 100000 );
        bench<vecidx::vector_index, uint16_t>( "vecidx::vector_index, uint16", 0xff, 100000 );
        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0xff, 100000 );
        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0xff, 100000 );

        std::cout << "\n";
        bench<vecidx::search_index, uint8_t>( "vecidx::search_index,  uint8", 0xff, 100000 );
        bench<vecidx::search_index, uint16_t>( "vecidx::search_index, uint16", 0xff, 100000 );
        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0xff, 100000 );
        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0xff, 100000 );

//        std::cout << "\n";
//        bench<vecidx::tree_index,  uint8_t>( "vecidx::tree_index,  uint8", 0xff, 100000 );
//        bench<vecidx::tree_index, uint16_t>( "vecidx::tree_index, uint16", 0xff, 100000 );
//        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0xff, 100000 );
//        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0xff, 100000 );

        std::cout << "\n";
        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0xff, 100000 );
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

//        std::cout << "\n";
//        bench<vecidx::tree_index, uint16_t>( "vecidx::tree_index, uint16", 0xffff, 100 );
//        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0xffff, 100 );
//        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0xffff, 100 );

        std::cout << "\n";
        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0xffff, 100 );
    }

    {
        std::cout << "\nsize: 0x03ff'ffff\n\n";
        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0x03ffffff, 1 );
        bench<vecidx::vector_index, uint64_t>( "vecidx::vector_index, uint64", 0x03ffffff, 1 );

        std::cout << "\n";
        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0x03ffffff, 1 );
        bench<vecidx::search_index, uint64_t>( "vecidx::search_index, uint64", 0x03ffffff, 1 );

//        std::cout << "\n";
//        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0x03ffffff, 1 );
//        bench<vecidx::tree_index, uint64_t>( "vecidx::tree_index, uint64", 0x03ffffff, 1 );

        std::cout << "\n";
        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step, uint32", 0x03ffffff, 1 );
    }

//    std::cout << "\nsize: 0x000f'ffff\n\n";
//    while( 1 ) 
//    {
//        size_t base = bench<vector_only, uint32_t>( "vector_only, uint32", 0x000fffff, 100 );
//        bench<vecidx::vector_index, uint32_t>( "vecidx::vector_index, uint32", 0x000fffff, 100 );
//        bench<vecidx::search_index, uint32_t>( "vecidx::search_index, uint32", 0x000fffff, 100 );
//        bench<vecidx::tree_index, uint32_t>( "vecidx::tree_index, uint32", 0x000fffff, 100 );
//        bench<vecidx::smart_step, uint32_t>( "vecidx::smart_step,  uint32", 0x000fffff, 100 );
//        size_t late = bench<lsmart_step, uint32_t>( "lsmart_step,  uint32", 0x000fffff, 100 );
//
//        std::cout << std::endl << "Smart Step Diff: " << std::fixed << std::setprecision(2)
//                  << 100.0f * (((float) late)/((float) base) - 1.0f) << "%" 
//                  << std::endl << std::endl;
//    }
    return 0;
}
