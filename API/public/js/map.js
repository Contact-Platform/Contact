function main() {
  let points = $.ajax({
    url: './api/get/geoJSON',
    dataType: "json",
    error: function (xhr) {
      alert(xhr.statusText)
    }
  });

  $.when(points).done(() => {
    let map = L.map('map', {
      center: [18.466333, -66.105721],
      zoom: 8.2
    });

    let url = 'https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token={accessToken}';

    let params = {
      attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, <a href="https://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
      maxZoom: 18,
      id: 'mapbox.streets',
      accessToken: 'pk.eyJ1Ijoid2ZhbGNvbmVyMSIsImEiOiJjam0yaTczcmIxYXowM2twYXhpemZtc2ZpIn0.s6FN340SxMq3rrrPpVuqEw'
    };

    L.tileLayer(url, params).addTo(map);

    let json = points.responseJSON;
    L.geoJSON(json).addTo(map);

    for (let feature of json.features) {
      L.marker([feature.geometry.coordinates[1], feature.geometry.coordinates[0]])
        .bindPopup('<strong>' + feature.properties.title + '</strong><br>' + feature.properties.description)
        .addTo(map);
    }

    map.on('click', e => {
      L.popup()
        .setLatLng(e.latlng)
        .setContent("The current coordinates of this location are: " + e.latlng.toString())
        .openOn(map);
    });
  });
}

main();
// setInterval(main(1), 60000);