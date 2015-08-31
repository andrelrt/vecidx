#ifndef VECIDX_VECTOR_INDEX_H
#define VECIDX_VECTOR_INDEX_H

#include <cstdint>
#include <functional>
#include <algorithm>

namespace vecidx {

template< typename Size_T,
          typename VecType_T,
          typename VecComp_T = std::less<VecType_T> >
class vector_index
{
public:
    typedef Size_T size_type;
    typedef VecType_T vector_type;
    typedef VecComp_T compare_type;
    typedef typename std::vector< vector_type >::const_iterator const_iterator;

    vector_index( const std::vector<VecType_T>& vec ) : vector_(vec) {}

    void build_index()
    {
        index_.resize( vector_.size(), 0 );
        size_type value = 0;
        std::generate( index_.begin(), index_.end(),
                       [&](){ return value++; });

        compare_type comp;
        std::sort( index_.begin(), index_.end(),
                   [=]( const size_type& lhs, const size_type& rhs )
                   {
                       return comp( vector_[lhs], vector_[rhs] );
                   });
    }

    const vector_type& at( size_t num )
    {
        return vector_[ index_[num] ];
    }

    const_iterator lower_bound( const vector_type& key ) const
    {
        compare_type comp;
        auto pos = std::lower_bound( index_.begin(), index_.end(), key,
                                     [&]( const size_type& lhs, const size_type& rhs )
                                     {
                                         return comp( vector_[lhs], vector_[rhs] );
                                     });
        if( index_.end() != pos )
        {
            auto ret = vector_.cbegin();
            std::advance( ret, static_cast<size_t>(*pos) );
            return ret;
        }
        return vector_.cend();
    }

    const_iterator find( const vector_type& key ) const
    {
        auto pos = lower_bound( key );
        if( vector_.cend() != pos &&
            key == *pos )
        {
            return pos;
        }
        return vector_.cend();
    }

private:
    const std::vector< vector_type >& vector_;
    std::vector< size_type > index_;
};

}

#endif // VECIDX_VECTOR_INDEX_H
