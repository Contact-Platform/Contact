document.getElementById('form').addEventListener('submit', () => {
  event.preventDefault(); //Don't reload page
  var request = new XMLHttpRequest(); //new HTTP Request
  request.open('POST', './api/analyze', true); //route
  request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

  // input value
  let text = document.getElementById('input').value;

  // make it a json object
  let data = JSON.stringify({
    content: text
  })

  request.send(data); // send the object

  document.getElementById('input').value = ''; //clear form input

});