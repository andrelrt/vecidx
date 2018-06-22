#ifndef VECIDX_SMART_STEP_H
#define VECIDX_SMART_STEP_H

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
    static inline size_t compare( uint8_t key, __m256i cmp ) {
        uint32_t mask = _mm256_movemask_epi8( _mm256_cmpgt_epi8( _mm256_set1_epi8( key ), cmp ) );
        return (mask == 0) ? 0 : _bit_scan_reverse( mask ) + 1;
    }
};

template<> struct smart_index< uint16_t >
{
    static inline size_t compare( uint16_t key, __m256i cmp ) {
        uint32_t mask = _mm256_movemask_epi8( _mm256_cmpgt_epi16( _mm256_set1_epi16( key ), cmp ) );
        return (mask == 0) ? 0 : (_bit_scan_reverse( mask ) + 1) >> 1;
    }
};

template<> struct smart_index< uint32_t >
{
    static inline size_t compare( uint32_t key, __m256i cmp ) {
        uint32_t mask = _mm256_movemask_epi8( _mm256_cmpgt_epi32( _mm256_set1_epi32( key ), cmp ) );
        return (mask == 0) ? 0 : (_bit_scan_reverse( mask ) + 1) >> 2;
    }
};

template<> struct smart_index< uint64_t >
{
    static inline size_t compare( uint64_t key, __m256i cmp ) {
        uint32_t mask = _mm256_movemask_epi8( _mm256_cmpgt_epi64( _mm256_set1_epi64x( key ), cmp ) );
        return (mask == 0) ? 0 : (_bit_scan_reverse( mask ) + 1) >> 3;
    }
};

template< typename DUMMY_T, typename VecType_T >
class smart_step
{
public:
    using vector_type    = VecType_T;
    using const_iterator = typename std::vector< vector_type >::const_iterator;

    smart_step( const std::vector< vector_type >& ref )
        : cmp_( _mm256_set1_epi64x( 0 ) ), ref_( ref ){}

    void build_index()
    {
        size_t step = ref_.size() / (array_size + 1);

        const_iterator end = ref_.begin();
        vector_type* pCmp = reinterpret_cast< vector_type* >( &cmp_ );
        for( size_t i = 0; i < array_size; ++i )
        {
            std::advance( end, step );
            *pCmp = *end;
            ++pCmp;
        }
        //std::cout << "Cmp: " << cmp_ << std::endl;
    }

    const_iterator find( const vector_type& key ) const
    {
        size_t i = smart_index< vector_type >::compare( key, cmp_ );
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
    __m256i cmp_;
    const std::vector< vector_type >& ref_;
    constexpr static size_t array_size = 32/sizeof(vector_type);
};

//Two-level smart_step
template< typename DUMMY_T, typename VecType_T >
class smart_step2
{
public:
    using vector_type    = VecType_T;
    using const_iterator = typename std::vector< vector_type >::const_iterator;

    smart_step2( const std::vector< vector_type >& ref )
        : ref_( ref ){}

    void build_index()
    {
        size_t step = ref_.size() / (array_size + 1);

        const_iterator end = ref_.begin();
        vector_type* pCmp = reinterpret_cast< vector_type* >( cmp_.data() );
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

    const_iterator find( const vector_type& key ) const
    {
        size_t i = smart_index< vector_type >::compare( key, cmp_[0] );
        size_t j = smart_index< vector_type >::compare( key, cmp_[i+1] );
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
    constexpr static size_t array_size = 32/sizeof(vector_type);

    const std::vector< vector_type >& ref_;
    std::array< __m256i, array_size +2 > cmp_;

    __m256i build_index( const_iterator begin, const_iterator end )
    {
        size_t size = std::distance( begin, end );
        size_t step = size / (array_size + 1);

        __m256i ret;
        vector_type* pRet = reinterpret_cast< vector_type* >( &ret );

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

} // namespace vecidx

#endif // VECIDX_SMART_STEP_H
