function main(flag = 0) {
  if (flag == 1) {
    let container = document.getElementById('sentiment')
    let ctx = container.getContext('2d');
    ctx.clearRect(0, 0, container.width, container.height);
  }
  fetch('./api/get/sentiment/datasets')
    .then(res => res.json())
    .then(json => {
      drawChart(json);
    })
    .catch(err => {
      console.log(err);
    });

  function drawChart(json) {
    let container = document.getElementById('sentiment').getContext('2d');
    let chart = new Chart(container, {
      type: 'line',
      data: {
        labels: json.labels,
        datasets: json.datasets
      },
      options: {
        responsive: true,
        tooltips: {
          mode: 'index',
          intersect: false,
        },
        hover: {
          mode: 'nearest',
          intersect: true
        }
      }
    });
  }
}

main();
setInterval(main(1), 60000);