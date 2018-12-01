
-- Copyright (C) 2018 European Spallation Source ERIC
-- Wireshark plugin for dissecting JADAQ readout data

-- jadaq header from: https://github.com/ess-dmsc/jadaq/blob/devel/DataFormat.hpp


-- -----------------------------------------------------------------------------------------------
-- the protocol dissector
-- -----------------------------------------------------------------------------------------------
jadaq_proto = Proto("jadaq","JADAQ Protocol")

function jadaq_proto.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "JADAQ"
  local protolen = buffer():len()

  local data_length_byte = 8

  if protolen >= 32 then
    local run_lo   = buffer( 0, 4):le_uint()
    local run_hi   = buffer( 4, 4):le_uint()
    local time_lo   = buffer( 8, 4):le_uint()
    local time_hi   = buffer(12, 4):le_uint()
    local time64 = Int64.new(time_lo, time_hi)
    local digit = buffer(16, 4):le_uint()
    local elemid = buffer(20, 2):le_uint()
    local readouts = buffer(22, 2):le_uint()
    local vermaj = buffer(24, 1):le_uint()
    local vermin = buffer(25, 1):le_uint()
    local seqno = buffer(26, 4):le_uint()

    local jadaqhdr = tree:add(jadaq_proto,buffer(),
    string.format("JADAQ digitizer: %d, readouts: %d", digit, readouts))

    jadaqhdr:add(buffer( 0, 8),
    string.format("runid: %08x%08x", run_hi, run_lo))
    jadaqhdr:add(buffer( 8, 8),
    string.format("time: %08x%08x (%d)", time_hi, time_lo, (time64):tonumber()))
    jadaqhdr:add(buffer(16, 4), "digitizer " .. digit)
    jadaqhdr:add(buffer(20, 2), "element id " .. elemid)
    jadaqhdr:add(buffer(22, 2), "number of elements " .. readouts)
    jadaqhdr:add(buffer(24, 2), "version " .. vermaj .. "." .. vermin)
    jadaqhdr:add(buffer(26, 4), "sequence number " .. seqno )

    pinfo.cols.info = string.format("digitizer: %3d, readouts: %3d", digit, readouts)

    for i=1,readouts do
      local offset = 32 + (i - 1) * data_length_byte
      local time =    buffer(offset    , 4):le_uint()
      local channel = buffer(offset + 4, 2):le_uint()
      local adc =     buffer(offset + 6, 2):le_uint()
      jadaqhdr:add(buffer(offset, data_length_byte),
      string.format("%2d: time (ns) %10d, channel %3d, adc %5d", i, time * 16, channel, adc))
    end
  end
end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(9000, jadaq_proto)
