# converts text files made from http://kvazars.com/littera/ to JSON

fs = require 'fs'

main = ->
  args = process.argv.slice(2)
  inputFilename = args.shift()
  outputFilename = args.shift()
  if not inputFilename or not outputFilename
    console.log "Syntax: fontjson.coffee [inputFilename] [outputFilename]"
    process.exit(0)

  lines = fs.readFileSync(inputFilename, "utf8").split(/\n/)
  glyphs = []
  for line in lines
    if matches = line.match(/^char (.+)/)
      rawKV = matches[1]
      glyph = {}
      for kv in rawKV.split(/\s+/)
        [key, value] = kv.split(/=/)
        value = parseInt(value)
        glyph[key] = value
      glyphs.push glyph

  fs.writeFileSync(outputFilename, JSON.stringify(glyphs, null, 2))
  console.log "Wrote '#{outputFilename}', #{glyphs.length} glyphs"

main()
