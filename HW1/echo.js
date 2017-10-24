/*
 * echo.js
 * Andrew Armenia
 * 3-1-2015
 * 
 * Echo client for testing of Network Programming Project 1
 *
 * DO NOT WRITE YOUR SOLUTION IN NODE.JS
 * YOUR SUBMISSION MUST BE WRITTEN IN C OR C++
 *
 * Run this as: node echo.js
 * Modify the addresses below so it registers with your server instance.
 */

var dgram = require('dgram');

// create a UDP socket
var s = dgram.createSocket('udp4');

// function to send registration packet to server
function register() {
	var packet = new Buffer("REGISTER echo");
	s.send(packet, 0, packet.length, 12345, "192.241.177.112");
}

// echo back any received packet
s.on('message', function(msg, rinfo) {
	s.send(msg, 0, msg.length, rinfo.port, rinfo.address);
});

// re-register every minute
setInterval(register, 60000);

// register right away
register();

