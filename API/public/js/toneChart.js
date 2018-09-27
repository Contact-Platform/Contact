function main(flag = 0) {
  if (flag == 1){
    let container = document.getElementById('tone-analyzer')
    let ctx = container.getContext('2d');
    ctx.clearRect(0, 0, container.width, container.height);
  }
  fetch('./api/get/tone/datasets')
    .then(res => res.json())
    .then(json => {
      drawChart(json)
    })
    .catch(err => {
      console.log(err);
    });

  function drawChart(json) {
    let container = document.getElementById('tone-analyzer').getContext('2d');
    let chart = new Chart(container, {
      type: 'horizontalBar',
      data: {
        labels: json.labels,
        datasets: json.datasets
      },
      options: {
        responsive: true,
        legend: {
          display: false
        }
      }
    });
  }
}

main();
setInterval(main(1), 60000);