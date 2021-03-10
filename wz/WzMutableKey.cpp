#include <cmath>

#include "WzMutableKey.hpp"

#define BATCH_SIZE 4096

namespace wz {

WzMutableKey::WzMutableKey(const std::vector<uint8_t>& aesUserKey): _aesUserKey(aesUserKey) {}
  
uint8_t WzMutableKey::at(size_t index) {
  size_t len = this->_keys.size();
  if (len == 0 || len <= index) {
    this->ensureKeySize(index + 1);
  }
  return this->_keys[index];
}

void WzMutableKey::ensureKeySize(size_t size) {
  size_t len = this->_keys.size();
  if (len != 0 && len >= size) {
    return;
  }

  size = (size_t) (::ceil((double) size / BATCH_SIZE) * BATCH_SIZE);
  this->_keys.resize(size, 0);
}

}
