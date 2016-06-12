function LOG(x) {
    //console.log(x);
}

var OWMWeather = function(options) {
  this._apiKey = '';
  this._segmentCount = 0;
  this.weather_done = false;
  this.forcast_done = false;
  
  this.REPLY_SEGMENT = 1;
  this.REPLY_LOCATION = 2;
  this.REPLY_DONE = 100;
  
    
  this.timeConverter = function(UNIX_timestamp){
    var a = new Date(UNIX_timestamp * 1000);
    var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
    var year = a.getFullYear();
    var month = months[a.getMonth()];
    var date = a.getDate();
    var hour = a.getHours();
    var min = a.getMinutes();
    var sec = a.getSeconds();
    var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec ;
    return time;
  };

  options = options || {};
  this._appKeyBase = options.baseAppKey || 0;

  this._appKeys = {
    'Request': 1, // framework
    'Reply': 2,
    
    'Time': 20, 
    'Segment': 21,    
    'ConditionId': 22,
    'Description': 23,
    'DescriptionShort': 24,
    'TempK': 25,
    'Pressure': 26,
    'WindSpeed': 27,
    'WindDirection': 28,
    'Rain': 29,
    'Snow': 30,
    'Clouds': 31,
    
    'Name': 50, // location
    'Sunrise': 51,
    'Sunset': 52,
    
    'BadKey': 100, //error handling
    'LocationUnavailable': 101,
    'Error' : 102
  };
  
  this.getAppKey = function(keyName) {
    return this._appKeyBase + this._appKeys[keyName];
  };

  this.sendAppMessage = function(obj, onSuccess, onError) {
    var msg = {};
    for(var key in obj) {
      // Make sure the key exists
      if (!key in this._appKeys) throw 'Unknown key: ' + key;

      msg[this.getAppKey(key)] = obj[key];
    }

    Pebble.sendAppMessage(msg, onSuccess, onError);
  };

  this._xhrWrapper = function(url, type, callback) {
    var xhr = new XMLHttpRequest();
    LOG("request:" + url);
    xhr.onload = function () {
      LOG("callback received");
      callback(xhr);
    };
    xhr.open(type, url);
    xhr.send();
  };
  
  function callbackForAck(data)
  {
    LOG("callbackForAck: " + data);
  }
  
  function callbackForNack(data, error)
  {
    LOG("callbackForNack: " + data);
    LOG("callbackForNack: " + error);
    var propValue;
    for(var propName in data) {
        propValue = data[propName];
    
        LOG(propName,propValue);
    }
  }

  this.notifyDone = function() {
    if (this.weather_done && this.forcast_done)
    {
      LOG("send done");
      this.sendAppMessage({
        'Reply': this.REPLY_DONE,
        }, callbackForAck, callbackForNack); 
    }
  };
  
  this.sendToPebbleWeather = function(json) {
    try {
      LOG("City:" + json.city.name);
      var count = json.cnt;
      for (var c = 0; (c < count) && (c < 11); c++)
      {
        //LOG("Weather: "+ c + " time " + this.timeConverter(json.list[c].dt) + 
        //            " type " + json.list[c].weather[0].id + ' clouds: ' + json.list[c].clouds.all + 
        //           " temp:" + json.list[c].main.temp +
        //            " wind:" + json.list[c].wind.speed);  
              
        var rain = 0;
        if (json.list[c].hasOwnProperty("rain"))
        {      
          if (json.list[c].rain.hasOwnProperty("3h")) {
            rain = json.list[c].rain["3h"];
          }
        }
        var snow = 0;
        if (json.list[c].hasOwnProperty("snow"))
        {
          if (json.list[c].snow.hasOwnProperty("3h")) {
            snow = json.list[c].snow["3h"];
          }
        }  
        //LOG("rain = " + rain + " snow " + snow);
    
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
      }
      this.forcast_done = true;
  
      this.notifyDone();  
    }
    catch(err) {

      LOG("send error");
      this.sendAppMessage({
        'Error': "failed to parse XML forecast",
        }, callbackForAck, callbackForNack); 
    }      
    
  };
  
  this.sendToPebbleLocation = function(json) {
    try {
      LOG("City:" + json.name);
      LOG(json.sys.sunrise);
      LOG(json.sys.sunset);
    
      var rain = 0;
      if (json.hasOwnProperty("rain"))
      {      
        if (json.rain.hasOwnProperty("3h")) {
            rain = json.rain["3h"];
        }
      }
      var snow = 0;
      if (json.hasOwnProperty("snow"))
      {
        if (json.snow.hasOwnProperty("3h")) {
          snow = json.snow["3h"];
        }
      }  
      LOG("rain = " + rain + " snow " + snow);
    
      this.sendAppMessage({
        'Reply': this.REPLY_SEGMENT,
        'Time': json.dt,
        'Segment': 0,
        'ConditionId': json.weather[0].id,
        //'Description': json.list[0].weather[0].description,
        //'DescriptionShort': json.list[0].weather[0].main,
        'TempK': Math.round(json.main.temp),
  
        //'Pressure': Math.round(json.list[c].main.pressure),
        'WindSpeed': Math.round(json.wind.speed*3.6),
        //'WindDirection': Math.round(json.list[c].wind.deg),
        'Rain': Math.round(rain),
        'Snow' : Math.round(snow),
        'Clouds' : json.clouds.all
      }, callbackForAck, callbackForNack);      
  
      this.sendAppMessage({
        'Reply': this.REPLY_LOCATION,
        'Name': json.name,
        'Sunrise': Math.round(json.sys.sunrise),
        'Sunset': Math.round(json.sys.sunset) }, callbackForAck, callbackForNack);
  
      this.weather_done = true;
      this.notifyDone();  
      }
    catch(err) {

      LOG("send error");
      this.sendAppMessage({
        'Error': "failed to parse XML weather",
        }, callbackForAck, callbackForNack); 
    } 
  };


  this._onLocationSuccess = function(pos) {
    var urlForcast = 'http://api.openweathermap.org/data/2.5/forecast?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + this._apiKey;
    var urlWeather = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + this._apiKey;
    //urlForcast = "http://api.openweathermap.org/data/2.5/forecast?q=London,uk&appid=f402bfb5a34389b2501c3e7007b46668";
    //urlWeather = "http://api.openweathermap.org/data/2.5/weather?q=London,uk&appid=f402bfb5a34389b2501c3e7007b46668";
    LOG('owm-weather: Location success. Contacting OpenWeatherMap.org...');
    LOG(urlForcast);
    LOG(urlWeather);
    this.weather_done = false;
    this.forcast_done = false;

    this._xhrWrapper(urlForcast, 'GET', function(req) {
      LOG('owm-weather: Got API response!');
      if(req.status == 200) {
        this.sendToPebbleWeather(JSON.parse(req.response));
      } else {
        LOG('owm-weather: Error fetching data (HTTP Status: ' + req.status + ')');
        this.sendAppMessage({ 'BadKey': 1 });
      }
    }.bind(this));
    
    this._xhrWrapper(urlWeather, 'GET', function(req) {
      LOG('owm-weather: Got API response!');
      if(req.status == 200) {
        this.sendToPebbleLocation(JSON.parse(req.response));
      } else {
        LOG('owm-weather: Error fetching data (HTTP Status: ' + req.status + ')');
        this.sendAppMessage({ 'BadKey': 1 });
      }
    }.bind(this));
    
  };

  this._onLocationError = function(err) {
    this.LOG('owm-weather: Location error');
    this.sendAppMessage({
      'LocationUnavailable': 1
    });
  };

  this.appMessageHandler = function(e) {
    var dict = e.payload;
    if(dict["Request"]) {
      this._apiKey = dict['Request'];
      if(dict['Segment']) {
        this._segmentCount = dict['Segment'];
      } 
      else {
        this._segmentCount = 10;
      }
        
      navigator.geolocation.getCurrentPosition(
        this._onLocationSuccess.bind(this),
        this._onLocationError.bind(this), {
          timeout: 15000,
          maximumAge: 60000
      });
    }
  };
};

module.exports = OWMWeather;
