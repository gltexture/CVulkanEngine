//
// Created by ded on 09.10.2026.
//

#pragma once
#include <algorithm>
#include <map>
#include <ranges>
#include <vector>

namespace cvulkan::collection {
    template<typename K, typename V>
    class IndexedMap {
    public:
        IndexedMap() = default;

        CLASS_NO_COPY_NO_MOVE(IndexedMap)

        void emplace(K key, V value) {
            auto it = this->_map.find(key);
            if (it == this->_map.end()) {
                this->_keys.push_back(key);
                this->_map.emplace(std::move(key), std::move(value));
            } else {
                it->second = std::move(value);
            }
        }

        [[nodiscard]] int32_t getIndexOf(const K& key) const {
            const auto it = std::find(this->_keys.begin(), this->_keys.end(), key);
            if (it == this->_keys.end()) {
                return -1;
            }
            return static_cast<int32_t>(std::distance(this->_keys.begin(), it));
        }

        [[nodiscard]] const V& getValueAtIndex(std::size_t i) const {
            return this->_map.at(this->_keys.at(i));
        }

        [[nodiscard]] const V& getValue(const K& key) {
            return _map.at(key);
        }

        [[nodiscard]] const std::map<K, V>& map() const {
            return _map;
        }

        [[nodiscard]] std::map<K, V>& map() {
            return _map;
        }

        [[nodiscard]] std::vector<std::reference_wrapper<const V>> values() const {
            std::vector<std::reference_wrapper<const V>> result;
            result.reserve(_map.size());
            for (const auto& value : _map | std::views::values) {
                result.emplace_back(std::cref(value));
            }
            return result;
        }

        [[nodiscard]] const std::vector<K>& keys() const {
            return _keys;
        }

    private:
        std::map<K, V> _map;
        std::vector<K> _keys;
    };
}
