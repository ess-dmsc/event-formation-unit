
-- declare our protocol
srsvmm_proto = Proto("srsvmm","SRSVMM Protocol")

nibswap = {0, 8, 4, 0xc, 2, 0xa, 6, 0xe, 1, 9, 5, 0xd, 3, 0xb, 7, 0xf}
shifts  = {28, 24, 20, 16, 12, 8, 4, 0}

local t0
local fc0

function srsvmm_proto.init()
  t0 = 0
  fc0 = 0
end

function reverse(value)
  local sum = 0
  for j = 1, 8 do
    local shf = shifts[j]
    sum = sum + bit.lshift(nibswap[bit.rshift(bit.band(value:uint(), bit.lshift(0xf, shf)), shf) + 1], 28 - shf)
  end
  return bit.tohex(sum)
end

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
          local d1rev = reverse(d1)
          local d2rev = reverse(d2)

          local d1handle = srshdr:add(d1, "Data1 " .. d1)

          local adc  = shiftmask(d1rev, 24, 0xff, 16, 0x03, 8)
          local tdc  = shiftmask(d1rev, 18, 0x3f,  8, 0x03, 6)
          local bcid = shiftmask(d1rev, 10, 0x3f,  0, 0x3f, 6)
          bcid = gray2bin32(bcid)

          d1handle:append_text(", (" .. d1rev .. "), tdc: " .. string.format("%5d", tdc) ..
                                                  ", adc: " .. string.format("%5d", adc) ..
                                                  ", bcid: " .. string.format("%5d", bcid))

          local d2handle = srshdr:add(d2, "Data2 " .. d2)
          local chno = shiftmask(d2rev, 2, 0x3f, 0, 0, 0)
          local flag = shiftmask(d2rev, 0, 0x01, 0, 0, 0)
          local othr = shiftmask(d2rev, 1, 0x01, 0, 0, 0)
          
          d2handle:append_text(", (" .. d2rev .. "), chno: " .. string.format("%4d", chno) ..
                                                  ", flag: " .. flag ..
                                                  ", over threshold: " .. othr)
        end
      end
    elseif dataid == 0x564132 then
      srshdr:add(buffer(4,4),"Data Id: No Data")
    else
      srshdr:add(buffer(4,4),"Data Id: Unknown data " .. buffer(5,3))
    end
  end
end

-- load the udp.port table
udp_table = DissectorTable.get("udp.port")

-- register our protocol to handle udp port 6006
udp_table:add(6006,srsvmm_proto)
