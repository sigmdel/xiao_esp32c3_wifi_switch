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
  <p><form action='toggle' method='get'><button class="button">Toggle</button></form></p>
  <div class="info">%INFO%</div>
  <script>
  if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function(e) {
    console.log("Events Connected");
    }, false);

    source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
    }, false);

    source.addEventListener('ledstate', function(e) {
    console.log("ledstate", e.data);
    document.getElementById("led").innerHTML = e.data;
    }, false);

    source.addEventListener('tempvalue', function(e) {
    console.log("tempvalue", e.data);
    document.getElementById("temp").innerHTML = e.data;
    }, false);

    source.addEventListener('humdvalue', function(e) {
    console.log("humdvalue", e.data);
    document.getElementById("humd").innerHTML = e.data;
    }, false);

    source.addEventListener('lightvalue', function(e) {
    console.log("ligthvalue", e.data);
    document.getElementById("light").innerHTML = e.data;
    }, false);

  }
  </script></body>
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
