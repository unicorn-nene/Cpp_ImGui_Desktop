//whenever we load the page, always display the first slide
var currentSlide = 0

//here we set how many slides we have using the .length property
//this is useful because we can use it as our max slide value
var totalSlides = $('.holder div').length


// 1. a function that deals with taking us to the next slide
var nextSlide = function(){
// increment our currentSlide value by reassigning it and increlenting 	   it by 1
  currentSlide = currentSlide + 1

  //a continuacion ponemos este codigo para que nuestras slides y numeros no sigan avanzando cuando lleguen a la 4ta
  //
  if(currentSlide >= totalSlides){
    currentSlide = 0
  }
// we are going to turn our currentSlide value into a negative vw unit
  var leftPosition = (-currentSlide * 100)  + 'vw'
// here we add the 'vw' unit into the end

// pass the vw unit into our css method below
//here we grab the holder and change it to the second slide
  $('.holder').css('left', leftPosition)

var slideNumber = currentSlide + 1
// here we set the text for the steps using currentSlide and total nubmer
$('.steps').text(slideNumber + ' / ' + totalSlides)
}

//2. a function that deals with taking us to the previous slide

var previousSlide = function(){
 //this is identical to our nextSlide function, apart from that we are decrementing the currentSlide value (taking us back rather than fowards)
  currentSlide = currentSlide - 1

 //a continuacion ponemos este codigo para que nuestras slides y numeros no sigan retrocediendo cuando lleguen al 0
 //
 if(currentSlide < 0){
    currentSlide = totalSlides - 1
 }

  var leftPosition = (-currentSlide * 100)  + 'vw'
  $('.holder').css('left', leftPosition)

var slideNumber = currentSlide + 1
// here we set the text for the steps using currentSlide and total nubmer
$('.steps').text(slideNumber + ' / ' + totalSlides)
}

//setInterval allows us to run a function every x amount of time
var autoSlide = setInterval(function(){
// here our nextSlide function will be run
nextSlide()
// runs every 3seconds (3000ms)
}, 3000)

//we also have setTimeout, wich is the same, but runs only once

$('.next').on('click', function(){
  //this is going to cancel our autoSlide interval function
  //as the user has taken over control of the slideshow
  clearInterval(autoSlide)
  // here we call the nextSlide function and fo to the next slide
  nextSlide()
})


$('.prev').on('click', function(){
  clearInterval(autoSlide)
  previousSlide()
})

$('body').on('keydown', function(event){
  var keyCode = event.keyCode
  if(keyCode ==37){
    clearInterval(autoSlide)
    previousSlide()
  }
   if(keyCode ==39){
    clearInterval(autoSlide)
    nextSlide()
   }
})
