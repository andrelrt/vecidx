#ifndef VECIDX_SEARCH_INDEX_H
#define VECIDX_SEARCH_INDEX_H

#include <iostream>
#include <cstdint>
#include <functional>
#include <algorithm>

namespace vecidx {

template< typename Size_T,
          typename VecType_T,
          typename VecComp_T = std::less<VecType_T> >
class search_index
{
public:
    typedef Size_T size_type;
    typedef VecType_T vector_type;
    typedef VecComp_T compare_type;
    typedef typename std::vector< vector_type >::const_iterator const_iterator;

    search_index( const std::vector<VecType_T>& vec ) : vector_(vec) {}


    void prebuild( std::vector< vector_type >& vec, size_t start, size_t end )
    {
        size_t diff = end - start;
        if( 1 == diff )
        {
            vec.push_back( start );
            return;
        } 
        if( 0 == diff )
        {
            return;
        }
        size_t mid = diff / 2;

        vec.push_back( start + mid );

        prebuild( vec, start, start + mid );
        prebuild( vec, start + mid + 1, end );
    }

    void build_index()
    {
        std::vector< size_type > idx;
        idx.resize( vector_.size(), 0 );
        size_type value = 0;
        std::generate( idx.begin(), idx.end(),
                       [&](){ return value++; });

        compare_type comp;
        std::sort( idx.begin(), idx.end(),
                   [&]( const size_type& lhs, const size_type& rhs )
                   {
                       return comp( vector_[lhs], vector_[rhs] );
                   });

        index_.clear();
        index_.reserve( idx.size() );

        std::vector< vector_type > vec;
        vec.reserve( idx.size() );
        prebuild( vec, 0, idx.size() );

        index_.push_back( vec[ 0 ] );

        size_t mid = vec.size() - 1;

        if( mid & 1 ) // odd
        {
            mid = mid / 2 + 1;
        }
        else // even
        {
            mid = mid / 2;
        }

        //size_t pos = 0;
        //size_type size = static_cast<size_type>( idx.size() );
        //index_.push_back( static_cast<size_type>( idx.size() / 2 ) );

        //size_t count = 1;
        //size_t div = 4;
        //while( size > div )
        //{
        //    for( size_t i = 0; i < count; ++i )
        //    {
        //        std::cout << "(" << (int) index_[ pos ] << ", " << size / div << ") - ";

        //        if( ( index_[ pos ] - last_value ) & 1 ) // odd
        //        {
        //            index_.push_back( static_cast<size_type>( index_[ pos ] - size / div - 1 ) );
        //            index_.push_back( static_cast<size_type>( index_[ pos ] + size / div + 1 ) );
        //        }
        //        else // even
        //        {
        //            index_.push_back( static_cast<size_type>( index_[ pos ] - size / div - 1 ) );
        //            index_.push_back( static_cast<size_type>( index_[ pos ] + size / div ) );
        //        }


        //        ++pos;
        //    }
        //    div <<= 1;
        //    count <<= 1;
        //}
        //size_t rsize = index_.size();
        //for( size_t i = 0; i < count; ++i )
        //{
        //    std::cout << (int) index_[ pos ] << ", ";
        //    index_.push_back( static_cast<size_type>( index_[ pos ] - 1 ) );
        //    index_.push_back( static_cast<size_type>( index_[ pos ] + 1 ) );
        //    ++pos;
        //}


        //std::transform( index_.begin(), index_.end(), index_.begin(), [ & ]( size_type val ) { return idx[ val ]; } );

        std::cout << "\n";
        std::for_each( index_.begin(), index_.end(), []( size_type val ) { std::cout << static_cast<int>( val ) << ", "; } );
        std::cout << "\n\n";
        //std::for_each( index_.begin(), index_.end(), [&]( size_type val ) { std::cout << static_cast<int>( vector_[val] ) << ", "; } );
        //std::cout << "\n\n";
    }

    const vector_type& at( size_t num )
    {
        return vector_[ index_[num] ];
    }

    const_iterator lower_bound( const vector_type& key ) const
    {
        size_t pos = 0;
        while( pos < index_.size() )
        {
            int retComp = comp( vector_[ index_[ pos ] ], key );
            if( 0 == retComp )
            {
                auto ret = vector_.cbegin();
                std::advance( ret, index_[ pos ] );
                return ret;
            }

            if( retComp < 0 )
            {
                pos += pos + 1;
            }
            else
            {
                pos += pos + 2;
            }
        }
        return vector_.cend();


        //for( size_type pos : index_ )
        //{
        //    if( 0 == comp( vector_[ pos ], key ) )
        //    {
        //        auto ret = vector_.cbegin();
        //        std::advance( ret, pos );
        //        return ret;
        //    }
        //}
        //return vector_.cend();

    }

    const_iterator find( const vector_type& key ) const
    {
        compare_type comp;
        size_t pos = 0;
        while( pos < index_.size() )
        {
            if( comp( key, vector_[ index_[ pos ] ] ) )
            {
                // key < vector_[ index_[ pos ] ]
                pos += pos + 1;
            }
            else if( comp( vector_[ index_[ pos ] ], key ) )
            {
                // vector_[ index_[ pos ] ] < key
                pos += pos + 2;
            }
            else
            {
                // vector_[ index_[ pos ] ] == key
                auto ret = vector_.cbegin();
                std::advance( ret, index_[ pos ] );
                return ret;
            }
        }
        return vector_.cend();
        //auto pos = lower_bound( key );
        //if( vector_.cend() != pos &&
        //    key == *pos )
        //{
        //    return pos;
        //}
        //return vector_.cend();
    }

private:
    const std::vector< vector_type >& vector_;
    std::vector< size_type > index_;

    void sort_index( const std::vector< size_type >& idx, size_t first, size_t last )
    {
        size_t size = last - first;
        if( size < 2 )
            return;

        size_t middle = size/2;
        index_.push_back( idx[ (middle - first)/2 ] );
        index_.push_back( idx[ (last - middle)/2 ] );

        sort_index( idx, first, middle );
        sort_index( idx, middle, last );
    }

    const_iterator lower_bound(  );
};

} // namespace vecidx

#endif // VECIDX_SEARCH_INDEX_H
