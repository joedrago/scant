async      = require 'async'
fs         = require 'fs'
path       = require 'path'
util       = require 'util'
pngReader  = require 'png-js'
pngWriter  = require('pngjs').PNG

gameDir = process.argv.slice(2).shift()
dataDir = path.join(gameDir, "data")

# TODO: choose a size based on the incoming sprites
tilesheetWidth = 2048
tilesheetHeight = 2048

pngBasenameRegex = /([^\\\/]+)\.png$/
trailingNumberRegex = /(.*[^\d])(\d+)$/

readpng = (filename, cb) ->
  basename = pngBasenameRegex.exec(filename)[1]
  png = pngReader.load filename
  png.decode (pixels) ->
    return cb(null, { name: basename, filename: filename, png: png, pixels: pixels })

generatePaddedTilesheet = (tilesheetName, tilePadding, genMetrics, cb) ->
  artDir = path.join(gameDir, tilesheetName)
  metricsFilename = path.join(dataDir, "#{tilesheetName}.json")
  pngFilename = path.join(dataDir, "#{tilesheetName}.png")
  tiles = fs.readdirSync artDir
  tiles.sort()
  filenames = (path.join(artDir, t) for t in tiles)
  async.map filenames, readpng, (err, results) ->
    png = new pngWriter {
      width: tilesheetWidth
      height: tilesheetHeight
      filterType: -1
    }

    x = tilePadding
    y = tilePadding
    maxY = 0
    for r in results
      if (x + r.png.width) > png.width
        x = tilePadding
        y += maxY + (tilePadding * 2)
        maxY = 0
      if maxY < r.png.height
        maxY = r.png.height

      r.x = x
      r.y = y

      # Pad top and bottom
      srcPixels = r.pixels
      for j in [0...tilePadding]
        for i in [0...r.png.width]
          # top
          srcIndex = 4 * i
          dstIndex = 4 * ((x+i) + ((y+j-tilePadding) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[srcIndex]
          png.data[dstIndex+1] = srcPixels[srcIndex+1]
          png.data[dstIndex+2] = srcPixels[srcIndex+2]
          png.data[dstIndex+3] = srcPixels[srcIndex+3]

          # bottom
          srcIndex = 4 * (i + ((r.png.height-1) * r.png.width))
          dstIndex = 4 * ((x+i) + ((y+j+r.png.height) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[srcIndex]
          png.data[dstIndex+1] = srcPixels[srcIndex+1]
          png.data[dstIndex+2] = srcPixels[srcIndex+2]
          png.data[dstIndex+3] = srcPixels[srcIndex+3]

      # Pad left and right
      srcPixels = r.pixels
      for j in [0...r.png.height]
        for i in [0...tilePadding]
          # left
          srcIndex = 4 * (j * r.png.width)
          dstIndex = 4 * ((x+i-tilePadding) + ((y+j) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[srcIndex]
          png.data[dstIndex+1] = srcPixels[srcIndex+1]
          png.data[dstIndex+2] = srcPixels[srcIndex+2]
          png.data[dstIndex+3] = srcPixels[srcIndex+3]

          # right
          srcIndex = 4 * (r.png.width-1 + (j * r.png.width))
          dstIndex = 4 * ((x+i+r.png.width) + ((y+j) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[srcIndex]
          png.data[dstIndex+1] = srcPixels[srcIndex+1]
          png.data[dstIndex+2] = srcPixels[srcIndex+2]
          png.data[dstIndex+3] = srcPixels[srcIndex+3]

      # Pad corners
      tlIndex = 0
      trIndex = 4 * (r.png.width - 1)
      blIndex = 4 * (r.png.width * (r.png.height-1))
      brIndex = 4 * ((r.png.width * r.png.height)-1)
      for j in [0...tilePadding]
        for i in [0...tilePadding]
          dstIndex = 4 * ((x+i-tilePadding) + ((y+j-tilePadding) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[tlIndex]
          png.data[dstIndex+1] = srcPixels[tlIndex+1]
          png.data[dstIndex+2] = srcPixels[tlIndex+2]
          png.data[dstIndex+3] = srcPixels[tlIndex+3]
          dstIndex = 4 * ((x+i+r.png.width) + ((y+j-tilePadding) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[trIndex]
          png.data[dstIndex+1] = srcPixels[trIndex+1]
          png.data[dstIndex+2] = srcPixels[trIndex+2]
          png.data[dstIndex+3] = srcPixels[trIndex+3]
          dstIndex = 4 * ((x+i-tilePadding) + ((y+j+r.png.height) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[blIndex]
          png.data[dstIndex+1] = srcPixels[blIndex+1]
          png.data[dstIndex+2] = srcPixels[blIndex+2]
          png.data[dstIndex+3] = srcPixels[blIndex+3]
          dstIndex = 4 * ((x+i+r.png.width) + ((y+j+r.png.height) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[brIndex]
          png.data[dstIndex+1] = srcPixels[brIndex+1]
          png.data[dstIndex+2] = srcPixels[brIndex+2]
          png.data[dstIndex+3] = srcPixels[brIndex+3]

      # Copy the actual tile itself
      srcIndex = 0
      srcPixels = r.pixels
      for j in [0...r.png.height]
        for i in [0...r.png.width]
          dstIndex = 4 * ((x+i) + ((y+j) * tilesheetWidth))
          png.data[dstIndex]   = srcPixels[srcIndex]
          png.data[dstIndex+1] = srcPixels[srcIndex+1]
          png.data[dstIndex+2] = srcPixels[srcIndex+2]
          png.data[dstIndex+3] = srcPixels[srcIndex+3]
          srcIndex += 4
      x += r.png.width + (tilePadding * 2)


    if genMetrics
      # generate metrics
      metrics = []
      for r in results
        metrics.push {
          name: r.name
          x: r.x
          y: r.y
          w: r.png.width
          h: r.png.height
        }

      fs.writeFileSync(metricsFilename, JSON.stringify(metrics, null, 2))
      util.log "Generated #{metricsFilename}"

    # write tilesheet png
    png.pack().pipe(fs.createWriteStream(pngFilename))
      .on 'finish', ->
        util.log "Generated #{pngFilename} (#{results.length} tiles)"
        cb() if cb # clue in the caller of generateTilesheet that we're done


main = ->
  generatePaddedTilesheet("csart", 2, true)
  generatePaddedTilesheet("art", 2, true)

main()
