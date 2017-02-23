-- trivial protocol example

-- declare our protocol
srsvmm_proto = Proto("srsvmm","SRSVMM Protocol")

local t0
local fc0

function srsvmm_proto.init()
  t0 = 0
  fc0 = 0
end

function reverse(value)
  local i = value:uint()
  local nibswap = {0, 8, 4, 0xc, 2, 0xa, 6, 0xe, 1, 9, 5, 0xd, 3, 0xb, 7, 0xf}
  local a =          nibswap[bit.rshift(bit.band(i, bit.lshift(0xf, 28)), 28) + 1]
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf, 24)), 24) + 1], 28 - 24)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf, 20)), 20) + 1], 28 - 20)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf, 16)), 16) + 1], 28 - 16)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf, 12)), 12) + 1], 28 - 12)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf,  8)),  8) + 1], 28 -  8)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf,  4)),  4) + 1], 28 -  4)
  a = a + bit.lshift(nibswap[bit.rshift(bit.band(i, bit.lshift(0xf,  0)),  0) + 1], 28 -  0)
  return bit.tohex(a)
end


-- create a function to dissect it
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
        if  dataid == 0x564d32 then
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

                    local adc = bit.band(bit.rshift(tonumber(d1rev,16), 24), 0xff)
                    adc = adc + bit.lshift(bit.band(bit.rshift(tonumber(d1rev,16), 16), 0x3), 8)
                    local tdc = bit.band(bit.rshift(tonumber(d1rev,16), 18), 0x3f)
                    adc = tdc + bit.lshift(bit.band(bit.rshift(tonumber(d1rev,16), 8), 0x3), 6)
                    d1handle:append_text(", (" .. d1rev .. "), tdc: " .. string.format("%5d", tdc) .. ", adc: " .. adc)


                    local d2handle = srshdr:add(d2, "Data2 " .. d2)
                    local chno = bit.rshift(bit.band(tonumber(d2rev, 16), 0xfc), 2)
                    d2handle:append_text(", (" .. d2rev .. "), chno: " .. string.format("%4d", chno))
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
