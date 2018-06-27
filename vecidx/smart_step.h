#ifndef VECIDX_SMART_STEP_H
#define VECIDX_SMART_STEP_H

#include <immintrin.h>
#include <x86intrin.h>

std::ostream& operator<<( std::ostream& out, const __m256i& val )
{
    const uint32_t* v = reinterpret_cast<const uint32_t*>( &val );
    out << std::hex << "(";
    for( size_t i = 0; i < 7; ++i )
    {
        out << "0x" << std::setw(8) << std::setfill( '0' ) << v[i] << ", ";
    }
    out << "0x" << std::setw(8) << std::setfill( '0' ) << v[7] << ")";
    return out;
}

namespace vecidx {

template< typename VecType_T > struct smart_index { };

template<> struct smart_index< uint8_t >
{
    using inner_type = __m128i;
    constexpr static size_t array_size = 16/sizeof(uint8_t);
    static inline size_t compare( uint8_t key, __m128i cmp ) {
        uint32_t mask = _mm_movemask_epi8( _mm_cmpgt_epi8( _mm_set1_epi8( key ), cmp ) );
        return _bit_scan_reverse( mask + 1 );
    }
};

template<> struct smart_index< uint16_t >
{
    using inner_type = __m128i;
    constexpr static size_t array_size = 16/sizeof(uint16_t);
    static inline size_t compare( uint16_t key, __m128i cmp ) {
        uint32_t mask = _mm_movemask_epi8( _mm_cmpgt_epi16( _mm_set1_epi16( key ), cmp ) );
        return _bit_scan_reverse( mask + 1 ) >> 1;
    }
};

template<> struct smart_index< uint32_t >
{
    using inner_type = __m128i;
    constexpr static size_t array_size = 16/sizeof(uint32_t);
    static inline size_t compare( uint32_t key, __m128i cmp ) {
        uint32_t mask = _mm_movemask_epi8( _mm_cmpgt_epi32( _mm_set1_epi32( key ), cmp ) );
        return _bit_scan_reverse( mask + 1 ) >> 2;
    }
};

template<> struct smart_index< uint64_t >
{
    using inner_type = __m128i;
    constexpr static size_t array_size = 16/sizeof(uint64_t);
    static inline size_t compare( uint64_t key, __m128i cmp ) {
        uint32_t mask = _mm_movemask_epi8( _mm_cmpgt_epi64( _mm_set1_epi64x( key ), cmp ) );
        return _bit_scan_reverse( mask + 1 ) >> 3;
    }
};

template< typename DUMMY_T, typename VecType_T >
class smart_step
{
public:
    using value_type    = VecType_T;
    using const_iterator = typename std::vector< value_type >::const_iterator;

    smart_step( const std::vector< value_type >& ref )
        : ref_( ref ){}

    void build_index()
    {
        size_t step = ref_.size() / (array_size + 1);

        const_iterator end = ref_.begin();
        value_type* pCmp = reinterpret_cast< value_type* >( &cmp_ );
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( end, step );
            *pCmp = *end;
            ++pCmp;
        }
        //std::cout << "Cmp: " << cmp_ << std::endl;
    }

    const_iterator find( const value_type& key ) const
    {
        size_t i = smart_index< value_type >::compare( key, cmp_ );
        size_t step = ref_.size() / (array_size + 1);

        const_iterator beg = ref_.begin();
        std::advance( beg, i * step );
        const_iterator end;
        if( i == array_size )
        {
            end = ref_.end();
        }
        else
        {
            end = beg;
            std::advance( end, step );
            ++end;
        }

        auto first = std::lower_bound( beg, end, key );
        return (first!=end && !(key<*first)) ? first : ref_.end();
    }

private:
    const std::vector< value_type >& ref_;
    typename smart_index< value_type >::inner_type cmp_;
    constexpr static size_t array_size = smart_index< value_type >::array_size;
};

//Two-level smart_step
template< typename DUMMY_T, typename VecType_T >
class smart_step2
{
public:
    using value_type    = VecType_T;
    using const_iterator = typename std::vector< value_type >::const_iterator;

    smart_step2( const std::vector< value_type >& ref )
        : ref_( ref ){}

    void build_index()
    {
        size_t step = ref_.size() / (array_size + 1);

        const_iterator end = ref_.begin();
        value_type* pCmp = reinterpret_cast< value_type* >( cmp_.data() );
        for( size_t i = 0; i < array_size; ++i )
        {
            const_iterator beg = end;
            std::advance( end, step );
            cmp_[ i+1 ] = build_index( beg, end+1 );

            *pCmp = *end;
            ++pCmp;
        }
        cmp_[ array_size +1 ] = build_index( end, ref_.end() );
        //std::cout << "Cmp: " << cmp_[0] << std::endl;
    }

