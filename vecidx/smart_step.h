#ifndef VECIDX_SMART_STEP_H
#define VECIDX_SMART_STEP_H

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
       
        const_iterator beg;
        const_iterator end = ref_.begin();
        for( size_t i = 0; i < array_size; ++i )
        {
            beg = end; 
            std::advance( end, step ); 
            ranges_[ i ] = std::make_pair( beg, end +1 );
        } 
        ranges_[ array_size ] = std::make_pair( end, ref_.end() );

        vector_type* pCmp = reinterpret_cast< vector_type* >( &cmp_ );
        for( size_t i = 1; i <= array_size; ++i )
        {
            *pCmp = *ranges_[ i ].first;
            ++pCmp;
        }
    }

    const_iterator find( const vector_type& key ) const
    {
        size_t i = smart_index< vector_type >::compare( key, cmp_ );
        auto first = std::lower_bound( ranges_[i].first, ranges_[i].second, key );
        return (first!=ranges_[i].second && !(key<*first)) ? first : ref_.end();
    }

private:
    __m256i cmp_;
    const std::vector< vector_type >& ref_;
    constexpr static size_t array_size = 32/sizeof(vector_type);
    std::array< std::pair< const_iterator, const_iterator >, array_size + 1 > ranges_;
};

} // namespace vecidx

#endif // VECIDX_SMART_STEP_H
