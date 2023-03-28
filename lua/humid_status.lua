--[[
Fix up humidity status of "Temp + Humidity" and "Temp + Humidity + Baro" type sensors
  Add their idx in the devices list below
  Tested with temp+humidity sensors with idx 206 and 208  and  temp+humidity+Baro sensor with idx 207

Change "units = 'C'"" to "units = 'F'"" if the °Fahrenheit are used to display temperature in the Domoticz
Web inteface. This assignement is the first line of execute = function(dz, item) below.
        User variable 'TemperatureUnit' must be 'C' or 'F' indicating unit used to display temperature values
Tested in Domoticz 2023.1, Build f9b9ac774, and dzVents Version: 3.1.8
]]

return
{
        on =
        {
                devices =
                {
                        206, 207, 208
                },
        },

        data = {
                updating = { initial = 0 }
        },

        logging = {
                level = domoticz.LOG_ERROR, -- change LOG_ERROR to LOG_DEBUG to see log messages in Domoticz log
               marker = 'fix_humid',
        },

        execute = function(dz, item)
                units = 'C'  -- set to 'F' if the display units set to °Faherenheit in Domoticz Settings
                dz.log('item '..item.name..', type: '..item.deviceType..', idx: '..item.id)
                if dz.data.updating == 0 then
                        dz.data.updating = 1
                        dz.log('updating item')
                        local temp = item.temperature
                        if units == 'F' then
                                dz.log('Converting '..temp..'°F')
                                temp = dz.utils.toCelsius(temp)
                                dz.log(' to '..temp..'°C')
                        end
                        if string.match(item.deviceType, 'Baro') then
                                dz.log('item.updateTempHumBaro')
                                item.updateTempHumBaro(temp, item.humidity, dz.HUM_COMPUTE, item.pressure, item.forecast)
                        else
                                dz.log('item.updateTempHum')
                                item.updateTempHum(temp, item.humidity, dz.HUM_COMPUTE)
                        end
                elseif dz.data.updating == 1 then
                        dz.log('not updating item')
                        dz.data.updating = 0
                end
        end
}
