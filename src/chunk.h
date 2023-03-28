#include <array>
#include <cmath>
#include <ranges>
#include <algorithm>

namespace detail {
    auto constexpr calc_chunk_size(
        const std::size_t dimensions,
        const std::size_t dim_size
    ) -> std::size_t {
        auto out = std::size_t{1};
        for (auto _: std::ranges::iota_view(size_t{0}, dimensions)) {
            out *= dim_size;
        }

        return out;
    }
}

template <std::size_t Expected, typename ...Dims>
concept CorrectDims = (sizeof...(Dims) == Expected);

template <typename Expected, typename ...Types>
concept AllAreType = (... && std::is_same_v<Expected, Types>);

template <typename T, std::size_t Dimensions, std::size_t DimensionSize>
class Chunk {
public:
    template <typename ...Dims>
    requires
        CorrectDims<Dimensions, Dims...> &&
        AllAreType<std::size_t, Dims...>
    auto operator[](const Dims... dims) -> T& {
        return data_[calc_index(dims...)];
    }

private:
    static auto calc_index() -> std::size_t {
        return 0;
    }
    
    template <typename ...Rest>
    static auto calc_index(const std::size_t next, const Rest... rest) -> std::size_t {
        return next + DimensionSize * calc_index(rest...);
    }

    std::array<T, detail::calc_chunk_size(Dimensions, DimensionSize)> data_;
};

