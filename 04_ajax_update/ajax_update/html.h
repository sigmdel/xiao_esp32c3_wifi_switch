#include <Arduino.h>

const char html_index[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>%TITLE%</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
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
  <tr><td>Temperature:</td><td><b><span id="temp">%TEMPERATURE%</span></b> °C</td></tr>
  <tr><td>Humidity:</td><td><b><span id="humd">%HUMIDITY%</span></b> &percnt;</td></tr>
  <tr><td>Brightness:</td><td><b><span id="light">%LIGHT%</span></b></td></tr>
  </table>
  <div class="state"><span id="led">%LEDSTATUS%</span></div>
  <p><button class="button" onclick="toggleLed()">Toggle</button></p>
  <div class="info">%INFO%</div>
  <script>
    function makeRequest(uri) {
      console.log("makeRequest: GET", uri);
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          const resp = this.responseText.split(/\r?\n/);
          document.getElementById("led").innerHTML = resp[0];
          document.getElementById("temp").innerHTML = resp[1];
          document.getElementById("humd").innerHTML = resp[2];
          document.getElementById("light").innerHTML = resp[3];
        }
      };
      xhttp.open("GET", uri, true);
      xhttp.send();
    }
    function toggleLed() { makeRequest("/toggle"); };
    setInterval(function ( ) { makeRequest("/state"); }, 2000)
  </script>
</body>
</html>)rawliteral";


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
