#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>

#define PIR_PIN D5
#define REED_PIN D7
#define TRIG_PIN D1
#define ECHO_PIN D2
#define BUZZER D6
#define LED D4
#define FLAME_PIN D3
#define POWER_LED D8

ESP8266WebServer server(80);

// SYSTEM FLAGS
bool motionSystem=false;
bool doorSystem=false;
bool ultraSystem=false;

// SENSOR VARIABLES
long duration;
int distance;
int previousMotion=LOW;
int lastDoorState;
bool fireDetected=false;

// LED BLINK
unsigned long ledBlinkTime=0;
bool ledBlinking=false;
const int blinkDuration=200;

// LOGS
String motionLogs="";
String doorLogs="";
String ultraLogs="";
String fireLogs="";
String alertMsg="";

// WIFI
const char* ssid="Nikendhuku";
const char* password="kannayya..";
const char* loginUser="sree8822";
const char* loginPass="2519";
bool loggedIn=false;


// LED BLINK FUNCTION
void blinkLED(){
digitalWrite(LED,HIGH);
ledBlinkTime=millis();
ledBlinking=true;
}


// LOGIN PAGE
const char* loginPage = R"rawliteral(
<html>
<head>
<title>Login</title>
<style>
body{font-family:Arial;background:#eef5ff;text-align:center;margin-top:100px;}
.box{background:white;padding:30px;border-radius:10px;display:inline-block;box-shadow:0 4px 10px rgba(0,0,0,0.3);}
input{padding:10px;margin:10px;width:200px;}
button{padding:10px 20px;background:#007bff;color:white;border:none;}
</style>
</head>

<body>

<div class="box">

<h2>Security Login</h2>

<input id="u" placeholder="Username"><br>
<input id="p" type="password" placeholder="Password"><br>

<button onclick="login()">Login</button>

</div>

<script>

function login(){

var u=document.getElementById("u").value
var p=document.getElementById("p").value

fetch("/login?u="+u+"&p="+p)
.then(r=>r.text())
.then(d=>{
if(d=="ok"){location="/dashboard"}
else{alert("Wrong Login")}
})

}

</script>

</body>
</html>
)rawliteral";



// DASHBOARD
const char* dashboard = R"rawliteral(

<html>

<head>

<meta name="viewport" content="width=device-width">

<style>

body{
font-family:Arial;
background:#eef5ff;
margin:0;
}

header{
background:#003366;
color:white;
padding:15px;
font-size:22px;
text-align:center;
}

.container{
display:grid;
grid-template-columns:repeat(auto-fit,minmax(200px,1fr));
gap:20px;
padding:20px;
}

.card{
background:white;
padding:20px;
border-radius:10px;
box-shadow:0 4px 10px rgba(0,0,0,0.2);
}

button{
padding:8px 15px;
border:none;
border-radius:6px;
color:white;
margin:5px;
cursor:pointer;
}

.onBtn{background:#007bff;}
.offBtn{background:#ff4d4d;}
.clearBtn{background:#ff9800;}

.logs{
background:white;
height:200px;
overflow:auto;
margin:20px;
padding:15px;
display:grid;
grid-template-columns:repeat(4,1fr);
}

.alert{
position:fixed;
top:20px;
right:20px;
background:red;
color:white;
padding:15px;
display:none;
}

</style>

</head>


<body>

<header>Smart Security Dashboard</header>

<div id="alert" class="alert"></div>


<div class="container">

<div class="card">
<h3>Motion</h3>
<div id="motionStatus">--</div>

<button id="motionON" class="onBtn" onclick="motionOn()">ON</button>
<button id="motionOFF" class="offBtn" onclick="motionOff()">OFF</button>
<button class="clearBtn" onclick="clearMotion()">CLEAR</button>
</div>


<div class="card">
<h3>Door</h3>
<div id="doorStatus">--</div>

<button id="doorON" class="onBtn" onclick="doorOn()">ON</button>
<button id="doorOFF" class="offBtn" onclick="doorOff()">OFF</button>
<button class="clearBtn" onclick="clearDoor()">CLEAR</button>
</div>


<div class="card">
<h3>Ultrasonic</h3>
<div id="ultraStatus">--</div>

<button id="ultraON" class="onBtn" onclick="ultraOn()">ON</button>
<button id="ultraOFF" class="offBtn" onclick="ultraOff()">OFF</button>
<button class="clearBtn" onclick="clearUltra()">CLEAR</button>
</div>


<div class="card">
<h3>Fire</h3>
<div id="fireStatus">Monitoring...</div>
<button class="clearBtn" onclick="clearFire()">CLEAR</button>
</div>

</div>


<h3 style="padding-left:20px;">Event History</h3>

<div class="logs">

<div id="motionLogCol"><h4>Motion</h4></div>
<div id="doorLogCol"><h4>Door</h4></div>
<div id="ultraLogCol"><h4>Ultrasonic</h4></div>
<div id="fireLogCol"><h4>Fire</h4></div>

</div>

<div style="text-align:center;margin:20px">
<button class="clearBtn" onclick="clearAll()">CLEAR ALL LOGS</button>
</div>


<script>

function popup(msg){

var a=document.getElementById("alert")

a.innerHTML=msg
a.style.display="block"

setTimeout(()=>{a.style.display="none"},3000)

}


// MOTION

function motionOn(){

fetch("/motionOn")

document.getElementById("motionON").style.background="green"
document.getElementById("motionOFF").style.background="#ff4d4d"

}

function motionOff(){

fetch("/motionOff")

document.getElementById("motionON").style.background="#007bff"
document.getElementById("motionOFF").style.background="red"

}


// DOOR

function doorOn(){

fetch("/doorOn")

document.getElementById("doorON").style.background="green"
document.getElementById("doorOFF").style.background="#ff4d4d"

}

function doorOff(){

fetch("/doorOff")

document.getElementById("doorON").style.background="#007bff"
document.getElementById("doorOFF").style.background="red"

}


// ULTRASONIC

function ultraOn(){

fetch("/ultraOn")

document.getElementById("ultraON").style.background="green"
document.getElementById("ultraOFF").style.background="#ff4d4d"

}

function ultraOff(){

fetch("/ultraOff")

document.getElementById("ultraON").style.background="#007bff"
document.getElementById("ultraOFF").style.background="red"

}


// CLEAR FUNCTIONS

function clearMotion(){fetch("/clearMotion")}
function clearDoor(){fetch("/clearDoor")}
function clearUltra(){fetch("/clearUltra")}
function clearFire(){fetch("/clearFire")}
function clearAll(){fetch("/clearAll")}


// LOAD LOGS

function loadLogs(){

fetch("/logs")
.then(r=>r.json())
.then(data=>{

document.getElementById("motionLogCol").innerHTML="<h4>Motion</h4>"+data.motion

document.getElementById("doorLogCol").innerHTML="<h4>Door</h4>"+data.door
document.getElementById("ultraLogCol").innerHTML="<h4>Ultrasonic</h4>"+data.ultra
document.getElementById("fireLogCol").innerHTML="<h4>Fire</h4>"+data.fire

document.getElementById("motionStatus").innerText=data.motionStatus
document.getElementById("doorStatus").innerText=data.doorStatus
document.getElementById("ultraStatus").innerText=data.ultraStatus
document.getElementById("fireStatus").innerText=data.fireStatus

})

}

setInterval(loadLogs,2000)


setInterval(()=>{

fetch("/alert")
.then(r=>r.text())
.then(d=>{
if(d!=""){popup(d)}
})

},1500)

</script>

</body>
</html>

)rawliteral";


// SETUP
void setup(){

Serial.begin(115200);

pinMode(PIR_PIN,INPUT);
pinMode(REED_PIN,INPUT_PULLUP);
pinMode(TRIG_PIN,OUTPUT);
pinMode(ECHO_PIN,INPUT);
pinMode(BUZZER,OUTPUT);
pinMode(LED,OUTPUT);
pinMode(FLAME_PIN,INPUT);
pinMode(POWER_LED,OUTPUT);

digitalWrite(POWER_LED,HIGH);

lastDoorState=digitalRead(REED_PIN);

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED){
delay(500);
Serial.print(".");
}

Serial.println(WiFi.localIP());

configTime(5*3600+19800,0,"pool.ntp.org","time.nist.gov");


// ROUTES
server.on("/",[]{server.send(200,"text/html",loginPage);});

server.on("/login",[]{

String u=server.arg("u");
String p=server.arg("p");

if(u==loginUser && p==loginPass){
loggedIn=true;
server.send(200,"text/plain","ok");
}
else{
server.send(200,"text/plain","fail");
}

});

server.on("/dashboard",[]{

if(!loggedIn){
server.send(200,"text/html",loginPage);
return;
}

server.send(200,"text/html",dashboard);

});


server.on("/motionOn",[]{motionSystem=true; server.send(200,"text/plain","ok");});
server.on("/motionOff",[]{motionSystem=false; server.send(200,"text/plain","ok");});

server.on("/doorOn",[]{doorSystem=true; server.send(200,"text/plain","ok");});
server.on("/doorOff",[]{doorSystem=false; server.send(200,"text/plain","ok");});

server.on("/ultraOn",[]{ultraSystem=true; server.send(200,"text/plain","ok");});
server.on("/ultraOff",[]{ultraSystem=false; server.send(200,"text/plain","ok");});


server.on("/clearMotion",[]{motionLogs=""; server.send(200,"text/plain","ok");});
server.on("/clearDoor",[]{doorLogs=""; server.send(200,"text/plain","ok");});
server.on("/clearUltra",[]{ultraLogs=""; server.send(200,"text/plain","ok");});
server.on("/clearFire",[]{fireLogs=""; server.send(200,"text/plain","ok");});

server.on("/clearAll",[]{
motionLogs="";
doorLogs="";
ultraLogs="";
fireLogs="";
server.send(200,"text/plain","ok");
});


server.on("/logs",[]{

String json="{";

json+="\"motion\":\""+motionLogs+"\",";
json+="\"door\":\""+doorLogs+"\",";
json+="\"ultra\":\""+ultraLogs+"\",";
json+="\"fire\":\""+fireLogs+"\",";

json+="\"motionStatus\":\""+String(digitalRead(PIR_PIN)==HIGH?"Active":"Idle")+"\",";
json+="\"doorStatus\":\""+String(digitalRead(REED_PIN)==HIGH?"Open":"Closed")+"\",";
json+="\"ultraStatus\":\""+String(distance<10?"Person Close":"Clear")+"\",";
json+="\"fireStatus\":\""+String(fireDetected?"Fire Detected":"Monitoring")+"\"";

json+="}";

server.send(200,"application/json",json);

});


server.on("/alert",[]{
server.send(200,"text/plain",alertMsg);
alertMsg="";
});

server.begin();

}


// LOOP
void loop(){

server.handleClient();

struct tm timeinfo;
char buf[25];

if(!getLocalTime(&timeinfo)) return;

strftime(buf,sizeof(buf),"%d-%m-%Y %H:%M:%S",&timeinfo);
String timestamp=String(buf);


// FIRE

int fire=digitalRead(FLAME_PIN);

if(fire==LOW){

if(!fireDetected){

blinkLED();
fireLogs+="<li>"+timestamp+" 🔥 Fire Detected</li>";
alertMsg="🔥 Fire Detected";

fireDetected=true;

}

digitalWrite(BUZZER,HIGH);

}
else{

fireDetected=false;
digitalWrite(BUZZER,LOW);

}


// MOTION

if(motionSystem){

int motion=digitalRead(PIR_PIN);

if(motion==HIGH){
delay(80); // small filter
if(digitalRead(PIR_PIN)==HIGH){

if(previousMotion==LOW){

blinkLED();
motionLogs+="<li>"+timestamp+" Motion Detected</li>";
alertMsg="Motion Detected";

previousMotion=HIGH;

}

}
}

if(motion==LOW){
previousMotion=LOW;
}

}

// DOOR

if(doorSystem){

int doorState=digitalRead(REED_PIN);

if(doorState!=lastDoorState){

blinkLED();

if(doorState==HIGH){
doorLogs+="<li>"+timestamp+" Door Open</li>";
alertMsg="Door Open";
}
else{
doorLogs+="<li>"+timestamp+" Door Closed</li>";
alertMsg="Door Closed";
}

lastDoorState=doorState;

}

}


// ULTRASONIC

if(ultraSystem){

digitalWrite(TRIG_PIN,LOW);
delayMicroseconds(2);

digitalWrite(TRIG_PIN,HIGH);
delayMicroseconds(10);

digitalWrite(TRIG_PIN,LOW);

duration=pulseIn(ECHO_PIN,HIGH);

distance=duration*0.034/2;

if(distance>0 && distance<10){

blinkLED();
digitalWrite(BUZZER,HIGH);

ultraLogs+="<li>"+timestamp+" Person Very Close</li>";
alertMsg="Person very close";

}
else{

digitalWrite(BUZZER,LOW);

}

delay(300);

}


// LED OFF

if(ledBlinking && millis()-ledBlinkTime>blinkDuration){

digitalWrite(LED,LOW);
ledBlinking=false;

}

delay(80);

}