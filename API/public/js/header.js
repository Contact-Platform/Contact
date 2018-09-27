document.getElementById("menu-btn").addEventListener("click", function () {
  var e = document.getElementById("menu");
  var h = "hidden";
  if (e.classList.contains(h)) {
    e.classList.remove(h);
  } else {
    e.classList.add(h);
  }
});