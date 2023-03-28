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

        logging = {
                level = domoticz.LOG_DEBUG, -- change LOG_ERROR to LOG_DEBUG to see log messages in Domoticz log
                marker = 'dusk',
        },

        execute = function(dz, device)
                local isOn = dz.devices(195).active
                local lux = device.lux
                local status = ' is off'
                if isOn then
                        status = ' is on'
                end

                dz.log('Device '..device.name..', lux = '..lux..' and '..dz.devices(195).name..status)
                if ((lux < dz.variables('dark_lo_threshold').value) and not isOn) then
                        dz.log('Set switch ON')
                        dz.devices(204).switchOn()
                elseif ((lux > dz.variables('dark_hi_threshold').value) and isOn) then
                        dz.log('Set switch OFF')
                        dz.devices(204).switchOff()
                else
                        dz.log('Do nothing')
                end
        end
}
