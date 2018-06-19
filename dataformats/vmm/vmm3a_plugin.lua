
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

function i64_ax(h,l)
 local o = {}; o.l = l; o.h = h; return o;
end -- +assign 64-bit v.as 2 regs

function i64u(x)
 return ( ( (bit.rshift(x,1) * 2) + bit.band(x,1) ) % (0xFFFFFFFF+1));
end -- keeps [1+0..0xFFFFFFFFF]


function i64_rshift(a,n)
 local o = {};
 if(n==0) then
   o.l=a.l; o.h=a.h;
 else
   if(n<32) then
     o.l= bit.rshift(a.l, n)+i64u( bit.lshift(a.h, (32-n))); o.h=bit.rshift(a.h, n);
   else
     o.l=bit.rshift(a.h, (n-32)); o.h=0;
   end
  end
  return o;
end

function i64_toInt(a)
  return (a.l + (a.h * (0xFFFFFFFF+1)));
end -- value=2^53 or even less, so better use a.l value

function i64_toString(a)
  local s1=string.format("%x",a.l);
  local s2=string.format("%x",a.h);
  -- reduced to 44 bit
  --local s3="00000000000";
  --s3=string.sub(s3,1,16-string.len(s1))..s1;
  --s3=string.sub(s3,1,8-string.len(s2))..s2..string.sub(s3,9);
  return "0x"..string.upper(s2)..string.upper(s1);
end

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
			local overflow = buffer(12,4):uint()
			srshdr:add(buffer(4,3),"Data Id: VMM3a Data")
			srshdr:add(buffer(7,1),"FEC ID: " .. fecid)
			srshdr:add(buffer(8,4),"UDP Timestamp: " .. time .. " (" .. (time - t0) .. ")")
			srshdr:add(buffer(12,4),"Offset overflow last frame: " .. overflow)



			if protolen >= 16 then
				local hits = (protolen-16)/data_length_byte
				pinfo.cols.info = string.format("FEC: %d, Hits: %3d", fecid, hits)
				local hit_id = 0
				local marker_id = 0
				for i=1,hits do
		 
					local d1 = buffer(16 + (i-1)*data_length_byte, 4)
					local d2 = buffer(20 + (i-1)*data_length_byte, 2)
					local d1rev = reversebits(d1)
					local d2rev = reversebits(d2)
					-- data marker
					local flag = bit.band(bit.rshift(d2:uint(), 15), 0x01) 
										
					if flag == 0 then
					-- marker
						--data 2 (16 bit):
						-- 	flag: 0: 1 bit
						-- 	vmmid: 1-5 : 5 bit
						--	timestamp: 6-15: 10 bit

						--data 1 (32 bit):
						--	timestamp: 0-31: 32 bit
	
						marker_id = marker_id + 1
						local timestamp1 = d1:uint() 
						local vmmid =  bit.band(bit.rshift(d2:uint(), 10), 0x1F) 
						local timestamp2 = bit.lshift(d2:uint(), 22) 

						local temp = i64_ax(timestamp1,timestamp2)
						local timestamp = i64_rshift(temp,22)

						local hit = srshdr:add(buffer(16 + (i-1)*data_length_byte, data_length_byte),
							string.format("Marker: %3d, SRS timestamp: %d, vmmid: %d",
							marker_id, i64_toInt(timestamp), vmmid))

						local d1handle = hit:add(d1, "Data1 " .. d1)
						d1handle:append_text(", (" .. d1rev .. ")")
						d1handle:add(d1, "timestamp: " .. i64_toString(timestamp))
						
						local d2handle = hit:add(d2, "Data2 " .. d2)
						d2handle:append_text(", (" .. d2rev .. ")")
						d2handle:add(d2, "flag: " .. flag)
						d2handle:add(d2, "vmmid: " .. vmmid)
						
					else
					-- hit
						hit_id = hit_id + 1
						--data 2 (16 bit):
						--	flag: 0
						-- 	overThreshold: 1
						-- 	chno: 2-7 : 6 bit
						-- 	tdc: 8-15: 8 bit

						--data 1 (32 bit):
						-- 	offset: 0-4: 5 bit
						-- 	vmmid: 5-9: 5 bit
						-- 	adc: 10-19: 10 bit
						-- 	bcid: 20-31: 12 bit
									
						
						local othr = bit.band(bit.rshift(d2:uint(), 14), 0x01) 
						local chno = bit.band(bit.rshift(tonumber(d2rev,16), 18), 0x3f) 
						local tdc  = bit.band(bit.rshift(tonumber(d2rev,16), 24), 0x3f) 

					
						local offset = bit.band(bit.rshift(d1:uint(), 27), 0x1f) 
						local vmmid = bit.band(bit.rshift(d1:uint(), 22), 0x1f) 
						local adc   = bit.band(bit.rshift(tonumber(d1rev,16), 10), 0x03FF) 
						local gbcid   = bit.band(bit.rshift(tonumber(d1rev,16), 20), 0x0FFF) 

						local bcid  = gray2bin32(gbcid)
					
					
					

						local hit = srshdr:add(buffer(16 + (i-1)*data_length_byte, data_length_byte),
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