    const_iterator find( const value_type& key ) const
    {
        size_t i = smart_index< value_type >::compare( key, cmp_[0] );
        size_t j = smart_index< value_type >::compare( key, cmp_[i+1] );
        size_t step = ref_.size() / (array_size + 1);

        const_iterator beg = ref_.begin();
        std::advance( beg, i * step );

        const_iterator end = beg;
        std::advance( end, step );
        ++end;

        step = step / (array_size +1);
        std::advance( beg, j * step );

        if( i == array_size && j == array_size )
        {
            end = ref_.end();
        }
        else if( j != array_size )
        {
            end = beg;
            std::advance( end, step );
            ++end;
        }

        auto first = std::lower_bound( beg, end, key );
        return (first!=end && !(key<*first)) ? first : ref_.end();
    }

private:
    constexpr static size_t array_size = smart_index< value_type >::array_size;

    const std::vector< value_type >& ref_;
    std::array< typename smart_index< value_type >::inner_type, array_size +2 > cmp_;

    typename smart_index< value_type >::inner_type build_index( const_iterator begin, const_iterator end )
    {
        size_t size = std::distance( begin, end );
        size_t step = size / (array_size + 1);

        typename smart_index< value_type >::inner_type ret;
        value_type* pRet = reinterpret_cast< value_type* >( &ret );

        const_iterator it = begin;
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( it, step );
            *pRet = *it;
            ++pRet;
        }
        //std::cout << "Cmp: " << ret << " - beg, end, size: " << *begin << ", " << *end << ", " << size << std::endl;
        return ret;
    }
};

//any container smart_step
template< class Cont_T >
class any_smart_step
{
public:
	using container_type = Cont_T;
    using value_type     = typename container_type::value_type;
    using const_iterator = typename container_type::const_iterator;

    any_smart_step( const container_type& ref )
        : ref_( ref ){}

    void build_index()
    {
        size_t step = ref_.size() / (array_size + 1);

        value_type* pCmp = reinterpret_cast< value_type* >( &cmp_ );
        const_iterator it = ref_.begin();
        ranges_[ 0 ] = it;
        for( size_t i = 1; i <= array_size; ++i )
        {
            std::advance( it, step );
            ranges_[ i ] = it;
            *pCmp = *it;
            ++pCmp;
        }
        ranges_[ array_size+1 ] = std::prev(ref_.end());
    }

    const_iterator find( const value_type& key ) const
    {
        size_t i = smart_index< value_type >::compare( key, cmp_ );
        auto beg = ranges_[ i ];
        auto end = std::next( ranges_[ i + 1 ] );
        auto first = std::lower_bound( beg, end, key );
        return (first!=end && !(key<*first)) ? first : ref_.end();
    }

private:
    constexpr static size_t array_size = smart_index< value_type >::array_size;

    const container_type& ref_;
    typename smart_index< value_type >::inner_type cmp_;
    std::array< const_iterator, array_size + 2 > ranges_;
};

//n-level smart_step
//template< typename DUMMY_T, typename VecType_T >
//class smart_stepN
//{
//public:
//    using value_type    = VecType_T;
//    using const_iterator = typename std::vector< value_type >::const_iterator;
//
//    smart_stepN( const std::vector< value_type >& ref )
//        : ref_( ref ), range_( std::make_pair( ref.begin(), ref.end() ) ) {}
//
//    void build_index()
//    {
//        size_t size = std::distance( range_.first, range_.second );
//        size_t step = size / (array_size + 1);
//
//        value_type* pCmp = reinterpret_cast< value_type* >( &cmp_ );
//
//        const_iterator it = range_.first;
//        for( size_t i = 0; i < array_size; ++i )
//        {
//            const_iterator beg = it;
//            std::advance( it, step );
//
//            *pCmp = *it;
//            ++pCmp;
//            std::advance( end, step );
//            cmp_[ i+1 ] = build_index( beg, end+1 );
//
//            *pCmp = *end;
//            ++pCmp;
//        }
//        //std::cout << "Cmp: " << cmp_ << " - beg, end, size: " << *range_.first << ", " << *range_.second << ", " << size << std::endl;
//    }
//
//    const_iterator find( const value_type& key ) const
//    {
//    }
//
//private:
//    constexpr static size_t array_size = 32/sizeof(value_type);
//
//    const std::vector< value_type >& ref_;
//    typename smart_index< value_type >::index_type cmp_;
//    const std::pair< const_iterator, const_iterator > range_;
//
//    smart_stepN( const std::vector< value_type >& ref, const_iterator beg, const_iterator end )
//        : ref_( ref ), range_( std::make_pair( beg, end ) ) {}
//};

} // namespace vecidx

#endif // VECIDX_SMART_STEP_H
