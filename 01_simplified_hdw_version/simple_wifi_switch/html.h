#include <Arduino.h>

const char html_index[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>%TITLE%</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <meta http-equiv="refresh" content="5;url=/">
  <link rel="icon" href="data:,">
  <style>
    html{font-family: Helvetica; display:inline-block; margin: 0px auto; text-align: center; background-color: white;}
    h1{color: #0F3376; padding: 2vh;}
    table{margin-left: auto; margin-right: auto; margin-top:20px;}
    td{font-size: 1.5rem; text-align: left; padding: 8px;}
    .state{font-size: 2rem; font-weight: bold;margin-top:28px;}
    .button{display: inline-block; background-color: blue; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 5em; height: 3em; text-decoration: none; margin: 2px; cursor: pointer;}
    .info{margin-top:48px;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <table>
  <tr><td>Temperature:</td><td><b>%TEMPERATURE%</b> °C</td></tr>
  <tr><td>Humidity:</td><td><b>%HUMIDITY%</b> &percnt;</td></tr>
  <tr><td>Brightness:</td><td><b>%LIGHT%</b></td></tr>
  </table>
  <div class="state">%LEDSTATUS%</div>
  <p><form action='toggle' method='get'><button class="button">Toggle</button></form></p>
  <div class="info">%INFO%</div>
</body>
</html>)rawliteral";

// Could use
//  <p><a href="/toggle"><button class="button">Toggle</button></a></p>
// instead of
//  <p><form action='/toggle' method='get'><button class="button">Toggle</button></form></p>


const char html_404[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>%DEVICENAME%</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <link rel="icon" href="data:,">
  <style>
    html{font-family: Helvetica; display:inline-block; margin: 0px auto; text-align: center; background-color: white;}
    h1{color: #0F3376; padding: 2vh;}
    p{font-size: 1.5rem;}
    .button{display: inline-block; background-color: green; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 5em; height: 3em; text-decoration: none; margin: 2px; cursor: pointer;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <p><b>404 Error</b></p>
  <p><form action='/' method='get'><button class="button">Return</button></form></p>
</body>
</html>)rawliteral";
