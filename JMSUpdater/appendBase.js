const fs = require('fs')
const path = require('path')

function append (p) {
  const basePath = path.join(__dirname, 'patchers/PatcherOriginal.exe')
  console.log(`Append ${basePath}`)
  const base = fs.readFileSync(basePath)
  const buf = Buffer.alloc(4)
  buf.writeUInt32LE(base.length)
  fs.appendFileSync(p, Buffer.concat([base, buf]))
}

setTimeout(() => {
  append(process.argv[2])
}, 200)
