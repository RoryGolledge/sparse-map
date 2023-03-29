#include <algorithm>
#include <limits>
#include <type_traits>
#include <unordered_map>

#include "chunk.h"

// When indexing into a `SparseMap`, positive and negative values can be used.
// Users of this class can specify which integer type to use for indexing.
//
// This allows restricting whether they want signed/unsigned types and how large
// an index can be (8 bits, 16 bits, 128 bits...)
template <typename IndexType>
concept ValidIndexType = 
    std::numeric_limits<IndexType>::is_integer;

template <typename T, std::size_t Dimensions, typename IndexType = int, std::size_t ChunkSize = 16>
requires
    ValidIndexType<IndexType>
class SparseMap {
public:
    template <typename ...Dims>
    requires
        CorrectDims<Dimensions, Dims...> &&
        AllAreType<IndexType, Dims...>
    auto operator[](const Dims... dims) -> T& {
        const auto chunk_index = std::array<IndexType, Dimensions>{{ align_to_chunk(dims)... }};

        return chunks_[chunk_index][align_inside_chunk(dims)...];
    }

private:
    static constexpr IndexType SignedChunkSize = static_cast<IndexType>(ChunkSize);

    auto align_to_chunk(IndexType index) -> IndexType {
        return index / SignedChunkSize;
    }

    auto align_inside_chunk(IndexType index) -> std::size_t {
        // Gets positive modulus => Safe to cast to std::size_t
        return static_cast<std::size_t>(
            (index % SignedChunkSize + SignedChunkSize) % SignedChunkSize
        );
    }

    struct ArrayHasher {
        // std::array hashing - Sourced from
        // https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
        std::size_t operator()(std::array<IndexType, Dimensions> arr) const noexcept {
            std::size_t seed = arr.size();
            for (const auto& i : arr) {
                seed ^= static_cast<typename std::make_unsigned<IndexType>::type>(i) +
                    0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };

    std::unordered_map<
        std::array<IndexType, Dimensions>,
        Chunk<T, Dimensions, ChunkSize>,
        ArrayHasher
    > chunks_;
};

