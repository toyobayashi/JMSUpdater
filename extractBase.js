const fs = require('fs')

function main (p) {
  const len = fs.statSync(p).size
  const fd = fs.openSync(p, 'r')
  const buffer = Buffer.alloc(4)
  fs.readSync(fd, buffer, 0, 4, len - 4)
  const realLen = buffer.readUInt32LE()
  const buffer2 = Buffer.alloc(realLen)
  fs.readSync(fd, buffer2, 0, realLen, len - 4 - realLen)
  fs.closeSync(fd)
  fs.writeFileSync('base2.exe', buffer2)
}

function append (p) {
  const base = fs.readFileSync('base')
  const buf = Buffer.alloc(4)
  buf.writeUInt32LE(base.length)
  fs.appendFileSync(p, Buffer.concat([fs.readFileSync('base'), buf]))
}

append(process.argv[2])
