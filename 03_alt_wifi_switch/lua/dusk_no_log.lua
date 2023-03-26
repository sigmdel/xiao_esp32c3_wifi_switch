--[[
 Night light with some hysteresis
   Device with idx 205 is the "lux" meter
   Device with idx 204 is the XIAO ESP32C3 Wi-Fi light switch
]]--
return
{
        on =
        {
                devices = {
                        205        -- lux meter
                }
        },

        execute = function(dz, device)
                local isOn = dz.devices(195).active
                local lux = device.lux

                if ((lux < dz.variables('dark_lo_threshold').value) and not isOn) then
                        dz.devices(204).switchOn()
                elseif ((lux > dz.variables('dark_hi_threshold').value) and isOn) then
                        dz.devices(204).switchOff()
                --else
                --        dz.log('Do nothing')
                end
        end
}
