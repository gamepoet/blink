$(document).ready(function() {
  var Profiler = function() {
    this.name = null;
    this.start = null;
  };
  Profiler.prototype.begin = function(name) {
    this.name = name;
    this.start = performance.now();
  };
  Profiler.prototype.end = function() {
    var end = performance.now();
    var ms = end - this.start;
    console.log('[profile] ' + ms + ' ' + this.name);
    this.name = null;
    this.start = null;
  };

  var ws = null;
  var canvas = $('#viewport')[0];
  var ctx = canvas.getContext('2d');
  var img = ctx.createImageData(canvas.width, canvas.height);

  var profLatency = new Profiler();
  var profRender = new Profiler();

  $("#viewport").click(function() {
    if (ws == null) {
      ws = new WebSocket('ws://' + location.host + '/ws');
      ws.binaryType = "arraybuffer";
      ws.onopen = function(ev) {
        console.log(ev);
        profLatency.begin('latency');
        ws.send('frame');
      };
      ws.onerror = function(ev) {
        console.log(ev);
      }
      ws.onclose = function(ev) {
        console.log(ev);
      }
      ws.onmessage = function(ev) {
        profLatency.end();
        profRender.begin('render');
    //    console.log(ev);
        //$("#msgs").append('<div>' + ev.data + '</div>');

        var src = new Uint8ClampedArray(ev.data);
        var dst = img.data;
        for (var index = 0; index < src.length; index += 4) {
          dst[index]      = src[index];
          dst[index + 1]  = src[index + 1];
          dst[index + 2]  = src[index + 2];
          dst[index + 3]  = 0xff;
        }
    //    img.data.set(src);

        ctx.putImageData(img, 0, 0);
        profRender.end();
        if (ws != null) {
          profLatency.begin('latency');
          ws.send('frame');
        }
      };
    }
    else {
      ws.close();
      ws = null;
    }
  });
});
