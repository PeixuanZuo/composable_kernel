#ifndef DEVICE_REDUCE_COMMON_HPP
#define DEVICE_REDUCE_COMMON_HPP

#include <vector>

#include "common_header.hpp"
#include "reduction_enums.hpp"
#include "reduction_operator.hpp"

namespace ck {
namespace tensor_operation {
namespace device {

// template <typename preUnaryOpType, typename posUnaryOpType>
// using DeviceReducePtr = std::unique_ptr<DeviceReduce<preUnaryOpType, posUnaryOpType>>;

template <int Rank, typename ReduceDims>
std::pair<size_t, size_t> get_2d_lengths(const std::vector<int>& inLengths)
{
    static_assert(Rank <= 6, "bigger Rank size not supported!");

    size_t tensor_total_length = 1;
    size_t reduce_total_length = 1;

    static_for<0, ReduceDims::Size(), 1>{}(
        [&](auto i) { reduce_total_length *= inLengths[ReduceDims::At(i)]; });

    static_for<0, Rank, 1>{}([&](auto i) { tensor_total_length *= inLengths[i.value]; });

    return std::make_pair(tensor_total_length / reduce_total_length, reduce_total_length);
};

template <int x, typename Seq>
constexpr bool belong()
{
    bool inside = false;

    static_for<0, Seq::Size(), 1>{}([&](auto i) { inside = (inside || (x == Seq::At(i))); });

    return (inside);
};

template <int Rank, typename ReduceDims, int start = 0>
constexpr auto get_invariant_dims()
{
    static_assert(Rank <= 6, "bigger Rank size not supported!");

    if constexpr(start >= Rank)
        return Sequence<>{};
    else
    {
        if constexpr(!belong<start, ReduceDims>())
            return merge_sequences(Sequence<start>{},
                                   get_invariant_dims<Rank, ReduceDims, start + 1>());
        else
            return get_invariant_dims<Rank, ReduceDims, start + 1>();
    };
};

// helper functions using variadic template arguments
template <index_t... Ns>
static auto make_tuple_from_array_and_index_seq(const std::vector<int>& lengths, Sequence<Ns...>)
{
    return make_tuple(static_cast<index_t>(lengths[Ns])...);
};

template <index_t arraySize>
static auto make_tuple_from_array(const std::vector<int>& lengths, Number<arraySize>)
{
    static_assert(arraySize >= 1 && arraySize <= 6, "The tensor should have 1 to 6 dimensions");

    constexpr auto index_seq = typename arithmetic_sequence_gen<0, arraySize, 1>::type{};

    return make_tuple_from_array_and_index_seq(lengths, index_seq);
};

} // namespace device
} // namespace tensor_operation

} // namespace ck
#endif