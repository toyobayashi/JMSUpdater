#include "BinaryReader.hpp"

#include <vector>
#include <sstream>
#include <utility>

namespace wz {

static void swap(char* b, int m, int n) {
  char i = *(b + n);
  *(b + n) = *(b + m);
  *(b + m) = i;
}

BinaryReader::BinaryReader(): size(0), f() {}

BinaryReader::~BinaryReader() {}

BinaryReader::BinaryReader(BinaryReader&& other) noexcept {
  this->f = std::move(other.f);
  this->size = other.size;
  other.size = 0;
}

BinaryReader& BinaryReader::operator=(BinaryReader&& other) noexcept {
  this->f = std::move(other.f);
  this->size = other.size;
  other.size = 0;
  return *this;
}

void BinaryReader::open(const std::string& path) {
  this->f.open(path, std::ios::binary | std::ios::in);
  this->f.exceptions(std::ios::failbit | std::ios::eofbit);
  this->f.seekg(0, this->f.end);
  this->size = (size_t) this->f.tellg();
  this->f.seekg(0, this->f.beg);
}

void BinaryReader::close() {
  this->f.close();
}

std::vector<uint8_t> BinaryReader::read(size_t len) {
  std::vector<uint8_t> buf(len);
  if (len > 0) {
    this->f.read(reinterpret_cast<char*>(&buf[0]), len);
  }
  return buf;
}

std::string BinaryReader::readString() {
  char c[2] = { 0 };
  std::ostringstream os;
  do {
    this->f.read(c, 1);
    os << c;
    if ((size_t)this->f.tellg() >= this->size) {
      break;
    }
  } while (c[0] != '\0');
  return os.str();
}

std::string BinaryReader::readString(size_t len) {
  if (len == 0) return "";
  std::vector<uint8_t> buf = this->read(len);
  return std::string(buf.begin(), buf.end());
}

bool BinaryReader::readBoolean() {
  char b;
  this->f.read(&b, 1);
  return b != '\0';
}

int8_t BinaryReader::readInt8() {
  char b;
  this->f.read(&b, 1);
  return b;
}

uint8_t BinaryReader::readUInt8() {
  char b;
  this->f.read(&b, 1);
  return (uint8_t) b;
}

int16_t BinaryReader::readInt16LE() {
  char b[2];
  this->f.read(b, 2);
  int16_t* ptr = (int16_t*)b;
  return *ptr;
}

uint16_t BinaryReader::readUInt16LE() {
  char b[2];
  this->f.read(b, 2);
  uint16_t* ptr = (uint16_t*)b;
  return *ptr;
}

int16_t BinaryReader::readInt16BE() {
  char b[2];
  this->f.read(b, 2);
  swap(b, 0, 1);
  int16_t* ptr = (int16_t*)b;
  return *ptr;
}

uint16_t BinaryReader::readUInt16BE() {
  char b[2];
  this->f.read(b, 2);
  swap(b, 0, 1);
  uint16_t* ptr = (uint16_t*)b;
  return *ptr;
}

int32_t BinaryReader::readInt32LE() {
  char b[4];
  this->f.read(b, 4);
  int32_t* ptr = (int32_t*)b;
  return *ptr;
}

uint32_t BinaryReader::readUInt32LE() {
  char b[4];
  this->f.read(b, 4);
  uint32_t* ptr = (uint32_t*)b;
  return *ptr;
}

int32_t BinaryReader::readInt32BE() {
  char b[4];
  this->f.read(b, 4);
  swap(b, 0, 3);
  swap(b, 1, 2);
  int32_t* ptr = (int32_t*)b;
  return *ptr;
}

uint32_t BinaryReader::readUInt32BE() {
  char b[4];
  this->f.read(b, 4);
  swap(b, 0, 3);
  swap(b, 1, 2);
  uint32_t* ptr = (uint32_t*)b;
  return *ptr;
}

int64_t BinaryReader::readBigInt64LE() {
  char b[8];
  this->f.read(b, 8);
  int64_t* ptr = (int64_t*)b;
  return *ptr;
}

uint64_t BinaryReader::readBigUInt64LE() {
  char b[8];
  this->f.read(b, 8);
  uint64_t* ptr = (uint64_t*)b;
  return *ptr;
}

int64_t BinaryReader::readBigInt64BE() {
  char b[8];
  this->f.read(b, 8);
  swap(b, 0, 7);
  swap(b, 1, 6);
  swap(b, 2, 5);
  swap(b, 3, 4);
  int64_t* ptr = (int64_t*)b;
  return *ptr;
}

uint64_t BinaryReader::readBigUInt64BE() {
  char b[8];
  this->f.read(b, 8);
  swap(b, 0, 7);
  swap(b, 1, 6);
  swap(b, 2, 5);
  swap(b, 3, 4);
  uint64_t* ptr = (uint64_t*)b;
  return *ptr;
}

float BinaryReader::readFloadLE() {
  char b[4];
  this->f.read(b, 4);
  float* ptr = (float*)b;
  return *ptr;
}

float BinaryReader::readFloadBE() {
  char b[4];
  this->f.read(b, 4);
  swap(b, 0, 3);
  swap(b, 1, 2);
  float* ptr = (float*)b;
  return *ptr;
}

double BinaryReader::readDoubleLE() {
  char b[8];
  this->f.read(b, 8);
  double* ptr = (double*)b;
  return *ptr;
}

double BinaryReader::readDoubleBE() {
  char b[8];
  this->f.read(b, 8);
  swap(b, 0, 7);
  swap(b, 1, 6);
  swap(b, 2, 5);
  swap(b, 3, 4);
  double* ptr = (double*)b;
  return *ptr;
}

BinaryReader& BinaryReader::seek(size_t pos) {
  this->f.seekg(pos, this->f.beg);
  return *this;
}

size_t BinaryReader::tell() {
  return (size_t) this->f.tellg();
}
void BinaryReader::clear() {
  this->f.clear();
}

}
