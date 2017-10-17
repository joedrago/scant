fs = require 'fs'

lines = fs.readFileSync("yoster.txt", "utf8").split(/\n/)
for line in lines
  if matches = line.match(/id=(\d+).+height=(\d+).+ yoffset=(\d+)/)
    id = parseInt(matches[1])
    height = parseInt(matches[2])
    yoffset = parseInt(matches[3])
    console.log "total #{height+yoffset} id #{id}"
