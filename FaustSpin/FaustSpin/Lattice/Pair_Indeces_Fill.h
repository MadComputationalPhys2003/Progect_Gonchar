//AI GENERATED! TODAY 24.10.26 I FEEL BAD, LATER I'm going to write it by myself
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace NeighborPairs {

    inline void fill(
        const std::vector<std::uint64_t>& indices,
        std::size_t neighborCount,
        std::vector<std::pair<std::size_t, std::size_t>>& unique_pairs,
        std::vector<std::size_t>& matrixIndexBySlot)
    {
        if (neighborCount == 0) {
            if (!indices.empty()) {
                throw std::invalid_argument(
                    "Nonempty neighbor list requires a positive neighbor count"
                );
            }

            unique_pairs.clear();
            matrixIndexBySlot.clear();
            return;
        }

        if (indices.size() % neighborCount != 0) {
            throw std::invalid_argument(
                "Neighbor list size must be divisible by neighbor count"
            );
        }

        const std::size_t ionCount =
            indices.size() / neighborCount;

        std::vector<std::pair<std::size_t, std::size_t>> pairs;
        pairs.reserve(indices.size());

        // ‘ормируем пары: меньший индекс всегда первый.
        for (std::size_t slot = 0; slot < indices.size(); ++slot) {
            const std::size_t i = slot / neighborCount;

            if (indices[slot] >= ionCount) {
                throw std::out_of_range(
                    "Neighbor index is out of range"
                );
            }

            const std::size_t j =
                static_cast<std::size_t>(indices[slot]);

            pairs.emplace_back(
                std::min(i, j),
                std::max(i, j)
            );
        }

        // —ортируем пары и удал€ем повторени€.
        std::sort(pairs.begin(), pairs.end());

        pairs.erase(
            std::unique(pairs.begin(), pairs.end()),
            pairs.end()
        );

        std::vector<std::size_t> mapping(indices.size());

        //  аждому слоту соседей назначаем номер уникальной пары.
        for (std::size_t slot = 0; slot < indices.size(); ++slot) {
            const std::size_t i = slot / neighborCount;

            const std::size_t j =
                static_cast<std::size_t>(indices[slot]);

            const std::pair<std::size_t, std::size_t> pair(
                std::min(i, j),
                std::max(i, j)
            );

            const auto position =
                std::lower_bound(
                    pairs.begin(),
                    pairs.end(),
                    pair
                );

            mapping[slot] =
                static_cast<std::size_t>(
                    position - pairs.begin()
                    );
        }

        // ќбновл€ем результаты после завершени€ всех проверок.
        unique_pairs.swap(pairs);
        matrixIndexBySlot.swap(mapping);
    }

}
