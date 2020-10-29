const WebSocket = require("ws");

const server = new WebSocket.Server({ port: 14050 });

server.on("open", function open() {
  console.log("connected");
});

server.on("close", function close() {
  console.log("disconnected");
});

server.on("connection", function connection(ws, req) {
  const ip = req.connection.remoteAddress;
  const port = req.connection.remotePort;
  const clientName = ip + port;

  console.log("%s is connected", clientName);

  // 发送欢迎信息给客户端
  ws.send("Welcome " + clientName);

  ws.on("message", function incoming(message) {
    console.log("received: %s from %s", message, clientName);

    // 消息给客户端
    if (GSocket) {
      try {
        var data = JSON.parse(message);
        if (data.type) {
          GSocket.write(data.type + "," + data.x + "," + data.y);
        }
      } catch (e) {
        console.log(e);
      }
    }
    // server.clients.forEach(function each(client) {
    //   if (client.readyState === WebSocket.OPEN) {
    //     // client.send(clientName + " -> " + message);
    //     var data = JSON.parse(message);
    //     GSocket.write(data.type + "," + data.x + "," + data.y);
    //   }
    // });
  });
});

var net = require("net");
//模块引入
var GSocket;
var listenPort = 12050; //监听端口
var server2 = net
  .createServer(function (socket) {
    // 创建socket服务端
    console.log("connect: " + socket.remoteAddress + ":" + socket.remotePort);
    socket.setEncoding("binary");
    //接收到数据
    socket.on("data", function (data) {
      console.log("client send:" + data);
    });
    // socket.write("Hello client!\r\n");
    // socket.pipe(socket);
    //数据错误事件
    socket.on("error", function (exception) {
      console.log("socket error:" + exception);
      socket.end();
    });
    //客户端关闭事件
    socket.on("close", function (data) {
      GSocket = NULL;
      console.log("client closed!");
      // socket.remoteAddress + ' ' + socket.remotePort);
    });
    GSocket = socket;
  })
  .listen(listenPort);
//服务器监听事件
server2.on("listening", function () {
  console.log("server listening:" + server.address().port);
});
//服务器错误事件
server2.on("error", function (exception) {
  console.log("server error:" + exception);
});
