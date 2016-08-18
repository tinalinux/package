require('luabin')
aiosResPath = aiosconf and aiosconf.resPath or 'res'
luabin.path = aiosResPath .. '/aios.lub'

if not aiosconf then
    aiosconf = {
        appKey    = "146182097285958e",
        secretKey = "42794791306de0ac9fbf1edb90b0febc",
        provision = "test/res/aios-1.0.0-146182097285958e.provision",
        serial    = "test/res/aios-aihome.serial",
        userId    = "soundbox",
        ailog_level = 3,
    }
end

local musicnode = require 'aios.node.music'
musicnode.run()
