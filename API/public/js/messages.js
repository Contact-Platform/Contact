function main() {
  fetch('./api/get/sentiment')
    .then(res => res.json())
    .then(json => {
      document.getElementById("neutral-number").innerText = json.neutral
      document.getElementById("negative-number").innerText = json.negative
      document.getElementById("positive-number").innerText = json.positive
    })
    .catch(err => {
      console.log(err);
    });
}

main();
setInterval(main, 60000);