var OWMWeather = require('lib/owm_weather.js');

var owmWeather = new OWMWeather();

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  //owmWeather.LOG('appmessage: ' + JSON.stringify(e.payload));
  owmWeather.appMessageHandler(e);
});
