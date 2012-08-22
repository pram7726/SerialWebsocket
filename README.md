###What?
Running this program exposes your serial ports to a websocket.

###Credits
  For my own lazyness I'm using
  
  qt-json by Eeli Reilin
    https://github.com/ereilin/qt-json
  
  QextSerialPort
    https://code.google.com/p/qextserialport/

  QtWebsocket
    http://gitorious.org/qtwebsocket

###Usage
Send commands as JSON to the socket {"type": "command"} 
To include simple data add {"type": "command", "data": "JSON encoded data"}

All serialdata to and from the server is encoded with base64,
all other data sent to and from are plaintext.

Commands
*	connect
*	disconnect
*	listSerialPorts
*	supportedConfiguration
*	serial

####connect

*	serial(required, port to connect)
*	baudrate
*	databits
*	stopbits
*	parity
*	flowcontrol

if a option is left out it will use default values as follows:
*	baudrate	: 57600
*	databits	: 8
*	stopbits	: 1
*	parity		: NONE
*	flowcontrol	: OFF

acceptable values can be retrieved with the command "supportedConfiguration"

	ws.send('{
		"type": "connect", 
		"data": {
			"serial": "ttyUSB0",
			"baudrate": 115200
		}
	}');

####disconnect
*	serial(port to stop listening on)
stops You from getting messages from specified port

	ws.send('{
		"type": "disconnect", 
		"data": {
			"serial": "ttyUSB0"
		}
	}');

####listSerialPorts
no options, will give You a list of serialports with portname and description

	ws.send('{
		"type": "listSerialPorts"
	}');

####supportedConfiguration
no options, sends You a list with acceptable options for connect
	
	ws.send('{
		"type": "supportedConfiguration"
	}');

####serial
*portname, serialport wich will recieve the data sent
*data, base64 encoded datastring

	ws.send('{
		"type": "serial", 
		"data":{
			"serial": "ttyUSB0",
			"data": window.atob("this is websocket speaking")
		}
	}');


The websocket server will respond with theese JSON objects
	
#####serialPorts
	{
		"type": "serialPorts",
		"data": [
			{ "port": "ttyUSB0", "desc": "USB-serial adapter 0"},
			{ "port": "rfcomm0", "desc": "Bluetooth-serial adapter 0"}
		]
			
	}

#####supportedConfiguration
	{
		"type": "supportedConfiguration",
		"data": {
			"databits": [5,6,7,8],
			"flowcontrol": ["HARDWARE", "XONXOFF", "OFF"],
			"parity": ["ODD", "EVEN", "NONE", "SPACE"],
			"stopbits": ["1", "2"]
		}
	}

#####serialData
	{
		"type": "serialData",
		"data" : {
			"serial": "ttyUSB0",
			"base"	: base64 encoded serialdata

		}
	}
