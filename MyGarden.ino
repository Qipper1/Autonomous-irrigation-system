//using ESP8266 V2 NodeMCU v3


#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <ESPAsyncTCP.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads;

#define DHTPIN 14
#define DHTTYPE DHT11 

DHT dht(DHTPIN,DHTTYPE);


const char* ssid = "WiFi";
const char* password = "boss1234";

float t = 0.0;
float h = 0.0;
float a = 0.0;
float b = 0.0;

const float espVCC = 3.11;
unsigned long R1 = 7500;
unsigned long R2 = 47300;
const int analogPin = A0;
const int inputResolution =1023;
float v = 0.0 ;

const int relay1 = 12; // valve 1 
const int relay2 = 13; // valve 2 
const int relay3 = 15; // water pump  
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

unsigned long previousMillis = 0;    
const long interval = 1000;  // update readings DHT 

const char index_html[] PROGMEM = R"webpage(
<!DOCTYPE HTML>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <style>
    body {
      min-width: 310px;
      max-width: 1000px;
      height: 400px;
      margin: 0 auto;
    }
    h2 {
      font-family:Arial, Helvetica, sans-serif;
      font-size: 3rem;
      text-align:center;
    }
  </style>
</head>
  <h2> MyGarden </h2>
    <br>
 <body onload="startTime()" >
     <img src="data:image/png;base64,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" width="20" height="20" align="middle" style="padding-bottom: 12px;">
      <span style="font-size:1.3rem;"></span>
      <b><span id="time" style="font-family:Arial, Helvetica, sans-serif; font-size:1.3rem;"></span></b>
        <div id="time"></div>
<img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAACXBIWXMAAAsTAAALEwEAmpwYAAAA1ElEQVR4nO3bwQqCQBgA4TZ6Rh/Sl6y7QiIlX+nMUXEZhnUPPzpuGzyn+fnu/pinsbXGkXzqd/+uzv9RAC2gGVvv0Nm5/A4ogBbQFEALaAqgBTQF0AKaAmgBTQG0gKYAWkDzWF7QM76jWc4/Lr8DCqAFNKszYIu9M8TlmaKfX3L5HVAALaApgBbQFEALaAqgBTQF0AKa1fcBzQMuRgG0gKZ5wJ7FzkgBtICmAFpAUwAtoCmAFtAUQAtomgcokV+hAFpAUwAtoCmAFtD036AW0BRAC2heXrU4ekB4r6wAAAAASUVORK5CYII=" width="20" height="20" align="middle" style="padding-bottom: 12px;">
      <span style="font-size:1.3rem;"></span>
      <b><span id="date" style="font-family:Arial, Helvetica, sans-serif; font-size:1.3rem;"></span></b>
      <div id="date"></div>
      <img src="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAABgCAMAAADVRocKAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAzUExURQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAKMFRskAAAAQdFJOUwABEBwgJKOnq9TV4+vv8/5Ae276AAAACXBIWXMAAA7DAAAOwwHHb6hkAAABKUlEQVRoQ+3Y0WrCUBAE0LRJG63V9v+/1oQd3GvCCHdcoZQ5T+5cnH0oLNTBzMzMzBrj/P1b5PQ5orQxltWvzhNq04ynIjNq0xdeihxRm/BQBrUJeRnUJhILSBOJBaSJxALSRGIBaSKxgDSRWECaSCwgTSQWkCYSC0gTiQXRtLo72xFheEo0heZsR4Chy3n53js+r6IJ8mzHjKHLwwV5tmPG0OXhgmzcjB3+5IKfmOAN6d7anbAFE0S2uBu9oBHZYjN2UP7IXf75gpefiopjtxVN4fKB8DULTnP2ly/AkEgsIE0kFpAmEgtIE4kFpInEAtJEYgFpIrGANJFYQJoiroPahLwMatMRD0X2/4gf8FJk/1PCdMFTieZM30yHup9z2jNtZmZmZjAMVy4KZ8KNwtI2AAAAAElFTkSuQmCC" width="35" height="35" align="middle" style="padding-bottom: 12px;">
      <span style="font-size:1.3rem;"></span>
      <b><span id="voltage" style="font-family:Arial, Helvetica, sans-serif; font-size:1.3rem;"></span></b>
       <b><span style="font-family:Arial, Helvetica, sans-serif; font-size:1.3rem;">V</span></b>
        <div id="voltage"></div>
     <br>
     <br>
    <div id="chart-temperature" class="container"></div>
     <br>
      <br>
        <br>
          <br>
    <div id="chart-humidity" class="container"></div>
    <br>
      <br>
        <br>
          <br>
  <div id="chart-firstsensor" class="container"></div>
    <br>
      <br>
        <br>
          <br>
  <div id="chart-secondsensor" class="container"></div>  
</body>
<script> 
 function startTime(){
  var t = new Date();
  var h = t.getHours();
  var m = t.getMinutes();
  var s = t.getSeconds();
  m = checkTime(m);
  s = checkTime(s);
  document.getElementById('time').innerHTML = h + ":" + m + ":" + s;
  var t = setTimeout(startTime,500);
 }
