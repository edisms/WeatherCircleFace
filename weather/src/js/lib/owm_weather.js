var OWMWeather = function(options) {
  this._apiKey = '';
  this._segmentCount = 0;
  
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
    'Request': 0,
    'Reply': 1,
    'Time': 2,
    'Segment': 3,    
    'ConditionId': 4,
    'Description': 5,
    'DescriptionShort': 6,
    'Name': 7,
    'TempK': 8,
    'Pressure': 9,
    'WindSpeed': 10,
    'WindDirection': 11,
    'Rain': 12,
    'Snow': 13,
    'Clouds': 14,
    'BadKey': 15,
    'LocationUnavailable': 16,
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
    xhr.onload = function () {
      callback(xhr);
    };
    xhr.open(type, url);
    xhr.send();
  };
  
    function callbackForAck(data)
  {
    console.log("callbackForAck: " + data);
  }
  
  function callbackForNack(data, error)
  {
    console.log("callbackForNack: " + data);
    console.log("callbackForNack: " + error);
    var propValue;
    for(var propName in data) {
        propValue = data[propName]
    
        console.log(propName,propValue);
    }
  }

  this.sendToPebble = function(json) {
    console.log("City:" + json.city.name);
    var count = json.cnt;
    for (var c = 0; (c < count) && (c < 11); c++)
    {
      console.log("Weather: "+ c + " time " + this.timeConverter(json.list[c].dt) + " type " + 
                json.list[c].weather[0].id + ' ' + json.list[c].clouds.all);  
            
      var rain = 0;
      if (json.list[c].rain.hasOwnProperty("3h")) {
        rain = json.list[c].rain["3h"];
      }
      var snow = 0;
      if (json.list[c].hasOwnProperty("snow"))
      {
        if (json.list[c].snow.hasOwnProperty("3h")) {
          snow = json.list[c].snow["3h"];
        }
      }  
      console.log("rain = " + rain + " snow " + snow);
  
      this.sendAppMessage({
        'Reply': 1,
        'Time': json.list[c].dt,
        'Segment': c,
        'ConditionId': json.list[c].weather[0].id,
        //'Description': json.list[0].weather[0].description,
        //'DescriptionShort': json.list[0].weather[0].main,
        'TempK': Math.round(json.list[0].main.temp),
        'Name': json.city.name,
        'Pressure': Math.round(json.list[0].main.pressure),
        'WindSpeed': Math.round(json.list[0].wind.speed),
        'WindDirection': Math.round(json.list[0].wind.deg),
        'Rain': Math.round(rain),
        'Snow' : Math.round(snow),
        'Clouds' : json.list[c].clouds.all
      }, callbackForAck, callbackForNack);      
        
    }
    console.log("send done");
    var a = this.sendAppMessage({
      'Reply': 2,
      'Segment':0
    }, callbackForAck, callbackForNack);   
    console.log("send done 2: " + a);
  };

  this._onLocationSuccess = function(pos) {
    var url = 'http://api.openweathermap.org/data/2.5/forecast?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + this._apiKey;
    url = "http://api.openweathermap.org/data/2.5/forecast?q=London,us&appid=f402bfb5a34389b2501c3e7007b46668";
    console.log('owm-weather: Location success. Contacting OpenWeatherMap.org...');
    console.log(url);

    this._xhrWrapper(url, 'GET', function(req) {
      console.log('owm-weather: Got API response!');
      if(req.status == 200) {
        this.sendToPebble(JSON.parse(req.response));
      } else {
        console.log('owm-weather: Error fetching data (HTTP Status: ' + req.status + ')');
        this.sendAppMessage({ 'BadKey': 1 });
      }
    }.bind(this));
  };

  this._onLocationError = function(err) {
    console.log('owm-weather: Location error');
    this.sendAppMessage({
      'LocationUnavailable': 1
    });
  };

  this.appMessageHandler = function(dict) {
    if(dict.payload[this.getAppKey('Request')]) {
      this._apiKey = dict.payload[this.getAppKey('Request')];
      if(dict.payload[this.getAppKey('Segment')]) {
        this._segmentCount = dict.payload[this.getAppKey('Segment')];
      } 
      else {
        this._segmentCount = 10;
      }
        
      console.log('owm-weather: Got fetch request from C app');

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