#include <Arduino.h>


/*

Poor selection of % as placeholder delimiter because is commonly used in CSS and JavaScript code. #1249
@ https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249
by aiotech_pub (2022-12-04)

A reply by Sveninndh
@ https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249#issuecomment-1342781407
    I ran into the same problem, it took me a few hours. When studying the source code, I noticed two things that are unfortunately not documented.
    1.) The maximum length of a placeholder is limited to 32 bytes.

    2.) If you use % within a string or HTML file,
    you must escape all occurrences of % - which are not enclose a placeholder - to %%.
    Example: 'Width: 50%;' needs to be changed to 'width: 50%%;'

*/

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
    .button{display: inline-block; background-color: blue; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 12em; height: 3rem; text-decoration: none; margin: 2px; cursor: pointer;}
    .state{font-size: 2rem; font-weight: bold;margin-top:28px; text-align:center;}
    .info{margin-top:48px;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <table>
  <tr><td>Temperature:</td><td><b><span id="temperatureID">%TEMPERATURE%</span></b> °C</td></tr>
  <tr><td>Humidity:</td><td><b><span id="humidityID">%HUMIDITY%</span></b> &percnt;</td></tr>
  <tr><td>Brightness:</td><td><b><span id="brightnessID">%BRIGHTNESS%</span></b></td></tr>
  </table>
  <div class="state"><span id="relayID">%RELAYSTATE%</span></div>
  <p><button class="button" onclick="toggleRelay()">Toggle</button></p>
  <p><form action='log' method='get'><button class="button">Console</button></form></p>

  <div class="info">%INFO%</div>
  <script>
  function toggleRelay() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/toggle", true);
    xhr.send();
  }
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

    source.addEventListener('relaystate', function(e) {
    console.log("relaystate", e.data);
    document.getElementById("relayID").innerHTML = e.data;
    }, false);

    source.addEventListener('tempvalue', function(e) {
    console.log("tempvalue", e.data);
    document.getElementById("temperatureID").innerHTML = e.data;
    }, false);

    source.addEventListener('humdvalue', function(e) {
    console.log("humdvalue", e.data);
    document.getElementById("humidityID").innerHTML = e.data;
    }, false);

    source.addEventListener('brightvalue', function(e) {
    console.log("ligthvalue", e.data);
    document.getElementById("brightnessID").innerHTML = e.data;
    }, false);

  }
  </script></body>
</html>)rawliteral";

//  NOTE: (2023-03-08) Changed from form action "/toggle" method "get" on toggle button click
//  to AJAX request to avoid reloading the index page

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
    .button{display: inline-block; background-color: blue; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 12em; height: 3rem; text-decoration: none; margin: 2px; cursor: pointer;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <p><b>404 Error</b></p>
  <p><form action='/' method='get'><button class="button">Return</button></form></p>
</body>
</html>)rawliteral";

const char html_console[] PROGMEM = R"rawliteral(
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
    input{padding:5px; font-size:1em; width: 98%%}
    textarea{resize:vertical;width:98%%;height:318px;padding:5px;overflow:auto;background:#1f1f1f;color:#65c115;}
    .button{display: inline-block; background-color: blue; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 12em; height: 3rem; text-decoration: none; margin: 2px; cursor: pointer;}
    .info{margin-top:48px;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <textarea readonly id='log' cols='340' wrap='off'>%LOG%</textarea>
  <p>
  <input id='cmd' placeholder='Enter command - NOT implemented yet' autofocus>
  </p>
  <p>
  <form action='.' method='get'><button class="button">Main Menu</button></form>
  </p>
  <div class="info">%INFO%</div>
  <script>
    if (!!window.EventSource) {
      var source2 = new EventSource('events');

      source2.addEventListener('open', function(e) {
      console.log("Events Connected to source2");
      }, false);

      source2.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected from source2");
      }
      }, false);

      source2.addEventListener('logvalue', function(e) {
      console.log("logvalue", e.data);
      ta = document.getElementById("log")
      ta.innerHTML += e.data + "\n";
      ta.scrollTop = ta.scrollHeight;
      }, false);
    }
 </script>
</body>
</html>)rawliteral";