function checkTime(i) {
  if (i < 10) {i = "0" + i};  // add zero in front of numbers < 10
  return i;
}
setInterval(function(){
  var d = new Date();
  var  dayNames = ["Sunday ", "Monday ", "Tuesday ", "Wednesday ", "Thursday ", "Friday ","Saturday "];
  var  monthNames = ["January", "February", "March", "April", "May", "June","July", "August", "September", "October", "November", "December"];
  document.getElementById("date").innerHTML = dayNames[d.getDay()] + ", " + d.getDate() + " " + monthNames[d.getMonth()] + " " + d.getFullYear();
},200);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("voltage").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/voltage", true);
  xhttp.send();
}, 200 ) ;

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { text: '<b>Temperature</b>' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: true }
    },
    series: { color: '#059e8a' }
  },
   xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: {text: '<b>Temperature (*C)</b>' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime() + 1000 * 60 * 60 * 3,
          y = parseFloat(this.responseText);
      if(chartT.series[0].data.length > 12) {
        chartT.series[0].addPoint([x, y], true, true, true);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 3000 ) ;

var chartH = new Highcharts.Chart({
  chart:{ renderTo:'chart-humidity' },
  title: { text: '<b>Air humidity </b>' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '<b>Humidity (%)</b>' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime() + 1000 * 60 * 60 * 3,
          y = parseFloat(this.responseText);
      if(chartH.series[0].data.length > 12) {
        chartH.series[0].addPoint([x, y], true, true, true);
      } else {
        chartH.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 3000 ) ;
var chartA = new Highcharts.Chart({
  chart:{ renderTo : 'chart-firstsensor' },
  title: { text: '<b>Soil humidity ( Sensor 1 )</b> ' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: true }
    },
    series: { color: '#059e8a' }
  },
   xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '<b>Humidity (%)</b>' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime() + 1000 * 60 * 60 * 3,
          y = parseFloat(this.responseText);
      if(chartA.series[0].data.length > 12) {
        chartA.series[0].addPoint([x, y], true, true, true);
      } else {
        chartA.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/firstsensor", true);
  xhttp.send();
}, 3000 ) ;
var chartB = new Highcharts.Chart({
  chart:{ renderTo:'chart-secondsensor' },
  title: { text: '<b>Soil Humidity ( Sensor 2 )</b>' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: '<b>Humidity (%)</b>' }
  },
  credits: { enabled: false }
});
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime() + 1000 * 60 * 60 * 3,
          y = parseFloat(this.responseText);
      if(chartB.series[0].data.length > 12) {
        chartB.series[0].addPoint([x, y], true, true, true);
      } else {
        chartB.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/secondsensor", true);
  xhttp.send();
}, 3000 ) ;
</script>
</html>)webpage";

void setup(){
  Wire.begin(D2,D1);
  Serial.begin(115200);                                                               
  ads.setGain(GAIN_ONE); // 1x gain   +/- 4.096V  1 bit =  2mV(ADS1015)   0.125mV (ADS1115)
  pinMode(relay1, OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
 
   // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  // afisare  ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/html", index_html);
});
server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(v).c_str());
});
server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(t).c_str());
});
server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(h).c_str());
});
server.on("/firstsensor", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(a).c_str());
});
server.on("/secondsensor", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(b).c_str());
});
server.begin();
 ads.begin();
 dht.begin();
}

void loop(){ 
  
   int A0Value = analogRead(analogPin);
    float voltage_sensed = A0Value * (espVCC / (float)inputResolution);       
  v = voltage_sensed * ( 1 + ( (float)R2 /  (float)R1) );
  
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);

  if (a < 70 ){
    digitalWrite(relay2,LOW);
    digitalWrite(relay3,LOW);
    delay(2000);
    digitalWrite(relay2,HIGH);
    digitalWrite(relay3,HIGH);
    delay(5000);
  }
  if (b < 70 ){
    digitalWrite(relay1,LOW);
    digitalWrite(relay3,LOW);
    delay(2000);
    digitalWrite(relay1,HIGH);
    digitalWrite(relay3,HIGH);
    delay(5000);
  }
   if (a < 70 && b < 70 ){
    digitalWrite(relay3,LOW);
    delay(2000);
    digitalWrite(relay3,HIGH);
    delay(5000);
  }
  
  int16_t adc0,adc1; 
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; 
    
    float newT = dht.readTemperature();
    if(isnan(newT)){
      Serial.println("Temperature reading error! ");
    }
    else{
      t = newT;
      Serial.println(t);
    }
     float newH = dht.readHumidity();
    if(isnan(newH)){
      Serial.println("Humidity reading error  ! ");
    }
    else{
      h = newH;
      Serial.println(h);
    }
    float newA = map(adc0,23560,10135,0,100);
    if (isnan(newA)) {
      Serial.println("Sensor 1 reading error !");
    }
    else {
      a = newA;
    }
    float newB = map(adc1,23560,9900,0,100); 
    if (isnan(newB)) {
      Serial.println("Sensor 2 reading error  !");
    }
    else {
      b = newB;
    }  
  } 
  
 Serial.print("V: ");
  Serial.print(v);
  Serial.println();
  Serial.print("A0: "); 
  Serial.print(adc0);
  Serial.print("\t");
  Serial.print("Moisture S1: ");
  Serial.print(a);
  Serial.println("%");
  Serial.print("A1: "); 
  Serial.print(adc1);
  Serial.print("\t");
  Serial.print("Moisture S2: ");
  Serial.print(b);
  Serial.println("%");
  Serial.println(); 
     delay(700);
}
