
-- Copyright (C) 2016, 2017 European Spallation Source ERIC
-- Wireshark plugin for dissecting VMM3/SRS readout data

-- helper variable and functions

--OLD FORMAT

--data 1 (32 bit):
-- 	adc: 0-7, 14-15: 10 bit
-- 	tdc: 8-13, 22-23: 8 bit
-- 	bcid: 16-21, 26-31: 12 bit
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

-- -----------------------------------------------------------------------------------------------
-- the protocol dissector
-- -----------------------------------------------------------------------------------------------
srsvmm_proto = Proto("srsvmm","SRSVMM Protocol")

function srsvmm_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "SRSVMM3a"
	local data_length_byte = 6
	local protolen = buffer():len()
	local srshdr = tree:add(srsvmm_proto,buffer(),"SRS Header")
	local fc = buffer(0,4):uint()

	if (fc0 == 0) and (fc ~= 0xfafafafa) then
		fc0 = fc
	end

	if fc == 0xfafafafa then
		srshdr:add("Frame Counter: 0xfafafafa (End of Frame)")
		pinfo.cols.info = "End of Frame"
	else
		local dataid = buffer(4,3):uint()
		local time = buffer(8,4):uint()
		if (t0 == 0) then
			t0 = time
		end

		srshdr:add(buffer(0,4),"Frame Counter: " .. fc .. " (" .. (fc-fc0) .. ")")
		if dataid == 0x564d32 then
			local fecid = buffer(7,1):uint()
			srshdr:add(buffer(4,3),"Data Id: VMM3a Data")
			srshdr:add(buffer(7,1),"FEC ID: " .. fecid)
			srshdr:add(buffer(8,4),"UDP Timestamp: " .. time .. " (" .. (time - t0) .. ")")



			if protolen >= 12 then
				local hits = (protolen-12)/data_length_byte
				pinfo.cols.info = string.format("FEC: %d, Hits: %3d", fecid, hits)
				local hit_id = 0
				local marker_id = 0
				for i=1,hits do
		 
					local d1 = buffer(12 + (i-1)*data_length_byte, 4)
					local d2 = buffer(16 + (i-1)*data_length_byte, 2)
					local d1rev = reversebits(d1)
					local d2rev = reversebits(d2)
					-- data marker
					local flag = bit.band(bit.rshift(d2:uint(), 15), 0x01) 
										
					if flag == 0 then
					-- marker
						--data 2 (16 bit):
						
						-- 	triggercounter: 1-10: 10 bit
						-- 	vmmid: 11-15 : 5 bit
						--	flag: 0: 1 bit

						--data 1 (32 bit):
						-- 	timestamp: 0-31: 32 bit
	
						marker_id = marker_id + 1
						local timestamp = d1:uint() 
						local triggercounter = bit.band(bit.rshift(d2:uint(), 5), 0x03FF) 
						local vmmid = bit.band(d2:uint(), 0x1F) 
						local hit = srshdr:add(buffer(12 + (i-1)*data_length_byte, data_length_byte),
							string.format("Marker: %3d, SRS timestamp: %d, triggercounter: %2d, vmmid: %d",
							marker_id, timestamp, triggercounter, vmmid))

						local d1handle = hit:add(d1, "Data1 " .. d1)
						d1handle:append_text(", (" .. d1rev .. ")")
						d1handle:add(d1, "timestamp: " .. timestamp)
						

						local d2handle = hit:add(d2, "Data2 " .. d2)
						d2handle:append_text(", (" .. d2rev .. ")")
						d2handle:add(d2, "flag: " .. flag)
						d2handle:add(d2, "triggercounter: " .. triggercounter)
						d2handle:add(d2, "vmmid: " .. vmmid)
						
					else
					-- hit
						hit_id = hit_id + 1
						--data 2 (16 bit):
						--	flag: 1
						-- 	overThreshold: 1
						-- 	chno: 2-7 : 6 bit
						-- 	tdc: 8-15: 8 bit

						--data 1 (32 bit):
						-- 	offset: 0-4: 5 bit
						-- 	vmmid: 5-9: 5 bit
						-- 	adc: 10-19: 10 bit
						-- 	bcid: 20-31: 12 bit
									
						
						local othr = bit.band(bit.rshift(d2:uint(), 14), 0x01) 
						-- local chno = shiftmask(d2rev, 24, 0x3f, 0, 0, 0)
						local chno =  bit.band(bit.rshift(tonumber(d2rev,16), 24), 0x3F) 
						
						--local tdc  = shiftmask(d2rev, 16, 0xff, 0, 0, 0)
						local tdc  = bit.band(bit.rshift(tonumber(d2rev,16), 16), 0xFF) 
					
						local offset = bit.band(bit.rshift(d1:uint(), 27), 0x1f) 
						local vmmid = bit.band(bit.rshift(d1:uint(), 22), 0x1f) 
						local adc   = bit.band(bit.rshift(tonumber(d1rev,16), 10), 0x03FF) 
						local gbcid   = bit.band(bit.rshift(tonumber(d1rev,16), 20), 0x0FFF) 

						local bcid  = gray2bin32(gbcid)
					
					
					

						local hit = srshdr:add(buffer(12 + (i-1)*data_length_byte, data_length_byte),
							string.format("Hit: %3d, offset: %d, vmmID: %2d, ch: %2d, bcid: %4d, tdc: %4d, adc: %4d, over thr: %d",
							hit_id, offset, vmmid, chno, bcid, tdc, adc, othr))

						local d1handle = hit:add(d1, "Data1 " .. d1)
						d1handle:append_text(", (" .. d1rev .. ")")
						d1handle:add(d1, "offset: " .. offset)
						d1handle:add(d1, "vmmid: " .. vmmid)
						d1handle:add(d1, "adc: " .. adc)
						d1handle:add(d1, "bcid(gray): " .. gbcid)
						d1handle:add(d1, "bcid: " .. bcid)
						
						

						local d2handle = hit:add(d2, "Data2 " .. d2)
						d2handle:append_text(", (" .. d2rev .. ")")
						d2handle:add(d2, "flag: " .. flag)
						d2handle:add(d2, "ovr thresh: " .. othr)
						d2handle:add(d2, "chno: " .. chno)
						d2handle:add(d2, "tdc: " .. tdc)
						
					end
				end
		  
			end
		elseif dataid == 0x564132 then
			srshdr:add(buffer(4,4),"Data Id: No Data")
			pinfo.cols.info = "No Data"
		else
			srshdr:add(buffer(4,4),"Data Id: Unknown data " .. buffer(5,3))
		end
	end
end

-- Register the protocol
udp_table = DissectorTable.get("udp.port")
udp_table:add(6006, srsvmm_proto)
