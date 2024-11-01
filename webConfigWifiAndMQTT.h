const char MainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>WiFi and MQTT config</title>
    <style>
        button, div, span{
            font-family: Arial, sans-serif;
        }
        body {
            background-color: rgb(246, 240, 240);
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }

        #mainTitle {
            font-size: 29px;
            font-weight: 700;
            display: flex;
            align-content: center;
            justify-content: center;
            margin-top: 50px;
        }
        #deviceName{
            margin-top: 20px;
            font-size: 20px;
            font-weight: 450;
        }

        #wifiForm, #mqttForm, #wifiFormsub, #mqttFormsub {
            margin-top: 20px;
            border-radius: 4px;
            border: solid 1px;
            border-color: rgb(69, 69, 69);
            border-style: solid;
            display: flex;
            flex-direction: column;
            width: 340px;
            justify-content: center;
        }
        #wifiFormsub, #mqttFormsub{
            border: none;
        }
        .formLabel{
            margin-top: 15px;
            margin-left: 5px;
            margin-right: 5px;
            margin-bottom: 3px;
            font-size: 16px;
            font-weight: 600;
        }
        .formInput{
            margin-left: 5px;
            margin-right: 5px;
            height: 25px;
        }
        .saveButton{
            margin-left: 5px;
            margin-right: 5px;
            margin-top: 10px;
            margin-bottom: 5px;
            height: 40px;
            font-weight: 600;
            background-color: #47c266;
            color: aliceblue;
            border-radius: 5px;
            cursor: pointer;
            border: none;
            transition: box-shadow 0.45s;
        }
        .saveButton:active{
            background-color: #139442;
            box-shadow: 0px 0px 1px 1px cadetblue;
        }


        #configContainer {
            display: flex;
            flex-direction: column;
            justify-content: center;
            height: 100px;
            width: 350px;
            margin-top: 70px;
        }
        .formTitle{
            font-size: 18px;
            font-weight: 600;
            display: flex;
            justify-content: center;
            margin-top: 5px;
        }
        a:link{
            text-decoration: none;
            color: aliceblue;
        }
    </style>
</head>

<body>
    <div id="mainTitle">WiFi and MQTT Config</div>
    <div id = "deviceName">ESP8266</div>
    <!-- Form cấu hình WiFi -->
    <div id="wifiForm">
        <div class="formTitle">WiFi Parameters</div>
        <form id="wifiFormsub" action="/savewifi" method="POST"> <!-- Sử dụng form và phương thức POST -->
            <label for="wifiSSID" class="formLabel">WiFi SSID</label>
            <input type="text" id="wifiSSID" class="formInput" placeholder="Enter WiFi SSID" name="ssid">
    
            <label for="wifiPassword" class="formLabel">WiFi Password</label>
            <input type="password" id="wifiPassword" class="formInput" placeholder="Enter WiFi Password" name="password" minlength="8" required>
    
            <button type="submit" id="btnSaveWifi" class="saveButton">Save WiFi</button>
        </form>
    </div>

    <!-- Form cấu hình MQTT -->
    <div id="mqttForm">
        <div class="formTitle">MQTT Parameters</div>
        <form  id="mqttFormsub" action="/savemqtt" method="POST"> <!-- Sử dụng form và phương thức POST -->
            <label for="mqttHost" class="formLabel">Host</label>
            <input type="text" id="mqttHost" class="formInput" placeholder="Enter MQTT Host" name="server">
    
            <label for="mqttPort" class="formLabel">Port <span>(1883)</span></label>
            <input type="text" id="mqttPort" class="formInput" placeholder="1883" name="port">
    
            <label for="mqttClient" class="formLabel">Client</label>
            <input type="text" id="mqttClient" class="formInput" placeholder="Enter Client ID" name="clientID">
    
            <label for="mqttUser" class="formLabel">User</label>
            <input type="text" id="mqttUser" class="formInput" placeholder="Enter Username" name="user">
    
            <label for="mqttPassword" class="formLabel">Password</label>
            <input type="password" id="mqttPassword" class="formInput" placeholder="Enter Password" name="pass">
    
            <button type="submit" id="btnSaveMqtt" class="saveButton">Save MQTT</button>
        </form>
    </div>
    
</body>
</html>
)=====";


const char wifiSuccessfulConnectionWeb[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <style>
            body{
                font-family: Arial, Helvetica, sans-serif;
                background-color: rgb(210, 229, 245);
                display: flex;
                flex-direction: column;
                align-items: center;
            }
            #device{
                font-size: 50px;
                display: flex;
                margin: 50px 0 0 0;
                font-weight: 600;
            }
            #status{
                font-size: 20px;
                margin: 20px;
                font-style: italic;
                color: darkcyan;
            }
        </style>
    </head>
    <body>
        <div id="device">ESP8266</div>
        <div id = "status">Successful WiFi Connection</div>
        <div id = "noti">Redirecting to new device's IP address</div>
    </body>
</html>
)=====";

const char mqttSuccessfulConnectionWeb[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <style>
            body{
                font-family: Arial, Helvetica, sans-serif;
                background-color: rgb(210, 229, 245);
                display: flex;
                flex-direction: column;
                align-items: center;
            }
            #device{
                font-size: 50px;
                display: flex;
                margin: 50px 0 0 0;
                font-weight: 600;
            }
            #status{
                font-size: 20px;
                margin: 20px;
                font-style: italic;
                color: darkcyan;
            }
        </style>
    </head>
    <body>
        <div id="device">ESP8266</div>
        <div id = "status">Successful MQTT Connection</div>
    </body>
</html>
)=====";