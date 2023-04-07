#include <Arduino.h>


/*

https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249
Poor selection of % as placeholder delimiter because is commonly used in CSS and JavaScript code. #1249
by aiotech_pub (2022-12-04)

In a reply
https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249#issuecomment-1342781407
Sveninndh

"
I ran into the same problem, it took me a few hours. When studying the source code, I noticed two things that are unfortunately not documented.
1.) The maximum length of a placeholder is limited to 32 bytes.

2.) If you use % within a string or HTML file,
you must escape all occurrences of % - which are not enclose a placeholder - to %%.
Example: 'Width: 50%;' needs to be changed to 'width: 50%%;'
"

This has been criticized by DeeEMM for not being

"
Escaping the % characters is little more than a hack as it generates non-compliant css...
any solution that creates another problem is not a solution at all ;)
"

Sveninndh suggested a different solution in a fork of the ESPAsyncWebServer

https://github.com/Sveninndh/ESPAsyncWebServer/commit/e7c9e3f0801bad234b25a6f04b76d9ceaa2a381a

which does not require escaping %% in the HTML code fed to the server. However

.../ESPAsyncWebServer/src/WebResponses.cpp

381:  size_t AsyncAbstractResponse::_fillBufferAndProcessTemplates(uint8_t* data, size_t len)
...
404:    } else { // double percent sign encountered, this is single percent sign escaped.
405:      // remove the 2nd percent sign
406:      memmove(pTemplateEnd, pTemplateEnd + 1, &data[len] - pTemplateEnd - 1);
407:        len += _readDataFromCacheOrContent(&data[len - 1], 1) - 1;
408:      ++pTemplateStart;
409:    }

So 'width: 50%%' will be changed to 'width: 50%' so compliant CSS code will be sent to any client.

Of course if '%%' (as a substitute for  U+2030 PER MILLE SIGN ‰) is used in the HTML file (instead )

Note that in .../ESPAsyncWebServer/src/WebResponseImpl.h

62:  #ifndef TEMPLATE_PLACEHOLDER
63:  #define TEMPLATE_PLACEHOLDER '%'
64:  #endif
65:
66:  #define TEMPLATE_PARAM_NAME_LENGTH 32

So, placeholder names must be 32 characters in length or less and it is possible to
use a different TEMPLATE_PLACEHOLDER in the platform build_flags such as

build_flags =
  "-D TEMPLATE_PLACEHOLDER='#'"

https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249#issuecomment-1372618455
by radozebra

That is not the best solution because # or any other ASCII character could easily appear in an
HTML file also.

https://github.com/me-no-dev/ESPAsyncWebServer/issues/1249#issuecomment-1342781407


  .button{display: inline-block; background-color: blue; border: none; border-radius: 6px; color: white; font-size: 1.5rem; width: 18em; height: 3em; text-decoration: none; margin: 2px; cursor: pointer;}

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
  <tr><td>Temperature:</td><td><b><span id="temp">%TEMPERATURE%</span></b> °C</td></tr>
  <tr><td>Humidity:</td><td><b><span id="humd">%HUMIDITY%</span></b> &percnt;</td></tr>
  <tr><td>Brightness:</td><td><b><span id="light">%LIGHT%</span></b></td></tr>
  </table>
  <div class="state"><span id="led">%LEDSTATUS%</span></div>
  <p><form action='toggle' method='get'><button class="button">Toggle</button></form></p>
  <p><form action='log' method='get'><button class="button">Console</button></form></p>

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
  <br/>
  <br/>
  <!-- form method='get' onsubmit='return l(1);' -->
  <input id='cmd' placeholder='Enter command - NOT implemented yet' autofocus>
  <!-- /form -->
  <br/>
  <br/>
  <form action='.' method='get'><button class="button">Main Menu</button></form>
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

      source.addEventListener('logvalue', function(e) {
      console.log("logvalue", e.data);
      ta = document.getElementById("log")
      ta.innerHTML += e.data + "\n";
      ta.scrollTop = ta.scrollHeight;
      }, false);
    }
 </script
</body>
</html>)rawliteral";

/*
      source.addEventListener('logvalue', function(e) {
      console.log("logvalue", e.data);
      document.getElementById("log").innerHTML += "\n" + e.data;
      document.getElementById("log").scrollTop = document.getElementById("log").scrollHeight;

*/
