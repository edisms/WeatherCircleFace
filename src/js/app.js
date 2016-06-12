var OWMWeather = require('./lib/owm_weather.js');

var owmWeather = new OWMWeather();

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = "https://dl.dropboxusercontent.com/u/4518579/config/index.html";

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  console.log('Configuration changed');
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));
  /*
          this.sendAppMessage({
          'Reply': this.REPLY_SEGMENT,
          'Time': json.list[c].dt,
          'Segment': c +1,
          'ConditionId': json.list[c].weather[0].id,
          //'Description': json.list[0].weather[0].description,
          //'DescriptionShort': json.list[0].weather[0].main,
          'TempK': Math.round(json.list[c].main.temp),
  
          //'Pressure': Math.round(json.list[c].main.pressure),
          'WindSpeed': Math.round(json.list[c].wind.speed*3.6),
          //'WindDirection': Math.round(json.list[c].wind.deg),
          'Rain': Math.round(rain),
          'Snow' : Math.round(snow),
          'Clouds' : json.list[c].clouds.all
        }, callbackForAck, callbackForNack);    
  */
  /*
    var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var backgroundColor = configData['background_color'];

  var dict = {};
  if(configData['high_contrast'] === true) {
    dict['KEY_HIGH_CONTRAST'] = configData['high_contrast'] ? 1 : 0;  // Send a boolean as an integer
  } else {
    dict['KEY_COLOR_RED'] = parseInt(backgroundColor.substring(2, 4), 16);
    dict['KEY_COLOR_GREEN'] = parseInt(backgroundColor.substring(4, 6), 16);
    dict['KEY_COLOR_BLUE'] = parseInt(backgroundColor.substring(6), 16);
  }

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
  */
});
                        
Pebble.addEventListener('appmessage', function(e) {
  //owmWeather.LOG('appmessage: ' + JSON.stringify(e.payload));
  owmWeather.appMessageHandler(e);
});

