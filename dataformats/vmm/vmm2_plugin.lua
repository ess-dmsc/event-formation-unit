
-- Copyright (C) 2016, 2017 European Spallation Source ERIC
-- Wireshark plugin for dissecting VMM2/SRS readout data

-- helper variable and functions

local t0=0
local fc0=0

function reversebits(value)
  nibswap = {0, 8, 4, 0xc, 2, 0xa, 6, 0xe, 1, 9, 5, 0xd, 3, 0xb, 7, 0xf}
  shifts  = {28, 24, 20, 16, 12, 8, 4, 0}
  local sum = 0
  for j = 1, 8 do
    local shf = shifts[j]
    sum = sum + bit.lshift(nibswap[bit.rshift(bit.band(value:uint(), bit.lshift(0xf, shf)), shf) + 1], 28 - shf)
  end
  return bit.tohex(sum)
end

-- recombine data from one or two bit fields
function shiftmask(value, sh1, ma1, sh2, ma2, sh3)
  return bit.band(bit.rshift(tonumber(value,16), sh1), ma1) +
         bit.lshift(bit.band(bit.rshift(tonumber(value,16), sh2), ma2), sh3)
end

function gray2bin32(ival)
  ival = bit.bxor(ival, bit.rshift(ival, 16))
  ival = bit.bxor(ival, bit.rshift(ival,  8))
  ival = bit.bxor(ival, bit.rshift(ival,  4))
  ival = bit.bxor(ival, bit.rshift(ival,  2))
  ival = bit.bxor(ival, bit.rshift(ival,  1))
  return ival
end

-- the protocol dissector
srsvmm_proto = Proto("srsvmm","SRSVMM Protocol")

function srsvmm_proto.dissector(buffer,pinfo,tree)
  pinfo.cols.protocol = "SRSVMM2"

  local protolen = buffer():len()
  local srshdr = tree:add(srsvmm_proto,buffer(),"SRS Header")
  local fc = buffer(0,4):uint()

  if (fc0 == 0) and (fc ~= 0xfafafafa) then
     fc0 = fc
  end

  if fc == 0xfafafafa then
    srshdr:add("Frame Counter: 0xfafafafa (End of Frame)")
  else
    local dataid = buffer(4,3):uint()
    local time = buffer(8,4):uint()
    if (t0 == 0) then
      t0 = time
    end

    srshdr:add(buffer(0,4),"Frame Counter: " .. fc .. " (" .. (fc-fc0) .. ")")
    if dataid == 0x564d32 then
      srshdr:add(buffer(4,3),"Data Id: VMM2 Data")
      srshdr:add(buffer(7,1),"VMM2 ID: " .. buffer(7,1):uint())
      srshdr:add(buffer(8,4),"SRS Timestamp: " .. time .. " (" .. (time - t0) .. ")")

      if protolen > 12 then
        for i=1,(protolen-12)/8 do
          local d1 = buffer(12 + (i-1)*8, 4)
          local d2 = buffer(16 + (i-1)*8, 4)
          local d1rev = reversebits(d1)
          local d2rev = reversebits(d2)
          local hit = srshdr:add(buffer(12 + (i-1)*8, 8), "Hit " .. i)

          local d1handle = hit:add(d1, "Data1 " .. d1)

          local adc   = shiftmask(d1rev, 24, 0xff, 16, 0x03, 8)
          local tdc   = shiftmask(d1rev, 18, 0x3f,  8, 0x03, 6)
          local gbcid = shiftmask(d1rev, 10, 0x3f,  0, 0x3f, 6)
          local bcid  = gray2bin32(gbcid)

          d1handle:append_text(", (" .. d1rev .. ")")
          d1handle:add(d1, "bcid(gray): " .. gbcid)
          d1handle:add(d1, "bcid: " .. bcid)
          d1handle:add(d1, "tdc: " .. tdc)
          d1handle:add(d1, "adc: " .. adc)

          local d2handle = hit:add(d2, "Data2 " .. d2)
          local chno = shiftmask(d2rev, 2, 0x3f, 0, 0, 0)
          local flag = shiftmask(d2rev, 0, 0x01, 0, 0, 0)
          local othr = shiftmask(d2rev, 1, 0x01, 0, 0, 0)

          d2handle:append_text(", (" .. d2rev .. ")")
          d2handle:add(d2, "chno: " .. chno)
          d2handle:add(d2, "flag: " .. flag)
          d2handle:add(d2, "ovr thresh: " .. othr)
        end
      end
    elseif dataid == 0x564132 then
      srshdr:add(buffer(4,4),"Data Id: No Data")
    else
      srshdr:add(buffer(4,4),"Data Id: Unknown data " .. buffer(5,3))
    end
  end
end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(6006, srsvmm_proto)
