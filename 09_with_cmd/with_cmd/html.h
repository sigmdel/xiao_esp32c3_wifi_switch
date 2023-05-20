#include <Arduino.h>


/*  //NOTE:  Remove this long comment when this is documented

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

// NOTE: (2023-03-11) About the "Double NOT (!!)"
//  https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Logical_NOT#double_not_!!
//  !!window.EventSource is boolean same as Window.EventSource
//  I take it that means if the browser supports SSE

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
    .bred{background:#d43535;}
    .info{margin-top:48px;}
  </style>
</head>
<body>
  <h1>%DEVICENAME%</h1>
  <textarea readonly id='log' cols='340' wrap='off'>%LOG%</textarea>
  <p>
  <input id="cmd" type="text" value="" placeholder='Enter commands separated with ;'  autofocus/>
  </p>
  <p>
  <form action='.' method='get'><button class="button">Main Menu</button></form>
  </p>
  <p>
  <form action='.' method='get' onsubmit='return confirm("Confirm Restart");'><button name='rst' class='button bred'>Restart</button></form>
  </p>
  <div class="info">%INFO%</div>
  <script>
    cmdinput = document.getElementById("cmd")
    cmdinput.addEventListener("change", sendCmd);
	  function sendCmd() {
      // alert(document.getElementById("cmd").value);
      const xhr = new XMLHttpRequest();
      const url = "/cmd?cmd=" + cmdinput.value;
      xhr.open("GET", url);
      xhr.send();
      cmdinput.value = "";
	  }
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

/*
---- OK -- OK -- OK
    document.getElementById("cmd").addEventListener("change", sendCmd);
	  function sendCmd() {
      // alert(document.getElementById("cmd").value);
      const xhr = new XMLHttpRequest();
      const url = "/cmd?cmd=" + document.getElementById("cmd").value;
      xhr.open("GET", url);
      xhr.send();
	  }

----------------

	  function sendCmd() {
      // alert(document.getElementById("cmd").value);
      const xhr = new XMLHttpRequest();
      xhr.open("POST", "/", true); // true=asynchronous request (default)
      xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
      xhr.send("cmd="+document.getElementById("cmd").value);
	  }

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request){
    addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("POST / with %d params"), request->params());
    delay(5);
    if (request->params() > 0) {
      addToLogP(LOG_DEBUG, TAG_WEBSERVER, PSTR("Getting aParam"));
      delay(5);
      AsyncWebParameter* aParam = request->getParam(0, true, false);
      addToLogPf(LOG_DEBUG, TAG_WEBSERVER, PSTR("aParam: %s:%s"), aParam->name().c_str(), aParam->value().c_str());
      delay(10);
      //if ((aParam) && (aParam->name().equals("cmd")) && (aParam->value().length() > 0)) {
      //  addToLogPf(LOG_DEBUG, TAG_COMMAND, PSTR("Web console command: \"%s\""), aParam->value().c_str());
       // }
    }
    request->send(200, "text/plain", "OK");
  });

00:00:26.898 WEB/dbg: POST / with 1 params
00:00:26.902 WEB/dbg: Getting aParam
Guru Meditation Error: Core  0 panic'ed (Load access fault). Exception was unhandled.

Core  0 register dump:
MEPC    : 0x40058eb6  RA      : 0x4200975e  SP      : 0x3fcadad0  GP      : 0x3fc8f600
TP      : 0x3fc8ce2c  T0      : 0x4005890e  T1      : 0x40389f70  T2      : 0xffffffff
S0/FP   : 0x3fc99000  S1      : 0x3fc99000  A0      : 0x00000000  A1      : 0xffffffff
A2      : 0x00000001  A3      : 0x7f7f7f7f  A4      : 0x00000000  A5      : 0x00000004
A6      : 0xfa000000  A7      : 0x00000003  S2      : 0x3fcafc5c  S3      : 0x3fcafc5c
S4      : 0x3fca9864  S5      : 0x3fc99000  S6      : 0x3fcafcc4  S7      : 0x0000000a
S8      : 0x00000000  S9      : 0x00000000  S10     : 0x00000000  S11     : 0x00000000
T3      : 0x203a6874  T4      : 0x676e654c  T5      : 0x2d746e65  T6      : 0x746e6f00
MSTATUS : 0x00001881  MTVEC   : 0x40380001  MCAUSE  : 0x00000005  MTVAL   : 0x00000000
MHARTID : 0x00000000

Stack memory:
3fcadad0: 0x3fca9864 0x00000000 0x00000000 0x00000001 0x00000000 0x00000000 0x00000000 0x00000000
3fcadaf0: 0x203a6874 0x676e654c 0x2d746e65 0xb607750e 0x3fc99000 0x3fcafc5c 0x3fc99000 0x42001a8e
3fcadb10: 0x3c0c66c0 0x00000000 0x00000001 0x3fc99000 0x3fcb1320 0x3fcafc5c 0x3fcafc90 0x42010690
3fcadb30: 0x3fcb1320 0x3fcafc5c 0x3fcafc90 0xb607750e 0x3fcb1c4c 0x3c0c6000 0x3fca9850 0x4200d61c
3fcadb50: 0x00000000 0x00000000 0x03c998d0 0x00000000 0x00000000 0x00000000 0x3fcafc5c 0x00000000
3fcadb70: 0x00000000 0x00000000 0x03c99000 0x00000000 0x00000000 0x00000000 0x00cafc90 0xb607750e
3fcadb90: 0x00000000 0x00000000 0x00000001 0x00000009 0x3fcb1c4c 0x00000009 0x3fcafc5c 0x4200a7c2
3fcadbb0: 0x00000010 0x0000000c 0x00001800 0x403825de 0x00000000 0x00000000 0x00000000 0x00000000
3fcadbd0: 0x00000000 0x00000000 0x00000000 0xb607750e 0x00000002 0x00000005 0x00000000 0x00000004
3fcadbf0: 0x00000002 0x3fcaec30 0x3fcaec1c 0x00000001 0x00000000 0x3fcafee4 0x3fcaebc4 0x42006798
3fcadc10: 0x3fcafc5c 0x3fcaebc4 0x3fcb1bba 0x0000009b 0x3fc98d70 0x00000005 0x00000000 0x3fc99000
3fcadc30: 0x3fc98d70 0x00000001 0x3fcaff28 0x4200707a 0x00000000 0x3fcaff28 0x00000000 0x00000000
3fcadc50: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcadc70: 0x00000000 0x00000000 0x00000000 0x4038a602 0x00000000 0x00000000 0x00000000 0x00000000
3fcadc90: 0x00000000 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5 0xa5a5a5a5
3fcadcb0: 0xbaad5678 0x00000160 0xabba1234 0x00000154 0x3fcada80 0x0000686b 0x3fc953c4 0x3fc953c4
3fcadcd0: 0x3fcadcc0 0x3fc953bc 0x00000016 0x3fca9bf8 0x3fca9bf8 0x3fcadcc0 0x00000000 0x00000003
3fcadcf0: 0x3fca9cb0 0x6e797361 0x63745f63 0x0cd50070 0x003849b7 0x00000000 0x3fcadca0 0x00000003
3fcadd10: 0x00000000 0x00000000 0x00000000 0x00000000 0x3fc9a2d4 0x3fc9a33c 0x3fc9a3a4 0x00000000
3fcadd30: 0x00000000 0x00000001 0x00000000 0x00000000 0x00000000 0x4209b912 0x00000000 0x00000000
3fcadd50: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcadd70: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcadd90: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcaddb0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcaddd0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcaddf0: 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000 0x00000000
3fcade10: 0x63000000 0xbaad5678 0x0000002c 0xabba1234 0x00000020 0x20626557 0x76726573 0x73207265
3fcade30: 0x74726174 0x00006465 0x00000000 0x00000000 0x00000000 0xbaad5678 0x0000002c 0xabba1234
3fcade50: 0x00000020 0x74696e49 0x696c6169 0x676e697a 0x44454c20 0x4f2f4920 0x6e697020 0x0000002e
3fcade70: 0x00000000 0xbaad5678 0x0000004c 0xabba1234 0x00000040 0x74696e49 0x696c6169 0x676e697a
3fcade90: 0x54484420 0x74203032 0x65706d65 0x75746172 0x61206572 0x6820646e 0x64696d75 0x20797469
3fcadeb0: 0x736e6573 0x0000726f 0x00000000 0x00000000 0x00000000 0xbaad5678 0x00000014 0xabba1234

michel@hp:~$ curl -d "cmd=hello" "http://192.168.1.135"
curl: (56) Recv failure: Connexion ré-initialisée par le correspondant

gives the same crash

*/
