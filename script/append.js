const fs = require('fs')
const path = require('path')

function writeUIntLE (buf, type, num) {
  if (type === 1) {
    return buf.writeUInt32LE(num)
  } else if (type === 2) {
    return buf.writeBigUInt64LE(BigInt(num))
  } else {
    throw new Error(`Unknown type: ${type}`)
  }
}

function writeStream (from, targetWs) {
  return new Promise((resolve, reject) => {
    try {
      fs.createReadStream(from, { emitClose: false, autoClose: false })
        .on('error', reject)
        .on('end', () => { resolve() })
        .pipe(
          targetWs
            .on('error', reject)
        )
    } catch (err) {
      reject(err)
    }
  })
}

async function append (
  base, type, patch,
  notice = 'https://github.com/toyobayashi/JMSUpdater/',
  target = path.join(path.dirname(patch), path.parse(patch).name + '.exe')
) {
  const noticeBuffer = Buffer.from(notice, 'ascii')

  const patchSize = fs.statSync(patch).size
  const patchSizeBuffer = Buffer.alloc(type * 4)
  writeUIntLE(patchSizeBuffer, type, patchSize)
  const noticeSizeBuffer = Buffer.alloc(type * 4)
  writeUIntLE(noticeSizeBuffer, type, noticeBuffer.length)

  fs.copyFileSync(base, target)

  await new Promise((resolve, reject) => {
    fs.createReadStream(patch)
      .on('error', reject)
      .pipe(
        fs.createWriteStream(target, { flags: 'a', encoding: 'binary' })
          .on('close', () => resolve())
          .on('error', reject)
      )
  })

  fs.appendFileSync(target, Buffer.concat([
    noticeBuffer,
    patchSizeBuffer,
    noticeSizeBuffer,
    Buffer.from(type === 2 ? [0xF3, 0xFB, 0xF7, 0xF2, 0x00, 0x00, 0x00, 0x00] : [0xF3, 0xFB, 0xF7, 0xF2]) // 4076338163
  ]))
}

function main () {
  const base = process.argv[2]
  const type = Number(process.argv[3]) || 1
  const patch = process.argv[4]

  append(base, type, patch).then(() => {
    console.log('Done.')
  }).catch(err => {
    console.error(err)
    process.exit(1)
  })
}

main()
