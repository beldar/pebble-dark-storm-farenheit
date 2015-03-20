var owkey = '9fe68485658a53b68b95fafb9ea4d1d1';
// make it easy to convert an icon to a number for the pebble side
var icons = [
  'clear-day',
  'clear-night',
  'cloudy',
  'fog',
  'partly-cloudy-day',
  'partly-cloudy-night',
  'rain',
  'sleet',
  'snow',
  'wind',
  'error',
  'storm'
];

var owicons = {
  '01d' : 'clear-day',
  '01n' : 'clear-night',
  '02d' : 'partly-cloudy-day',
  '02n' : 'partly-cloudy-night',
  '03d' : 'cloudy',
  '03n' : 'cloudy',
  '04d' : 'cloudy',
  '04n' : 'cloudy',
  '09d' : 'rain',
  '09n' : 'rain',
  '10d' : 'rain',
  '10n' : 'rain',
  '11d' : 'storm',
  '11n' : 'storm',
  '13d' : 'snow',
  '13n' : 'snow',
  '50d' : 'fog',
  '50n' : 'fog'
};

var owids = {
  905 : 'wind',
  957 : 'wind',
  611 : 'sleet',
  612 : 'sleet'
};


function getAndShowWeather ( ) {
  navigator.geolocation.getCurrentPosition(function (position) {
    // position.coords.latitude, position.coords.longitude
    getCurrentWeather(position.coords.longitude, position.coords.latitude);
  });

  setTimeout(getAndShowWeather, 1800000); //30 min
}

function getCurrentWeather (lon, lat) {
  console.log('Get weather lon:'+lon+', lat:'+lat);
  var req = new XMLHttpRequest();
    req.open('GET','http://api.openweathermap.org/data/2.5/weather?APPID=' + owkey + '&lat=' + lat + '&lon=' + lon, true);
    req.onload = function(e) {
      if (req.readyState == 4 && req.status == 200) {
        if(req.status == 200) {
          var response = JSON.parse(req.responseText);
          var icon = 10;
          var iconName = '';
          var send = { };
        
          if (response.weather && response.main) {
            if (typeof owids[response.weather[0].id] !== 'undefined') {
              iconName = owids[response.weather[0].id];
            } else {
              iconName = owicons[response.weather[0].icon];
            }

            if (icons.indexOf(iconName) !== -1) {
              icon = icons.indexOf(iconName);
            }

            send.icon = icon;
            send.temperature_f = Number(KtoF(response.main.temp)).toFixed(0);
            send.temperature_c = Number(KtoC(response.main.temp)).toFixed(0);
            send.precip = Number(response.main.humidity).toFixed(0);
          } else {
            console.log('Error');
          }

          Pebble.sendAppMessage(send);
        } else {
          console.log('Error');
        }
      }
    };

    req.send(null);
}

function KtoC (t) {
  t = parseFloat(t);
  return t - 273.15;
}

function KtoF (t) {
  t = parseFloat(t);
  return 1.8 * (t - 273.15) + 32;
}

Pebble.addEventListener('ready',
  function(e) {
    setTimeout(getAndShowWeather, 2000);
  }
);
