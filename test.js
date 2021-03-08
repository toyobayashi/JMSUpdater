// const fs = require('fs')

// function main (p) {
//   const len = fs.statSync(p).size
//   const fd = fs.openSync(p, 'r')
//   const buffer = Buffer.alloc(4)
//   fs.readSync(fd, buffer, 0, 4, len - 4)
//   const realLen = buffer.readUInt32LE()
//   const buffer2 = Buffer.alloc(realLen)
//   fs.readSync(fd, buffer2, 0, realLen, len - 4 - realLen)
//   fs.closeSync(fd)
//   fs.writeFileSync('base2.exe', buffer2)
// }

// function append (p) {
//   const base = fs.readFileSync('base')
//   const buf = Buffer.alloc(4)
//   buf.writeUInt32LE(base.length)
//   fs.appendFileSync(p, Buffer.concat([fs.readFileSync('base'), buf]))
// }

// append(process.argv[2])

const fs = require('fs')

const patch = '00393to00394.patch'
const notice = 'Notice'
const noticeBuffer = Buffer.from(notice.split('').map(c => c.charCodeAt(0)))

const patchSize = fs.statSync(patch).size
const patchSizeBuffer = Buffer.alloc(8)
patchSizeBuffer.writeBigUInt64LE(BigInt(patchSize))
const noticeSizeBuffer = Buffer.alloc(8)
noticeSizeBuffer.writeBigUInt64LE(BigInt(noticeBuffer.length))

fs.appendFileSync('Patcher.exe', Buffer.concat([
  fs.readFileSync(patch),
  noticeBuffer,
  patchSizeBuffer,
  noticeSizeBuffer,
  Buffer.from([0xF3, 0xFB, 0xF7, 0xF2, 0x00, 0x00, 0x00, 0x00]) // 4076338163
]))

// const iconv = require('iconv-lite')
// const patherPath = 'MaplePatch173to174.exe'
// const patcherSize = fs.statSync(patherPath).size
// const patcher = fs.openSync(patherPath, 'r')

// const magicBuf = Buffer.alloc(8)
// fs.readSync(patcher, magicBuf, 0, 8, patcherSize - 8)
// const magicNumber = magicBuf.readBigUInt64LE()
// console.log(magicNumber)

// const noticeLengthBuf = Buffer.alloc(8)
// fs.readSync(patcher, noticeLengthBuf, 0, 8, patcherSize - 16)
// const noticeLength = noticeLengthBuf.readBigUInt64LE()
// console.log('noticeLength: ', noticeLength)

// const patchLengthBuf = Buffer.alloc(8)
// fs.readSync(patcher, patchLengthBuf, 0, 8, patcherSize - 24)
// const patchLength = patchLengthBuf.readBigUInt64LE()
// console.log('patchLength: ', patchLength)

// const noticeBuf = Buffer.alloc(Number(noticeLength))
// fs.readSync(patcher, noticeBuf, 0, Number(noticeLength), patcherSize - 24 - Number(noticeLength))
// const notice = iconv.decode(noticeBuf, 'gbk')
// console.log('notice:\n', notice)

// const patchBuf = Buffer.alloc(Number(patchLength))
// fs.readSync(patcher, patchBuf, 0, Number(patchLength), patcherSize - 24 - Number(noticeLength) - Number(patchLength))
// fs.writeFileSync('MaplePatch173to174.patch', patchBuf)

// fs.createReadStream('', {
//   flags: 'r',
//   fd: patcher,
//   autoClose: false,
//   start: 0,
//   end: patcherSize - 24 - Number(noticeLength) - Number(patchLength) - 1
// }).pipe(fs.createWriteStream('Patcher.exe'))
