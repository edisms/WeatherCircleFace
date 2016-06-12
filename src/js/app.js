var OWMWeather = require('./lib/owm_weather.js');

var owmWeather = new OWMWeather();

function LOG(x) {
    //console.log(x);
}

Pebble.addEventListener('ready', function(e) {
  LOG('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = "https://dl.dropboxusercontent.com/u/4518579/config/index.html";

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Configuration changed');
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));
  var dict = {};
  dict['KEY_USE_FAKE_WEATHER'] = configData['KEY_USE_FAKE_WEATHER'] ? 1 : 0;
  dict['KEY_API_KEY'] = configData['KEY_API_KEY'];
  dict['KEY_DEFAULT_API_KEY'] = configData['KEY_DEFAULT_API_KEY'] ? 1 : 0;
  dict['KEY_TWENTY_FOUR'] = configData['KEY_TWENTY_FOUR'] ? 1 : 0;
  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    LOG('Send successful: ' + JSON.stringify(dict));
  }, function() {
    LOG('Send failed!');
  });
  
});
                        
Pebble.addEventListener('appmessage', function(e) {
  LOG('appmessage: ' + JSON.stringify(e.payload));
  owmWeather.appMessageHandler(e);
});

