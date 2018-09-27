document.getElementById('form').addEventListener('submit', () => {
  event.preventDefault(); //Don't reload page
  var request = new XMLHttpRequest(); //new HTTP Request
  request.open('POST', './api/add/location', true); // route
  request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

  // Input values
  let id = document.getElementById('id').value;
  let lat = document.getElementById('lat').value;
  let lon = document.getElementById('lon').value;
  let desc = document.getElementById('desc').value;

  // make it a json object
  let data = JSON.stringify({
    id: id,
    lat: lat,
    lon: lon,
    description: desc
  })
  request.send(data); // send the object


  // clear form inputs
  document.getElementById('id').value = '';
  document.getElementById('lat').value = '';
  document.getElementById('lon').value = '';
  document.getElementById('desc').value = '';
});