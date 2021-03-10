#ifndef __WZ_MUTABLE_KEY_HPP_
#define __WZ_MUTABLE_KEY_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

namespace wz {
  class WzMutableKey {
  private:
    std::vector<uint8_t> _aesUserKey;
    std::vector<uint8_t> _keys;

  public:
    explicit WzMutableKey(const std::vector<uint8_t>& aesUserKey);
    uint8_t at(size_t index);
    void ensureKeySize(size_t size);
  };
}

#endif
