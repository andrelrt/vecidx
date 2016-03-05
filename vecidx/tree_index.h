#ifndef VECIDX_MAP_INDEX_H
#define VECIDX_MAP_INDEX_H

#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>

namespace vecidx {

template< typename Size_T,
          typename VecType_T,
          typename VecComp_T = std::less<VecType_T> >
class tree_index
{
public:
    typedef Size_T size_type;
    typedef VecType_T vector_type;
    typedef VecComp_T compare_type;
    typedef typename std::vector< vector_type >::const_iterator const_iterator;

    tree_index( const std::vector<VecType_T>& vec ) : vector_( vec ) {}

    void build_index()
    {
        std::vector< size_type > idx;
        idx.resize( vector_.size(), 0 );
        std::iota( idx.begin(), idx.end(), 0 );

        compare_type comp;
        std::sort( idx.begin(), idx.end(),
            [ & ]( const size_type& lhs, const size_type& rhs ) -> bool
            {
                return comp( vector_[ lhs ], vector_[ rhs ] );
            } );

        index_vector_.resize( 1 );

        size_t index_size = get_index_size();

        fill_index( idx.begin(), idx.end(), index_size );
    }
    
    const_iterator find( const vector_type& key ) const
    {
        auto index = find_index( key );

        if( 0 == index.first )
        {
            return index.second;
        }
        
        compare_type comp;
        auto it = std::lower_bound( index_vector_[ index.first ].begin(),
                                    index_vector_[ index.first ].end(), key,
            [ & ]( const size_type& lhs, const size_type& rhs ) -> bool
            {
                return comp( vector_[ lhs ], vector_[ rhs ] );
            });

        if( index_vector_[ index.first ].end() == it )
        {
            return vector_.cend();
        }

        if( !comp( key, vector_[ *it ] ) &&
            !comp( vector_[ *it ], key ) )
        {
            auto ret = vector_.cbegin();
            std::advance( ret, *it );
            return ret;
        }

        return vector_.cend();
    }

private:
    const std::vector< vector_type >& vector_;
    std::vector< std::vector< size_type > > index_vector_;

    static const size_t cache_line_size_ = 64;

    constexpr size_t get_index_size() const
    {
        return ( 16 * cache_line_size_ ) / sizeof( size_type );
    }

    void fill_index( typename std::vector< size_type >::const_iterator begin,
                     typename std::vector< size_type >::const_iterator end,
                     size_t index_size )
    {
        if( 1 == index_size )
        {
            index_vector_.emplace_back( begin, end );
            return;
        }
        size_t diff = std::distance( begin, end );
        if( 0 == diff )
        {
            return;
        }
        if( 1 == diff )
        {
            index_vector_[ 0 ].push_back( *begin );
            return;
        }
        
        auto middle = begin;
        std::advance( middle, diff / 2 );

        index_vector_[ 0 ].push_back( *middle );
        fill_index( begin, middle, index_size / 2 );
        fill_index( middle + 1, end, index_size / 2 );
    }

    std::pair<size_t, const_iterator> find_index( const vector_type& key ) const
    {
        compare_type comp;
        size_t index_size = get_index_size();

        size_t pos = 0;
        size_t ret_index = 1;
        while( pos < index_vector_[ 0 ].size() )
        {
            if( comp( key, vector_[ index_vector_[ 0 ][ pos ] ] ) )
            {
                // key < vector_[ index_vector_[ 0 ][ pos ] ]
                pos++;
            }
            else if( comp( vector_[ index_vector_[ 0 ][ pos ] ], key ) )
            {
                // vector_[ index_vector_[ 0 ][ pos ] ] < key
                pos += index_size / 2;
                ret_index += index_size / 2;
            }
            else
            {
                // vector_[ index_vector_[ 0 ][ pos ] ] == key
                auto ret = vector_.cbegin();
                std::advance( ret, index_vector_[ 0 ][ pos ] );
                return std::make_pair( 0, ret );
            }
            index_size /= 2;

            if( 1 == index_size )
            {
                return std::make_pair( ret_index, vector_.cend() );
            }
        }

        // will never happen
        // but just in case, return end()
        return std::make_pair( 0, vector_.cend() );
    }

};

} // namespace vecidx

#endif // VECIDX_MAP_INDEX_H
