var Words;

function main() {
  fetch('./api/get/wordcloud')
    .then(res => res.json())
    .then(words => {
      Words = words;
      drawCloud(words)
    })
    .catch(err => {
      console.log(err);
    });
}

function drawCloud(words) {
  $('#wordcloud').empty();
  let entries = d3.entries(words)
  let width = $('#wordcloud').width();
  let height = $('#wordcloud').height();
  var fill = d3.scale.category20();
  var xScale = d3.scale.linear()
    .domain([0, d3.max(entries, d => d.value)])
    .range([10, getMaxScale()]);

  d3.layout.cloud()
    .size([width, height])
    .words(entries)
    .rotate(0)
    .text(d => d.key)
    .font('sans-serif')
    .fontSize(d => xScale(+d.value))
    .on("end", draw)
    .start();

  d3.layout.cloud().stop();

  function draw(words) {
    d3.select('#wordcloud').append("svg")
      .attr("width", '100%')
      .attr("height", '100%')
      .append("g")
      .attr("transform", "translate(" + [width >> 1, height >> 1] + ")")
      .selectAll("text")
      .data(words)
      .enter().append("text")
      .style("font-size", d => xScale(d.value) + "px")
      .style("fill", (d, i) => fill(i))
      .attr("text-anchor", "middle")
      .attr("transform", d => "translate(" + [d.x, d.y] + ")rotate(" + d.rotate + ")")
      .text(d => d.key);
  }

  function getMaxScale() {
    let width = $('#wordcloud').width();
    if (width < 350)
      return 30
    if (width < 400)
      return 50
    if (width < 576)
      return 60
    else
      return 100
  }
}

$(window).resize(() => {
  var waitUntilDoneResizing;
  clearTimeout(waitUntilDoneResizing);
  waitUntilDoneResizing = setTimeout(doneResizing, 750);

  function doneResizing() {
    drawCloud(Words);
  }
});

main();
setInterval(main, 60000);