const fs = require('fs')

function append (p) {
  const base = fs.readFileSync('base')
  const buf = Buffer.alloc(4)
  buf.writeUInt32LE(base.length)
  fs.appendFileSync(p, Buffer.concat([fs.readFileSync('base'), buf]))
}

setTimeout(() => {
  console.log('Append base')
  append(process.argv[2])
}, 200)
