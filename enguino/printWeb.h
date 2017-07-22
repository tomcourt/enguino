

void printHomePage() {
  print_P(F(
  "<!DOCTYPE html>\n"
  "<html>\n"
    "<head>\n"
      "<title>Enguino</title>\n"
      "<meta name='apple-mobile-web-app-capable' content='yes'>\n"      
      "<meta name='mobile-web-app-capable' content='yes'>\n"      
      "<style>\n"
        ".segment { stroke:gray; stroke-width:20; }\n"
        ".rectgauge  {  fill:none; stroke:black; stroke-width:40; }\n"
        ".roundgauge {  fill:none; stroke:black; }\n"
        ".label  { fill:dimgrey; text-anchor:middle; font-size:500px; }\n"
        ".value  { fill:black; text-anchor:middle; font-size:500px; }\n"
        ".number { fill:dimgrey; text-anchor:start; font-size:300px; alignment-baseline:central; }\n"
        ".mnumber { fill:dimgrey; text-anchor:middle; font-size:300px; alignment-baseline:central; }\n"
        ".unit   { fill:dimgrey; text-anchor:middle; font-size:300px; }\n"
        ".abutton { fill:lightgrey; stroke:black; stroke-width:40; }\n"
        ".indicator { fill:black }\n"
      "</style>\n"
      "<script type='text/javascript'>\n"
        "function ajax(u) {\n"
          "var xhttp = new XMLHttpRequest();\n"
          "xhttp.open('GET',u,true);\n"
          "xhttp.onreadystatechange = function() {\n"
            "if (this.readyState == 4 && this.status==200) {\n"
              "document.getElementById('dyn').innerHTML=this.responseText;\n"
             "}\n"
          "};\n"
          "xhttp.ontimeout = function() {\n"
            "xhttp.abort();\n"
            "document.getElementById('dyn').innerHTML='';"
           "};\n"
          "xhttp.timeout=900;\n"
          "xhttp.send();\n"
        "}\n"
        "setInterval(function(){ajax('d')}, 1000);\n"
      "</script>\n"
    "</head>\n"
    "<body>\n"
    "<svg viewBox='0 0 13330 10000' style='display:block; position:absolute; top:5%; left:5%; width:90%; height:90%;'>\n"
#ifdef BOUNDING_BOX
    "<rect x='1' y='1' width='13327' height='9998' fill='none' stroke='red'/>\n"
#endif
    "<defs>\n"
    "<g id='hmark'>\n"
    "<path d='M0 310 l-50 -50 v-520 l50 -50 l50 50 v520 Z' class='indicator'>\n"
    "</g>\n"
    "<g id='xmark'>\n"
    "<path d='M0 220 l-150 150 h300 Z ' class='indicator'>\n"
    "</g>\n"
    "<g id='vmark'>\n"
    "<path d='M310 0 l-50 -50 h-520 l-50 50 l50 50 h520 Z' class='indicator'>\n"
    "</g>\n"
    "</defs>\n"
    "<g id='dyn'></g>\n"
    "</svg>\n"
    "</body>\n"
  "</html>\n"
  ));

  // greying engine screen when connection fails can be done by replacing "document.getElementById('dyn').innerHTML='';" with
  // "document.getElementById('over').style.display='';\n"
  // and add "document.getElementById('over').style.display='none';\n" when dynamic content is loaded
  // and add the following to style and html respectively
  // ".overc { background-color:black; opacity:0.8; z-index:20; height:100%; width:100%; background-repeat:no-repeat; background-position:center; position:absolute; top:0px; left:0px; transition:2.0s }\n"        
  // "<div id='over' class='overc'></div>\n"
}


