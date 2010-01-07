const cURL = require('curl');
const asserts = require('test').asserts;

exports.test_curlCleanup = function() {
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  c.cleanup();
  asserts.ok(!c.valid());
};

exports.test_curlReset = function() {
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  c.options.url = 'foo';
  asserts.same(c.options.url, 'foo');
  c.reset();
  asserts.ok(c.valid());
  asserts.same(c.options.url, '');
};

exports.test_curlEncode = function() {
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  var input = 'Hellö Wörld!';
  var coded = c.escape(input);
  var result = c.unescape(coded);
  asserts.same(input, result);
};

exports.test_options = function() {
  var c = new cURL.Easy();
  asserts.ok(c.valid());
  asserts.throwsOk(function() {
    c.options.httppost = {
      noname : "yes"
    };
                 }, TypeError);
};

if (require.main === module)
  require('test').runner(exports);