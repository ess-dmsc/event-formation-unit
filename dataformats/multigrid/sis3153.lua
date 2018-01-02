
-- Copyright (C) 2017 European Spallation Source ERIC
-- Wireshark plugin for dissecting SIS3153 readout data

-- protocol commands and register addresses

cmds = { [0x20] = "VME Request",
         [0x22] = "VME ACK, Zero packet",
         [0x24] = "VME ACK, Last packet",
         [0x30] = "DMA Request",
         [0x32] = "DMA ACK, Zero packet",
         [0x34] = "DMA ACK, Last packet",
         [0x59] = "Heartbeat?",
         [0x60] = "Readout Data",
      }

-- helper variable and functions

function arr2str(arr, val)
  res = arr[val]
  if (res == nil)
  then
      res = "[Unknown]"
  end
  return res
end

function cmd2str(type)
  return arr2str(cmds, type)
end

function readorwrite(value)
   if bit.band(value, 0x4) then
      return " Write Request"
   else
      return " Read Request"
   end
end

-- -----------------------------------------------------------------------------------------------
-- the protocol dissectors
-- -----------------------------------------------------------------------------------------------

-- ------------------------------------------------------
-- DATA - UDP port 57344
-- ------------------------------------------------------

sis3153_data = Proto("sis3153","SIS3153 Control and Data")

function sis3153_data.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "SIS3153"

  local protolen = buffer():len()
  local header = tree:add(sis3153_data,buffer(),"SIS3153 Header")
  local cmd = buffer(0,1):uint()
  local cmdstr = cmd2str(cmd)
  header:add(buffer(1,1), string.format("Packet id %d", buffer(1,1):uint()))

  if cmd == 0x20 or cmd == 0x30 then
    local len= (buffer(2,2):le_uint() + 1) * 4
    header:add(buffer(2,2), string.format("Length %d bytes", len))
    header:add(buffer(4,8), "Protocol Header")
    local hdrcmd = buffer(6,1):uint()
    cmdstr = cmdstr .. readorwrite(hdrcmd)

  elseif cmd == 0x22 then
    header:add(buffer(2,2), string.format("Status 0x%04x", buffer(2,1):uint()))

  elseif cmd == 0x24 then
    header:add(buffer(2,2), string.format("Status 0x%04x", buffer(2,1):uint()))

  elseif cmd == 0x59 then
    local seqno = buffer(3,3):le_uint()
    header:add(buffer(2,2), string.format("SeqNo %d", seqno))

  elseif cmd == 0x56 or cmd == 0x60 then
    local timestamp = buffer(protolen - 12, 4):le_uint()
    local seqno = buffer(7,3):le_uint()
    header:add(buffer(7, 3), string.format("SeqNo %d", seqno))
    header:add(buffer(protolen - 12, 4), string.format("Time 0x%04x", timestamp))
  end

  pinfo.cols.info = string.format("%s", cmdstr)

end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(57344, sis3153_data)