void printSetupPage() {
  print_P(F(   
    "<!DOCTYPE html>\n"
    "<html>\n"
      "<style>\n"
        "form {\n"
          "border: solid gray;\n"
          "border-radius: 1em;\n"
          "padding: 1em;\n"
          "position: absolute;\n"
          "top: 50%;\n"
          "left: 50%;\n"
          "margin-right: -50%;\n"
          "transform: translate(-50%, -50%);\n"
        "}\n"
        "input {\n"
          "margin-right: 1em;\n"
        "}\n"
      "</style>\n"
      "<form action='/'>\n"
        "<input type='radio' name='x' value='a' checked/>Add fuel &#8530;<br>\n"
        "<input type='radio' name='x' value='f'/>Set fuel capacity &#8530;<br>\n"
        "<input type='radio' name='x' value='h'/>Set hobbs &#8530;<br>\n"
        "<input type='radio' name='x' value='k'/>Set fuel flow k<br>\n"
        "<br>\n"
        "<input type='number' name='n' pattern='[0-9]*'/>\n"
        "<input type='submit'/><br>\n"
        "<br>\n"
        "<input type='button' value='Tanks Filled' onClick=\"javascript:location.assign('/?x=a&n=9999');\"/>\n"
        "<input type='button' value='Cancel' style='float:right;' onClick=\"javascript:location.assign('/');\"/\n"
      "</form>\n"
    "</html>\n"
  ));
}


void serveUpWebPage(char url, char var, word num) {
  switch(url) {  
    case '?':     // lean/cancel/ button pressed or return from setup
      switch (var) {
        case 'l':     // lean mode
          leanMode = !leanMode;
          if (leanMode)
            memset(peakEGT,0,sizeof(peakEGT));
          break;
        case 'a':   // add fuel
          ee_status.fuel += num<<2;
          if (ee_status.fuel > ee_settings.fullFuel)
            ee_status.fuel = ee_settings.fullFuel;
          goto eeStatus;
        case 'h':   // set hobbs
          ee_status.hobbs1k = 0;
          while (num >= 10000) {
            num -= 10000;
            (ee_status.hobbs1k)++;
          }
          ee_status.hobbs = num<<2; 
eeStatus:
          eeUpdateStatus();
          break;
        case 'f':   // set full fuel
          ee_settings.fullFuel = num<<2;
          goto eeSettings;
        case 'k':   // set k-factor
          ee_settings.kFactor = num;
eeSettings:
          eeUpdateSettings();
          break;
       }
       // fallthru to static page
    case ' ':     // static webpage
      printHomePage();
      break;
    case 's':      // setup page
     printSetupPage();
     break;
   case 'd':     // dynamic webpage   
      for (int i=0; i<N(gauges); i++)
        printGauge(gauges+i);
      break;
  }
}


void pollForHttpRequest() {
  // listen for incoming clients
  client = server.available();
  if (client) {
    // an http request ends with a blank line
    char lastToken = 0;
    const char *request = "GET /?x= &n=";
    char *state = request;
    char url = 0;
    char var = 0;
    word num = 0;
    while (client.connected()) {
      if (client.available()) {
        char token = client.read();
        if (token == '\r')
          continue;               // ignore '/r'
        // Serial.print(token);          
        // two newlines in a row is the end of the request
        if (token == '\n' && lastToken == '\n') {
          // send a standard http response header
          print_P(F(
            "HTTP/1.1 200 OK\n"
            "Content-Type: text/html\n"
            "Connection: close\n"     // the connection will be closed after completion of the response
            "\n"
          ));
          serveUpWebPage(url, var, num);
          flush();   
          break;
        }
        lastToken = token;
        
        if (state != 0) {
          switch (*state) {
            case '\0':
              if (token >= '0' && token <= '9') {
                num = num*10 + token - '0';
                continue;       // hold state
              }
              break;
            case '?':
              url = token;      // either a ? or the url
              break;
            case ' ':
              var = token;
              state++;          // advance state (falling thru would complete request which we don't want to do)
              continue;           
          }
          if (token == *state)
            state++;            // advance state
          else if (url)
            state = 0;          // completed getting the request because other than the templated query
          else
            state = request;    // reset looking for request if we haven't gotten to the '?'
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}


