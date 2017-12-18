
-- Copyright (C) 2017 European Spallation Source ERIC
-- Wireshark plugin for dissecting IDEAS readout data

-- protocol commands and register addresses

cmd = { [0x10] = "Write SysReg",
        [0x11] = "Read SysReg",
        [0x12] = "SysReg Readback",
        [0xc0] = "ASIC Config",
        [0xc1] = "ASIC Config Readback",
        [0xd6] = "Time Triggered Data",
      }

addr = { [0x0000] = "Serial Number",
         [0x0001] = "Firmware Type",
         [0x0002] = "Firmware Version",
         [0x0010] = "System Number",
         [0x0C00] = "Calibration Execute",
         [0x0C01] = "Calibration Pulse Polarity",
         [0x0C02] = "Calibration Num Pulses",
         [0x0C03] = "Calibration Pulse Length",
         [0x0C04] = "Calibration Pulse Interval",
         [0x0E00] = "Cal DAC",
         [0xF016] = "cfg_phystrig_en",
         [0xF017] = "cfg_forced_en",
         [0xF018] = "cfg_event_num",
         [0xF019] = "cfg_forced_asic",
         [0xF01A] = "cfg_forced_channel",
         -- not in the documentation
         [0xF020] = "cfg_timing_readout_en",
         [0xF021] = "cfg_event_num",
         [0xF030] = "cfg_all_ch_en"
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
  return arr2str(cmd, type)
end

function addr2str(address)
  return arr2str(addr, address)
end


-- -----------------------------------------------------------------------------------------------
-- the protocol dissectors
-- -----------------------------------------------------------------------------------------------

-- ------------------------------------------------------
-- CONTROL - TCP port 50010
-- ------------------------------------------------------

sonde_ctrl = Proto("ideasctrl","IDEAS Control")

function sonde_ctrl.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "IDEAS CTL"
  local protolen = buffer():len()
  local header = tree:add(sonde_ctrl,buffer(),"IDEAS Header")

  datai = 0
  remain = protolen
  cmds = 0

  while remain > 0 do
    local versys = buffer( 0 + datai, 1):uint()
    local type   = buffer( 1 + datai, 1):uint()
    local len    = buffer( 8 + datai, 2):uint() + 10
    -- for read/write register commands
    local addr   = buffer(10 + datai, 2):uint()

    cmds = cmds + 1

    if (type == 0x10 or type == 0x11) then
      header:add(buffer(0 + datai, len), string.format("(0x%02x) %s <0x%04x %s>",
                 type, cmd2str(type), addr, addr2str(addr)))

    elseif (type == 0x12) then
       local regsize =  buffer(12 + datai, 1):uint()
       local val = buffer(13 + datai, regsize):uint()
       header:add(buffer(0 + datai,len), string.format("(0x%02x) %s <0x%04x %s> %d (U%d)",
             type, cmd2str(type), addr, addr2str(addr), val, regsize))

    elseif (type == 0xc0 or type == 0xc1) then
      local asic   = buffer(10 + datai, 1):uint()
      local cfglen = buffer(11 + datai, 2):uint()
      header:add(buffer(0 + datai,len), string.format("(0x%02x) %s - ASIC %d, bits %d",
                 type, cmd2str(type), asic, cfglen))
    else
      header:add(buffer(0 + datai,len), string.format("Undecoded command"))
    end

    datai = datai + len
    remain = remain - len
  end
  pinfo.cols.info = string.format("Commands: %d", cmds)
                -- bit.rshift(versys,6), bit.band(versys, 0x3f))
end


-- ------------------------------------------------------
-- DATA - UDP port 50011
-- ------------------------------------------------------


sonde_data = Proto("ideasdata","IDEAS Data")

function sonde_data.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "IDEAS DATA"

  local protolen = buffer():len()
  local header = tree:add(sonde_data,buffer(),"IDEAS Header")

  local versys = buffer(0,1):uint()
  header:add(buffer(0,1), string.format("%d%d.. .... = version %d", bit.band(versys, 0x80), bit.band(versys, 0x40), bit.rshift(versys, 6)))
  header:add(buffer(0,1), string.format("..xx xxxx = System  %d", bit.band(versys, 0x3f)))
  local type = buffer(1,1):uint()
  header:add(buffer(1,1), string.format("Packet Type: %s (0x%02x) ", cmd[type], type))

  header:add(buffer(2,2), "Packet Sequence")

  local ts = buffer(4,4):uint()
  header:add(buffer(4,4), string.format("Timestamp: %d (0x%04x)", ts, ts))

  local datalen = buffer(8,2):uint()
  header:add(buffer(8,2), string.format("Data length: %d", datalen))

  if type == 0xd6 then
    local hits = ((protolen-10) - 1)/5
    pinfo.cols.info = string.format("Version: %d, system: %d, %s",
            bit.rshift(versys,6), bit.band(versys, 0x3f), cmd[type])

    for i=1,hits do
      local ts =  buffer(11 + (i-1)*5, 4):uint()
      local asch = buffer(15 + (i-1)*5, 1):uint()
      local hit = header:add(buffer(11 + (i-1)*5, 5),
          string.format("Timestamp %d, ASIC %d, channel %d", ts, bit.rshift(asch, 6), bit.band(asch, 0x3f)))
    end
  elseif type == 0xd5 then
    local source = buffer(10,1):uint()
    header:add(buffer(10,1), string.format("source: %d", source))
    local trigger = buffer(11,1):uint()
    header:add(buffer(11,1), string.format("trigger: %d", trigger))
    local channel = buffer(12,1):uint()
    header:add(buffer(12,1), string.format("channel: %d", channel))
    local hold_delay = buffer(13,2):uint()
    header:add(buffer(13,2), string.format("hold delay: %d", hold_delay))
    local nsamples = buffer(15,2):uint()
    header:add(buffer(15,2), string.format("samples: %d", nsamples))
    pinfo.cols.info = string.format("Single EV pulse height")
  end
end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(50011, sonde_data)
udp_table:add(50021, sonde_data)

tcp_table = DissectorTable.get("tcp.port")
tcp_table:add(50010, sonde_ctrl)
tcp_table:add(50020, sonde_ctrl)
