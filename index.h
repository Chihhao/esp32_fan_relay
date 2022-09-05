const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>風扇控制器</title>
    <link href="https://code.jquery.com/ui/1.11.4/themes/smoothness/jquery-ui.css" rel="stylesheet" type="text/css">
    <style type="text/css">
    body {
        font-family: "微軟正黑體", "黑體-繁", sans-serif;
    text-align: center;
    background-color: #202020;
    color: white; /* White text */
    }
  
  #status {
    color: #FFD700;
  }  
    
  #btn-group button {
    background-color: #404040; 
    border: 1px solid grey; 
    color: white; /* White text */
    padding: 20px 30px; 
    font-size: 2.5em; /* 40px/16=2.5em */
    cursor: pointer; /* Pointer/hand icon */
    width: 100%; /* Set a width if needed */
    display: block; /* Make the buttons appear below each other */
  }

  #btn-group button:not(:last-child) {
    border-bottom: none; /* Prevent double borders */
  }

  /* Add a background color on hover */
  #btn-group button:hover {
    background-color: #B8860B;
  }
  
    </style>
  </head>

  <body>
  <h1>風扇控制器</h1>
  
  <div id="status">
    <h2>現在狀態：<span id="__LEVEL__"></span></h2>
  </div>
  
  <div id="btn-group">
    <button id="Button0"> 停 </button>
    <button id="Button1"> 弱 </button>
    <button id="Button2"> 中 </button>
    <button id="Button3"> 強 </button>   
  </div>
  
  <div id="ip">
    <h3><span id="__IP__"></span></h3>   
  </div>  
  
    <script src="https://code.jquery.com/jquery-1.12.1.min.js"></script>
    <script src="https://code.jquery.com/ui/1.11.4/jquery-ui.min.js"></script>
    <script>    
    getIP();
    
    $("#Button0").click(function(){postLevel("0");});
    $("#Button1").click(function(){postLevel("1");});
    $("#Button2").click(function(){postLevel("2");});
    $("#Button3").click(function(){postLevel("3");});
    
    function postLevel(levelStr){
      $.post("/", {level:levelStr});       
    }

    setInterval(function() {
      // Call a function repetatively with 2 Second interval
      getData();
    }, 2000); //2000mSeconds update rate

    function getData() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("__LEVEL__").innerHTML =
          this.responseText;
        }
      };
      xhttp.open("GET", "readLevel", true);
      xhttp.send();      
    }

    function getIP() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("__IP__").innerHTML =
          this.responseText;
        }
      };
      xhttp.open("GET", "readIP", true);
      xhttp.send();      
    }
    
    
    
    </script>
  </body>
</html>

)=====";
