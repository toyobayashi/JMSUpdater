const fs = require('fs')
const path = require('path')

// const iconv = require('iconv-lite')

function checkPatcher (fd, patcherSize) {
  const magicBuf1 = Buffer.alloc(4)
  fs.readSync(fd, magicBuf1, 0, 4, patcherSize - 4)
  const magicNumber = magicBuf1.readUInt32LE()
  if (magicNumber === 0xF2F7FBF3) {
    return 1
  } else {
    const magicBuf2 = Buffer.alloc(8)
    fs.readSync(fd, magicBuf2, 0, 8, patcherSize - 8)
    const magicNumber = magicBuf2.readBigUInt64LE()
    if (magicNumber === BigInt(0xF2F7FBF3)) {
      return 2
    } else {
      throw new Error('Invalid patcher')
    }
  }
}

function readUInt (buf, type) {
  if (type === 1) {
    return buf.readUInt32LE()
  } else if (type === 2) {
    return buf.readBigUInt64LE()
  } else {
    throw new Error(`Unknown type: ${type}`)
  }
}

async function split (patherPath, fd, type, patcherSize) {
  const noticeLengthBuf = Buffer.alloc(type * 4)
  fs.readSync(fd, noticeLengthBuf, 0, type * 4, patcherSize - type * 4 * 2)
  const noticeLength = readUInt(noticeLengthBuf, type)
  console.log('noticeLength: ', noticeLength)

  const patchLengthBuf = Buffer.alloc(type * 4)
  fs.readSync(fd, patchLengthBuf, 0, type * 4, patcherSize - type * 4 * 3)
  const patchLength = readUInt(patchLengthBuf, type)
  console.log('patchLength: ', patchLength)

  const noticeBuf = Buffer.alloc(Number(noticeLength))
  fs.readSync(fd, noticeBuf, 0, Number(noticeLength), patcherSize - type * 4 * 3 - Number(noticeLength))
  fs.writeFileSync(path.join(path.dirname(patherPath), path.parse(patherPath).name + '.txt'), noticeBuf)

  await writeStream(
    path.join(path.dirname(patherPath), path.parse(patherPath).name + '.patch'),
    fd,
    patcherSize - type * 4 * 3 - Number(noticeLength) - Number(patchLength),
    patcherSize - type * 4 * 3 - Number(noticeLength) - 1
  )

  await writeStream(
    path.join(path.dirname(patherPath), path.parse(patherPath).name + '.base.exe'),
    fd,
    0,
    patcherSize - type * 4 * 3 - Number(noticeLength) - Number(patchLength) - 1
  )
}

function writeStream (target, fd, start, end) {
  return new Promise((resolve, reject) => {
    try {
      fs.createReadStream('', {
        fd: fd,
        autoClose: false,
        start,
        end
      })
        .on('error', reject)
        .pipe(
          fs.createWriteStream(target)
            .on('close', () => resolve())
            .on('error', reject)
        )
    } catch (err) {
      reject(err)
    }
  })
}

function main () {
  const patherPath = process.argv[2]
  const patcherSize = fs.statSync(patherPath).size
  const patcher = fs.openSync(patherPath, 'r')

  const type = checkPatcher(patcher, patcherSize)

  split(patherPath, patcher, type, patcherSize).then(() => {
    fs.closeSync(patcher)
    console.log('Done.')
  }).catch(err => {
    console.error(err)
    process.exit(1)
  })
}

main()
