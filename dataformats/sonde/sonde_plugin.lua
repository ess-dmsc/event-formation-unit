
-- Copyright (C) 2016, 2017 European Spallation Source ERIC
-- Wireshark plugin for dissecting IDEAS readout data

-- helper variable and functions

function typetostring(type)
   if (type == 0xd6) then
      return "Time Triggered Data"
   else
      return "Unsupported"
   end
end


-- -----------------------------------------------------------------------------------------------
-- the protocol dissector
-- -----------------------------------------------------------------------------------------------
sonde_proto = Proto("ideassonde","IDEAS Protocol")

function sonde_proto.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "IDEAS"

  local protolen = buffer():len()
  local header = tree:add(sonde_proto,buffer(),"IDEAS Header")

  local versys = buffer(0,1):uint()
  header:add(buffer(0,1), string.format("%d%d.. .... = version %d", bit.band(versys, 0x80), bit.band(versys, 0x40), bit.rshift(versys, 6)))
  header:add(buffer(0,1), string.format("..xx xxxx = System  %d", bit.band(versys, 0x3f)))
  local type = buffer(1,1):uint()
  header:add(buffer(1,1), string.format("Packet Type: %s (0x%02x) ", typetostring(type), type))

  header:add(buffer(2,2), "Packet Sequence")

  local ts = buffer(4,4):uint()
  header:add(buffer(4,4), string.format("Timestamp: %d (0x%04x)", ts, ts))

  local datalen = buffer(8,2):uint()
  header:add(buffer(8,2), string.format("Data length: %d", datalen))

  local hits = ((protolen-10) - 1)/5
  pinfo.cols.info = string.format("Version: %d, system: %d, %s",
          bit.rshift(versys,6), bit.band(versys, 0x3f), typetostring(type))


  for i=1,hits do
    local ts =  buffer(11 + (i-1)*5, 4):uint()
    local asch = buffer(15 + (i-1)*5, 1):uint()
    local hit = header:add(buffer(11 + (i-1)*5, 5),
        string.format("Timestamp %d, ASIC %d, channel %d", ts, bit.rshift(asch, 6), bit.band(asch, 0x3f)))
  end
end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(50011, sonde_proto)
