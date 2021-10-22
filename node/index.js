"use strct";
let killtimerid = 0;
let cols = 90;
let rows = 30;
let fontSize = 16;
let msgport = 3000;
let port = 8890;
let port2 = 8891;
let port3 = 8892;
let home = "c:\\";
let home2 = "c:\\";
let home3 = "~/";
let distro = "";
let user = "root";
let createterm = 1;
let theme = {
  "name": "Campbell",
  "cursorColor": "#FFFFFF",
  "selectionBackground": "#FFFFFF",
  "background": "#050505",
  "foreground": "#CCCCCC",
  "black": "#0C0C0C",
  "blue": "#0037DA",
  "cyan": "#3A96DD",
  "green": "#13A10E",
  "purple": "#881798",
  "red": "#C50F1F",
  "white": "#CCCCCC",
  "yellow": "#C19C00",
  "brightBlack": "#767676",
  "brightBlue": "#3B78FF",
  "brightCyan": "#61D6D6",
  "brightGreen": "#16C60C",
  "brightPurple": "#B4009E",
  "brightRed": "#E74856",
  "brightWhite": "#F2F2F2",
  "brightYellow": "#F9F1A5"
};
const express = require('express');
const fs = require('fs');
const exapp = express();
const KeydownParser = require('./KeymapParser.js');
const path = require("path");
const ips = ["127.0.0.1"];
const ipfilter = require("express-ipfilter").IpFilter;
const http = require("http");

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: 3001, host: '127.0.0.1' });
const msgserver = http.createServer(exapp);
const msgio = require("socket.io")(msgserver);

const server = http.createServer(exapp);
const io = require("socket.io")(server);
const server2 = http.createServer(exapp);
const io2 = require("socket.io")(server2);
const server3 = http.createServer(exapp);
const io3 = require("socket.io")(server3);

const pty = require("node-pty");
const store = require("store");
const sockets = {};
const socketids = [];
const pubpath = path.join(__dirname, "public");
const icopath = path.join(__dirname, "img/t.ico");
const pugpath = path.join(__dirname, "views/index.pug");

exapp.use(ipfilter(ips, { mode: "allow" }));
exapp.use(express.static(pubpath));
exapp.set("view engine", "pug");
var kparser = new KeydownParser();

exapp.use(express.urlencoded({
  extended: true
}));
exapp.use(express.json());

const getOptions = () => {
  // const value = store.get("__opt__");
  // if(value)return;
  // let opt = JSON.parse(value);
  // setOptions(opt, true);
};
const setOptions = (opt, nosave) => {
  createterm = opt.term - 0;
  cols = opt.cols - 0;
  rows = opt.rows - 0;
  fontSize = opt.fontSize - 0;
  port = opt.port - 0;
  port2 = opt.port2 - 0;
  port3 = opt.port3 - 0;
  home = opt.home;
  home2 = opt.home2;
  home3 = opt.home3;
  theme = opt.theme;
  distro = opt.distro;
  user = opt.user;
  if (!nosave) store.set("__opt__", JSON.stringify(opt));
};
const sendAll = (msg) => {
  for (let idx = 0; idx < socketids.length; idx++) {
    let id = socketids[idx];
    let socket = sockets[id];
    socket.emit(msg, {
      cols,
      rows,
      fontSize
    });
  }
}
const killNodejs = ()=>{
  process.exit(1);
}
process.on('exit',  (code)=> {
  console.log('Program exit. code = ', code);
});
wss.on('connection', function connection(ws, req) {
  if ("127.0.0.1" !== req.socket.remoteAddress)
    return;
  console.log("WebSocket connection");

  clearTimeout(killtimerid);
  ws.on('message', function incoming(message) {
    const obj = JSON.parse(message);
    let cb = (json) => {
      ws.send(JSON.stringify(json));
    };
    kparser.parse(obj, cb);
  });
  ws.on('close', function () {
    console.log('Client disconnected');
    clearTimeout(killtimerid);
    killtimerid = setTimeout(()=>{
      killNodejs();
    },1000);
  });
  ws.on('error', function (err) {
    console.log('Error: ' + err.code);
  });
});
io.on("connect", (socket) => {
  setConnect("cmd.exe", socket);
});
io2.on("connect", (socket) => {
  setConnect("powershell.exe", socket);
});
io3.on("connect", (socket) => {
  setConnect("wsl.exe", socket, ['-d', distro, '-u', user]);
  sockets[socket.id] = socket;
  socketids.push(socket.id);
});

const setConnect = (shell, socket, opts) => {
  if (!opts) opts = [];
  const term = pty.spawn(shell, opts, {
    name: "xterm-color",
    cols: cols,
    rows: rows,
    cwd: home2,
    env: process.env,
  });
  term.onData((d) => socket.emit("data", d));
  socket.on("theme", (d) => { });
  socket.on("data", (d) => term.write(d));
  socket.on("disconnect", () => {
    delete sockets[socket.id];
    let idx = socketids.indexOf(socket.id)
    if (idx > -1) {
      socketids.splice(idx, 1);
    }
    term.destroy();
  });
  socket.on("resize", (size) => {
    term.resize(
      Math.max(size ? size.cols : term.cols, 1),
      Math.max(size ? size.rows : term.rows, 1)
    );
  });
  socket.on("ready", () => {
    if (shell === "cmd.exe") {
      if (!home) return;
      term.write("cd " + home + "\r\n");
    } else if (shell === "powershell.exe") {
      if (!home2) return;
      term.write("cd " + home2 + "\r\n");
    } else if (shell === "wsl.exe") {
      if (!home3) return;
      term.write("cd " + home3 + "\n");
    }
  });
  socket.emit("init", {
    cols,
    rows,
    fontSize,
    theme,
  });
  return term;
};
getOptions();

exapp.get("/", (req, res) => {
  let rport = req.get('host').split(":")[1];
  if (port === rport - 0) {
    res.render(pugpath, { shell: "cmd-" + createterm + "-" + rows });

  } else if (port2 === rport - 0) {
    res.render(pugpath, { shell: "ps-" + createterm + "-" + rows });

  } else if (port3 === rport - 0) {
    res.render(pugpath, { shell: "wsl-" + createterm + "-" + rows });

  } else if (msgport === rport - 0) {
    fs.readFile("debug.html", function (err, data) {
      res.header('Content-Type', 'text/plain;charset=utf-8');
      res.writeHead(200, { "Content-Type": "text/html" });
      res.write(data);
      res.end();
    });
  }
});
exapp.post('/', function (req, res) {
  res.header('Content-Type', 'text/plain;charset=utf-8');
  console.log(req.body);
  res.json(req.body);
  res.end();
})
msgserver.listen(msgport, "localhost", function () {
});
server.listen(port, "localhost", function () {
});
server2.listen(port2, "localhost", function () {
});
server3.listen(port3, "localhost", function () {
});
console.log('debug  http://localhost:3000');
console.log('Server port:8890');
console.log('Server port:8891');
console.log('Server port:8892');






