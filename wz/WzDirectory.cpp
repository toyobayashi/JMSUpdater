
#include <string>
#include "WzDirectory.hpp"

namespace wz {
  WzDirectory::WzDirectory(WzBinaryReader* r): blockSize(0), checksum(0), offset(0), reader(r) {}

  WzDirectory::~WzDirectory() {
    for (WzImage* img : this->images) {
      delete img;
    }
    for (WzDirectory* dir : this->subDirs) {
      delete dir;
    }
  }

  bool WzDirectory::parseDirectory() {
    if (this->reader->tell() != this->offset) {
      this->reader->seek(this->offset);
    }
    WzBinaryReader* reader = this->reader;
    int32_t entryCount = reader->readWzInt();
    if (entryCount < 0 || entryCount > 100000) {
      return false;
    }

    for (int32_t i = 0; i < entryCount; i++) {
      uint8_t type = reader->readUInt8();
      std::string fname = "";
      int32_t fsize;
      int32_t checksum;
      uint32_t offset;
      size_t rememberPos = 0;
      switch (type) {
        case 1: {
          /* const unknown =  */reader->readInt32LE();
          reader->readInt16LE();
          /* const offs =  */reader->readWzOffset();
          continue;
        }
        case 2: {
          int32_t stringOffset = reader->readInt32LE();
          rememberPos = reader->tell();
          reader->seek(reader->header.fstart + stringOffset);
          type = reader->readUInt8();
          fname = reader->readWzString();
          break;
        }
        case 3:
        case 4: {
          fname = reader->readWzString();
          rememberPos = reader->tell();
          break;
        }
        default: break;
      }
      reader->seek(rememberPos);
      fsize = reader->readWzInt();
      checksum = reader->readWzInt();
      offset = reader->readWzOffset();
      if (type == 3) {
        WzDirectory* subDir = new WzDirectory(reader);
        subDir->blockSize = fsize;
        subDir->checksum = checksum;
        subDir->offset = offset;
        // subDir->parent = this;
        this->subDirs.push_back(subDir);
      } else {
        // const img = new WzImage(fname, reader, checksum)
        WzImage* img = new WzImage();
        // img.blockSize = fsize;
        img->offset = offset;
        // img.parent = this;
        this->images.push_back(img);
      }
    }

    for (WzDirectory* subdir : this->subDirs) {
      if (!subdir->parseDirectory()) {
        return false;
      }
    }

    return true;
  }

  std::list<WzImage*> WzDirectory::getChildImages() const {
    std::list<WzImage*> imgFiles;
    for (WzImage* img : this->images) {
      imgFiles.push_back(img);
    }
    for (const WzDirectory* subDir : this->subDirs) {
      std::list<WzImage*> list = subDir->getChildImages();
      for (WzImage* img : list) {
        imgFiles.push_back(img);
      }
    }
    return imgFiles;
  }
}
