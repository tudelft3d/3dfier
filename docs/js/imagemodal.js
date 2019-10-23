// implement modal function for images included using imagezoom.html
$(function() {
  // Get the modal
  var modal = document.getElementById('imgModal');
  var navbar = document.getElementsByClassName('navbar');

  // Get the image and insert it inside the modal - use its "alt" text as a caption
  var img = document.getElementsByClassName('figureImg');
  var modalImg = document.getElementById("img01");
  var captionText = document.getElementById("caption");

  var showModal = function(){
      modal.style.display = "block";
      modalImg.src = this.src;
      captionText.innerHTML = this.alt;
      navbar[0].style.display = "none";
  }

  for (var i = 0; i < img.length; i++) {
      img[i].addEventListener('click', showModal);
  }

  // When the user clicks on the modal, close the modal
  if(modal != null){
    modal.onclick = function() {
        modal.style.display = "none";
        navbar[0].style.display = "block";
    }
  }
});